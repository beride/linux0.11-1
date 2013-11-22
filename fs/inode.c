/* passed
 *  linux/fs/inode.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

#include <string.h>// 字符串头文件。主要定义了一些有关字符串操作的嵌入函数。
#include <sys/stat.h>// 文件状态头文件。含有文件或文件系统状态结构stat{}和常量。

#include <linux/sched.h>// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
						// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/mm.h>// 内存管理头文件。含有页面大小定义和一些页面释放函数原型。
#include <asm/system.h>// 系统头文件。定义了设置或修改描述符/中断门等的嵌入式汇编宏。

struct m_inode inode_table[NR_INODE]={0};// 内存中i 节点表（NR_INODE=32 项）。

static void read_inode(struct m_inode * inode);
static void write_inode(struct m_inode * inode);

//// 等待指定的i 节点可用。
// 如果i 节点已被锁定，则将当前任务置为不可中断的等待状态。直到该i 节点解锁。
static _inline void wait_on_inode(struct m_inode * inode)
{
	cli();
	while (inode->i_lock)
		sleep_on(&inode->i_wait);
	sti();
}

//// 对指定的i 节点上锁（锁定指定的i 节点）。
// 如果i 节点已被锁定，则将当前任务置为不可中断的等待状态。
// 直到该i 节点解锁，然后对其上锁。
static _inline void lock_inode(struct m_inode * inode)
{
	cli();
	while (inode->i_lock)
		sleep_on(&inode->i_wait);
	inode->i_lock=1;	// 置锁定标志。
	sti();
}

//// 对指定的i 节点解锁。
// 复位i 节点的锁定标志，并明确地唤醒等待此i 节点的进程。
static _inline void unlock_inode(struct m_inode * inode)
{
	inode->i_lock=0;
	wake_up(&inode->i_wait);
}

//// 释放内存中设备dev 的所有i 节点。
// 扫描内存中的i 节点表数组，如果是指定设备使用的i 节点就释放之。
void invalidate_inodes(int dev)
{
	int i;
	struct m_inode * inode;

	inode = 0+inode_table;		// 让指针首先指向i 节点表指针数组首项。
	for(i=0 ; i<NR_INODE ; i++,inode++) {	// 扫描i 节点表指针数组中的所有i 节点。
		wait_on_inode(inode);				// 等待该i 节点可用（解锁）。
		if (inode->i_dev == dev) {			// 如果是指定设备的i 节点，则
			if (inode->i_count)				// 如果其引用数不为0，则显示出错警告；
				printk("inode in use on removed disk\n\r");
			inode->i_dev = inode->i_dirt = 0;	// 释放该i 节点(置设备号为0 等)。
		}
	}
}

//// 同步所有i 节点。
// 同步内存与设备上的所有i 节点信息。
void sync_inodes(void)
{
	int i;
	struct m_inode * inode;

	inode = 0+inode_table;		// 让指针首先指向i 节点表指针数组首项。
	for(i=0 ; i<NR_INODE ; i++,inode++) {	// 扫描i 节点表指针数组。
		wait_on_inode(inode);				// 等待该i 节点可用（解锁）。
		if (inode->i_dirt && !inode->i_pipe)	// 如果该i 节点已修改且不是管道节点，
			write_inode(inode);					// 则写盘。
	}
}

//// 文件数据块映射到盘块的处理操作。(block 位图处理函数，bmap - block map)
// 参数：inode – 文件的i 节点；block – 文件中的数据块号；create - 创建标志。
// 如果创建标志置位，则在对应逻辑块不存在时就申请新磁盘块。
// 返回block 数据块对应在设备上的逻辑块号（盘块号）。
static int _bmap(struct m_inode * inode,int block,int create)
{
	struct buffer_head * bh;
	int i;

// 如果块号小于0，则死机。
	if (block<0)
		panic("_bmap: block<0");
// 如果块号大于直接块数+ 间接块数+ 二次间接块数，超出文件系统表示范围，则死机。
	if (block >= 7+512+512*512)
		panic("_bmap: block>big");
// 如果该块号小于7，则使用直接块表示。
	if (block<7) {
// 如果创建标志置位，并且i 节点中对应该块的逻辑块（区段）字段为0，则向相应设备申请一磁盘
// 块（逻辑块，区块），并将盘上逻辑块号（盘块号）填入逻辑块字段中。然后设置i 节点修改时间，
// 置i 节点已修改标志。最后返回逻辑块号。
		if (create && !inode->i_zone[block])
			if (inode->i_zone[block]=new_block(inode->i_dev)) {
				inode->i_ctime=CURRENT_TIME;
				inode->i_dirt=1;
			}
		return inode->i_zone[block];
	}
// 如果该块号>=7，并且小于7+512，则说明是一次间接块。下面对一次间接块进行处理。
	block -= 7;
	if (block<512) {
// 如果是创建，并且该i 节点中对应间接块字段为0，表明文件是首次使用间接块，则需申请
// 一磁盘块用于存放间接块信息，并将此实际磁盘块号填入间接块字段中。然后设置i 节点
// 已修改标志和修改时间。
		if (create && !inode->i_zone[7])
			if (inode->i_zone[7]=new_block(inode->i_dev)) {
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
// 若此时i 节点间接块字段中为0，表明申请磁盘块失败，返回0 退出。
		if (!inode->i_zone[7])
			return 0;
// 读取设备上的一次间接块。
		if (!(bh = bread(inode->i_dev,inode->i_zone[7])))
			return 0;
// 取该间接块上第block 项中的逻辑块号（盘块号）。
		i = ((unsigned short *) (bh->b_data))[block];
// 如果是创建并且间接块的第block 项中的逻辑块号为0 的话，则申请一磁盘块（逻辑块），并让
// 间接块中的第block 项等于该新逻辑块块号。然后置位间接块的已修改标志。
		if (create && !i)
			if (i=new_block(inode->i_dev)) {
				((unsigned short *) (bh->b_data))[block]=i;
				bh->b_dirt=1;
			}
// 最后释放该间接块，返回磁盘上新申请的对应block 的逻辑块的块号。
		brelse(bh);
		return i;
	}
// 程序运行到此，表明数据块是二次间接块，处理过程与一次间接块类似。下面是对二次间接块的处理。
// 将block 再减去间接块所容纳的块数(512)。
	block -= 512;
// 如果是新创建并且i 节点的二次间接块字段为0，则需申请一磁盘块用于存放二次间接块的一级块
// 信息，并将此实际磁盘块号填入二次间接块字段中。之后，置i 节点已修改编制和修改时间。
	if (create && !inode->i_zone[8])
		if (inode->i_zone[8]=new_block(inode->i_dev)) {
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
		}
// 若此时i 节点二次间接块字段为0，表明申请磁盘块失败，返回0 退出。
	if (!inode->i_zone[8])
		return 0;
// 读取该二次间接块的一级块。
	if (!(bh=bread(inode->i_dev,inode->i_zone[8])))
		return 0;
// 取该二次间接块的一级块上第(block/512)项中的逻辑块号。
	i = ((unsigned short *)bh->b_data)[block>>9];
// 如果是创建并且二次间接块的一级块上第(block/512)项中的逻辑块号为0 的话，则需申请一磁盘
// 块（逻辑块）作为二次间接块的二级块，并让二次间接块的一级块中第(block/512)项等于该二级
// 块的块号。然后置位二次间接块的一级块已修改标志。并释放二次间接块的一级块。
	if (create && !i)
		if (i=new_block(inode->i_dev)) {
			((unsigned short *) (bh->b_data))[block>>9]=i;
			bh->b_dirt=1;
		}
	brelse(bh);
// 如果二次间接块的二级块块号为0，表示申请磁盘块失败，返回0 退出。
	if (!i)
		return 0;
// 读取二次间接块的二级块。
	if (!(bh=bread(inode->i_dev,i)))
		return 0;
// 取该二级块上第block 项中的逻辑块号。(与上511 是为了限定block 值不超过511)
	i = ((unsigned short *)bh->b_data)[block&511];
// 如果是创建并且二级块的第block 项中的逻辑块号为0 的话，则申请一磁盘块（逻辑块），作为
// 最终存放数据信息的块。并让二级块中的第block 项等于该新逻辑块块号(i)。然后置位二级块的
// 已修改标志。
	if (create && !i)
		if (i=new_block(inode->i_dev)) {
			((unsigned short *) (bh->b_data))[block&511]=i;
			bh->b_dirt=1;
		}
// 最后释放该二次间接块的二级块，返回磁盘上新申请的对应block 的逻辑块的块号。
	brelse(bh);
	return i;
}

//// 根据i 节点信息取文件数据块block 在设备上对应的逻辑块号。
int bmap(struct m_inode * inode,int block)
{
	return _bmap(inode,block,0);
}

//// 创建文件数据块block 在设备上对应的逻辑块，并返回设备上对应的逻辑块号。
int create_block(struct m_inode * inode, int block)
{
	return _bmap(inode,block,1);
}

//// 释放一个i 节点(回写入设备)。
void iput(struct m_inode * inode)
{
	if (!inode)
		return;
	wait_on_inode(inode);	// 等待inode 节点解锁(如果已上锁的话)。
	if (!inode->i_count)
		panic("iput: trying to free free inode");
// 如果是管道i 节点，则唤醒等待该管道的进程，引用次数减1，如果还有引用则返回。否则释放
// 管道占用的内存页面，并复位该节点的引用计数值、已修改标志和管道标志，并返回。
// 对于pipe 节点，inode->i_size 存放着物理内存页地址。参见get_pipe_inode()，228，234 行。
	if (inode->i_pipe) {
		wake_up(&inode->i_wait);
		if (--inode->i_count)
			return;
		free_page(inode->i_size);
		inode->i_count=0;
		inode->i_dirt=0;
		inode->i_pipe=0;
		return;
	}
// 如果i 节点对应的设备号=0，则将此节点的引用计数递减1，返回。
	if (!inode->i_dev) {
		inode->i_count--;
		return;
	}
// 如果是块设备文件的i 节点，此时逻辑块字段0 中是设备号，则刷新该设备。并等待i 节点解锁。
	if (S_ISBLK(inode->i_mode)) {
		sync_dev(inode->i_zone[0]);
		wait_on_inode(inode);
	}
repeat:
// 如果i 节点的引用计数大于1，则递减1。
	if (inode->i_count>1) {
		inode->i_count--;
		return;
	}
// 如果i 节点的链接数为0，则释放该i 节点的所有逻辑块，并释放该i 节点。
	if (!inode->i_nlinks) {
		truncate(inode);
		free_inode(inode);
		return;
	}
// 如果该i 节点已作过修改，则更新该i 节点，并等待该i 节点解锁。
	if (inode->i_dirt) {
		write_inode(inode);	/* we can sleep - so do again */
		wait_on_inode(inode);
		goto repeat;
	}
// i 节点引用计数递减1。
	inode->i_count--;
	return;
}

//// 从i 节点表(inode_table)中获取一个空闲i 节点项。
// 寻找引用计数count 为0 的i 节点，并将其写盘后清零，返回其指针。
struct m_inode * get_empty_inode(void)
{
	struct m_inode * inode;
	static struct m_inode * last_inode = inode_table;// last_inode 指向i 节点表第一项。
	int i;

	do {
		// 扫描i 节点表。
		inode = NULL;
		for (i = NR_INODE; i ; i--) {
// 如果last_inode 已经指向i 节点表的最后1 项之后，则让其重新指向i 节点表开始处。
			if (++last_inode >= inode_table + NR_INODE)
				last_inode = inode_table;
// 如果last_inode 所指向的i 节点的计数值为0，则说明可能找到空闲i 节点项。让inode 指向
// 该i 节点。如果该i 节点的已修改标志和锁定标志均为0，则我们可以使用该i 节点，于是退出循环。
			if (!last_inode->i_count) {
				inode = last_inode;
				if (!inode->i_dirt && !inode->i_lock)
					break;
			}
		}
// 如果没有找到空闲i 节点(inode=NULL)，则将整个i 节点表打印出来供调试使用，并死机。
		if (!inode) {
			for (i=0 ; i<NR_INODE ; i++)
				printk("%04x: %6d\t",inode_table[i].i_dev,
					inode_table[i].i_num);
			panic("No free inodes in mem");
		}
// 等待该i 节点解锁（如果又被上锁的话）。
		wait_on_inode(inode);
// 如果该i 节点已修改标志被置位的话，则将该i 节点刷新，并等待该i 节点解锁。
		while (inode->i_dirt) {
			write_inode(inode);
			wait_on_inode(inode);
		}
	} while (inode->i_count);// 如果i 节点又被其它占用的话，则重新寻找空闲i 节点。
// 已找到空闲i 节点项。则将该i 节点项内容清零，并置引用标志为1，返回该i 节点指针。
	memset(inode,0,sizeof(*inode));
	inode->i_count = 1;
	return inode;
}

//// 获取管道节点。返回为i 节点指针（如果是NULL 则失败）。
// 首先扫描i 节点表，寻找一个空闲i 节点项，然后取得一页空闲内存供管道使用。
// 然后将得到的i 节点的引用计数置为2(读者和写者)，初始化管道头和尾，置i 节点的管道类型表示。
struct m_inode * get_pipe_inode(void)
{
	struct m_inode * inode;

	if (!(inode = get_empty_inode()))	// 如果找不到空闲i 节点则返回NULL。
		return NULL;
	if (!(inode->i_size=get_free_page())) {// 节点的i_size 字段指向缓冲区。
		inode->i_count = 0;					// 如果已没有空闲内存，则
		return NULL;						// 释放该i 节点，并返回NULL。
	}
	inode->i_count = 2;	/* 读/写两者总计 */
	PIPE_HEAD(*inode) = PIPE_TAIL(*inode) = 0;// 复位管道头尾指针。
	inode->i_pipe = 1;			// 置节点为管道使用的标志。
	return inode;		// 返回i 节点指针。
}

//// 从设备上读取指定节点号的i 节点。
// nr - i 节点号。
struct m_inode * iget(int dev,int nr)
{
	struct m_inode * inode, * empty;

	if (!dev)
		panic("iget with dev==0");
// 从i 节点表中取一个空闲i 节点。
	empty = get_empty_inode();
// 扫描i 节点表。寻找指定节点号的i 节点。并递增该节点的引用次数。
	inode = inode_table;
	while (inode < NR_INODE+inode_table) {
// 如果当前扫描的i 节点的设备号不等于指定的设备号或者节点号不等于指定的节点号，则继续扫描。
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode++;
			continue;
		}
// 找到指定设备号和节点号的i 节点，等待该节点解锁（如果已上锁的话）。
		wait_on_inode(inode);
// 在等待该节点解锁的阶段，节点表可能会发生变化，所以再次判断，如果发生了变化，则再次重新
// 扫描整个i 节点表。
		if (inode->i_dev != dev || inode->i_num != nr) {
			inode = inode_table;
			continue;
		}
// 将该i 节点引用计数增1。
		inode->i_count++;
		if (inode->i_mount) {
			int i;

// 如果该i 节点是其它文件系统的安装点，则在超级块表中搜寻安装在此i 节点的超级块。如果没有
// 找到，则显示出错信息，并释放函数开始获取的空闲节点，返回该i 节点指针。
			for (i = 0 ; i<NR_SUPER ; i++)
				if (super_block[i].s_imount==inode)
					break;
			if (i >= NR_SUPER) {
				printk("Mounted inode hasn't got sb\n");
				if (empty)
					iput(empty);
				return inode;
			}
// 将该i 节点写盘。从安装在此i 节点文件系统的超级块上取设备号，并令i 节点号为1。然后重新
// 扫描整个i 节点表，取该被安装文件系统的根节点。
			iput(inode);
			dev = super_block[i].s_dev;
			nr = ROOT_INO;
			inode = inode_table;
			continue;
		}
// 已经找到相应的i 节点，因此放弃临时申请的空闲节点，返回该找到的i 节点。
		if (empty)
			iput(empty);
		return inode;
	}
// 如果在i 节点表中没有找到指定的i 节点，则利用前面申请的空闲i 节点在i 节点表中建立该节点。
// 并从相应设备上读取该i 节点信息。返回该i 节点。
	if (!empty)
		return (NULL);
	inode=empty;
	inode->i_dev = dev;
	inode->i_num = nr;
	read_inode(inode);
	return inode;
}

//// 从设备上读取指定i 节点的信息到内存中（缓冲区中）。
static void read_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;
	int block;

// 首先锁定该i 节点，取该节点所在设备的超级块。
	lock_inode(inode);
	if (!(sb=get_super(inode->i_dev)))
		panic("trying to read inode without dev");
// 该i 节点所在的逻辑块号= (启动块+超级块) + i 节点位图占用的块数+ 逻辑块位图占用的块数+
// (i 节点号-1)/每块含有的i 节点数。
	block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks +
		(inode->i_num-1)/INODES_PER_BLOCK;
// 从设备上读取该i 节点所在的逻辑块，并将该inode 指针指向对应i 节点信息。
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
	*(struct d_inode *)inode =
		((struct d_inode *)bh->b_data)
			[(inode->i_num-1)%INODES_PER_BLOCK];
// 最后释放读入的缓冲区，并解锁该i 节点。
	brelse(bh);
	unlock_inode(inode);
}

//// 将指定i 节点信息写入设备（写入缓冲区相应的缓冲块中，待缓冲区刷新时会写入盘中）。
static void write_inode(struct m_inode * inode)
{
	struct super_block * sb;
	struct buffer_head * bh;
	int block;

// 首先锁定该i 节点，如果该i 节点没有被修改过或者该i 节点的设备号等于零，则解锁该i 节点，
// 并退出。
	lock_inode(inode);
	if (!inode->i_dirt || !inode->i_dev) {
		unlock_inode(inode);
		return;
	}
// 获取该i 节点的超级块。
	if (!(sb=get_super(inode->i_dev)))
		panic("trying to write inode without device");
// 该i 节点所在的逻辑块号= (启动块+超级块) + i 节点位图占用的块数+ 逻辑块位图占用的块数+
// (i 节点号-1)/每块含有的i 节点数。
	block = 2 + sb->s_imap_blocks + sb->s_zmap_blocks +
		(inode->i_num-1)/INODES_PER_BLOCK;
// 从设备上读取该i 节点所在的逻辑块。
	if (!(bh=bread(inode->i_dev,block)))
		panic("unable to read i-node block");
// 将该i 节点信息复制到逻辑块对应该i 节点的项中。
	((struct d_inode *)bh->b_data)
		[(inode->i_num-1)%INODES_PER_BLOCK] =
			*(struct d_inode *)inode;
// 置缓冲区已修改标志，而i 节点修改标志置零。然后释放该含有i 节点的缓冲区，并解锁该i 节点。
	bh->b_dirt=1;
	inode->i_dirt=0;
	brelse(bh);
	unlock_inode(inode);
}
