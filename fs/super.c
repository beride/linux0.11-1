/* passed
* linux/fs/super.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

/*
* super.c contains code to handle the super-block tables.
*/
#include <linux/config.h>	// 内核配置头文件。定义键盘语言和硬盘类型（HD_TYPE）可选项。
#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/system.h>		// 系统头文件。定义了设置或修改描述符/中断门等的嵌入式汇编宏。

#include <errno.h>		// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。
#include <sys/stat.h>		// 文件状态头文件。含有文件或文件系统状态结构stat{}和常量。

int sync_dev (int dev);		// 对指定设备执行高速缓冲与设备上数据的同步操作。(fs/buffer.c,59)
void wait_for_keypress (void);	// 等待击键。(kernel/chr_drv/tty_io.c, 140)

/* set_bit()使用了setb 指令，因为汇编编译器gas 不能识别指令setc */
//// 测试指定位偏移处比特位的值(0 或1)，并返回该比特位值。(应该取名为test_bit()更妥帖)
// 嵌入式汇编宏。参数bitnr 是比特位偏移值，addr 是测试比特位操作的起始地址。
// %0 - ax(__res)，%1 - 0，%2 - bitnr，%3 - addr
/*#define set_bit(bitnr,addr) ({ \
register int __res __asm__( "ax"); \
__asm__( "bt %2,%3;setb %%al": "=a" (__res): "a" (0), "r" (bitnr), "m" (*(addr))); \
__res; })*/
extern _inline int set_bit(int bitnr,char* addr)
{
//	register int __res;
	_asm{
		xor eax,eax
		mov ebx,bitnr
		mov edx,addr
		bt [edx],ebx
		setb al
//		mov __res,eax
	}
//	return __res;
}

struct super_block super_block[NR_SUPER] = {0};	// 超级块结构数组（共8 项）。
/* this is initialized in init/main.c */
/* ROOT_DEV 已在init/main.c 中被初始化 */
int ROOT_DEV;

//// 锁定指定的超级块。
static void
lock_super (struct super_block *sb)
{
	cli ();			// 关中断。
	while (sb->s_lock)		// 如果该超级块已经上锁，则睡眠等待。
		sleep_on (&(sb->s_wait));
	sb->s_lock = 1;		// 给该超级块加锁（置锁定标志）。
	sti ();			// 开中断。
}

//// 对指定超级块解锁。（如果使用ulock_super 这个名称则更妥帖）。
static void
free_super (struct super_block *sb)
{
	cli ();			// 关中断。
	sb->s_lock = 0;		// 复位锁定标志。
	wake_up (&(sb->s_wait));	// 唤醒等待该超级块的进程。
	sti ();			// 开中断。
}

//// 睡眠等待超级块解锁。
static void
wait_on_super (struct super_block *sb)
{
	cli ();			// 关中断。
	while (sb->s_lock)		// 如果超级块已经上锁，则睡眠等待。
		sleep_on (&(sb->s_wait));
	sti ();			// 开中断。
}

//// 取指定设备的超级块。返回该超级块结构指针。
struct super_block *
get_super (int dev)
{
	struct super_block *s;

// 如果没有指定设备，则返回空指针。
	if (!dev)
		return NULL;
// s 指向超级块数组开始处。搜索整个超级块数组，寻找指定设备的超级块。
	s = 0 + super_block;
	while (s < NR_SUPER + super_block)
// 如果当前搜索项是指定设备的超级块，则首先等待该超级块解锁（若已经被其它进程上锁的话）。
// 在等待期间，该超级块有可能被其它设备使用，因此此时需再判断一次是否是指定设备的超级块，
// 如果是则返回该超级块的指针。否则就重新对超级块数组再搜索一遍，因此s 重又指向超级块数组
// 开始处。
	if (s->s_dev == dev)
	{
		wait_on_super (s);
		if (s->s_dev == dev)
			return s;
		s = 0 + super_block;
// 如果当前搜索项不是，则检查下一项。如果没有找到指定的超级块，则返回空指针。
	}
	else
		s++;
	return NULL;
}

//// 释放指定设备的超级块。
// 释放设备所使用的超级块数组项（置s_dev=0），并释放该设备i 节点位图和逻辑块位图所占用
// 的高速缓冲块。如果超级块对应的文件系统是根文件系统，或者其i 节点上已经安装有其它的文件
// 系统，则不能释放该超级块。
void
put_super (int dev)
{
	struct super_block *sb;
//  struct m_inode *inode;
	int i;

// 如果指定设备是根文件系统设备，则显示警告信息“根系统盘改变了，准备生死决战吧”，并返回。
	if (dev == ROOT_DEV)
	{
		printk ("root diskette changed: prepare for armageddon\n\r");
		return;
	}
// 如果找不到指定设备的超级块，则返回。
	if (!(sb = get_super (dev)))
		return;
// 如果该超级块指明本文件系统i 节点上安装有其它的文件系统，则显示警告信息，返回。
	if (sb->s_imount)
	{
		printk ("Mounted disk changed - tssk, tssk\n\r");
		return;
	}
// 找到指定设备的超级块后，首先锁定该超级块，然后置该超级块对应的设备号字段为0，也即即将
// 放弃该超级块。
	lock_super (sb);
	sb->s_dev = 0;
// 然后释放该设备i 节点位图和逻辑块位图在缓冲区中所占用的缓冲块。
	for (i = 0; i < I_MAP_SLOTS; i++)
		brelse (sb->s_imap[i]);
	for (i = 0; i < Z_MAP_SLOTS; i++)
		brelse (sb->s_zmap[i]);
// 最后对该超级块解锁，并返回。
	free_super (sb);
	return;
}

//// 从设备上读取超级块到缓冲区中。
// 如果该设备的超级块已经在高速缓冲中并且有效，则直接返回该超级块的指针。
static struct super_block *
read_super (int dev)
{
	struct super_block *s;
	struct buffer_head *bh;
	int i, block;

// 如果没有指明设备，则返回空指针。
	if (!dev)
		return NULL;
// 首先检查该设备是否可更换过盘片（也即是否是软盘设备），如果更换过盘，则高速缓冲区有关该
// 设备的所有缓冲块均失效，需要进行失效处理（释放原来加载的文件系统）。
	check_disk_change (dev);
// 如果该设备的超级块已经在高速缓冲中，则直接返回该超级块的指针。
	if (s = get_super (dev))
		return s;
// 否则，首先在超级块数组中找出一个空项(也即其s_dev=0 的项)。如果数组已经占满则返回空指针。
	for (s = 0 + super_block;; s++)
	{
		if (s >= NR_SUPER + super_block)
			return NULL;
		if (!s->s_dev)
			break;
	}
// 找到超级块空项后，就将该超级块用于指定设备，对该超级块的内存项进行部分初始化。
	s->s_dev = dev;
	s->s_isup = NULL;
	s->s_imount = NULL;
	s->s_time = 0;
	s->s_rd_only = 0;
	s->s_dirt = 0;
// 然后锁定该超级块，并从设备上读取超级块信息到bh 指向的缓冲区中。如果读超级块操作失败，
// 则释放上面选定的超级块数组中的项，并解锁该项，返回空指针退出。
	lock_super (s);
	if (!(bh = bread (dev, 1)))
	{
		s->s_dev = 0;
		free_super (s);
		return NULL;
	}
// 将设备上读取的超级块信息复制到超级块数组相应项结构中。并释放存放读取信息的高速缓冲块。
	*((struct d_super_block *) s) = *((struct d_super_block *) bh->b_data);
	brelse (bh);
// 如果读取的超级块的文件系统魔数字段内容不对，说明设备上不是正确的文件系统，因此同上面
// 一样，释放上面选定的超级块数组中的项，并解锁该项，返回空指针退出。
// 对于该版linux 内核，只支持minix 文件系统版本1.0，其魔数是0x137f。
	if (s->s_magic != SUPER_MAGIC)
	{
		s->s_dev = 0;
		free_super (s);
		return NULL;
	}
// 下面开始读取设备上i 节点位图和逻辑块位图数据。首先初始化内存超级块结构中位图空间。
	for (i = 0; i < I_MAP_SLOTS; i++)
		s->s_imap[i] = NULL;
	for (i = 0; i < Z_MAP_SLOTS; i++)
		s->s_zmap[i] = NULL;
// 然后从设备上读取i 节点位图和逻辑块位图信息，并存放在超级块对应字段中。
	block = 2;
	for (i = 0; i < s->s_imap_blocks; i++)
		if (s->s_imap[i] = bread (dev, block))
			block++;
		else
			break;
	for (i = 0; i < s->s_zmap_blocks; i++)
		if (s->s_zmap[i] = bread (dev, block))
			block++;
		else
			break;
// 如果读出的位图逻辑块数不等于位图应该占有的逻辑块数，说明文件系统位图信息有问题，超级块
// 初始化失败。因此只能释放前面申请的所有资源，返回空指针并退出。
	if (block != 2 + s->s_imap_blocks + s->s_zmap_blocks)
	{
// 释放i 节点位图和逻辑块位图占用的高速缓冲区。
		for (i = 0; i < I_MAP_SLOTS; i++)
			brelse (s->s_imap[i]);
		for (i = 0; i < Z_MAP_SLOTS; i++)
			brelse (s->s_zmap[i]);
//释放上面选定的超级块数组中的项，并解锁该超级块项，返回空指针退出。
		s->s_dev = 0;
		free_super (s);
		return NULL;
	}
// 否则一切成功。对于申请空闲i 节点的函数来讲，如果设备上所有的i 节点已经全被使用，则查找
// 函数会返回0 值。因此0 号i 节点是不能用的，所以这里将位图中的最低位设置为1，以防止文件
// 系统分配0 号i 节点。同样的道理，也将逻辑块位图的最低位设置为1。
	s->s_imap[0]->b_data[0] |= 1;
	s->s_zmap[0]->b_data[0] |= 1;
// 解锁该超级块，并返回超级块指针。
	free_super (s);
	return s;
}

//// 卸载文件系统的系统调用函数。
// 参数dev_name 是设备文件名。
int
sys_umount (char *dev_name)
{
	struct m_inode *inode;
	struct super_block *sb;
	int dev;

// 首先根据设备文件名找到对应的i 节点，并取其中的设备号。
	if (!(inode = namei (dev_name)))
		return -ENOENT;
	dev = inode->i_zone[0];
// 如果不是块设备文件，则释放刚申请的i 节点dev_i，返回出错码。
	if (!S_ISBLK (inode->i_mode))
	{
		iput (inode);
		return -ENOTBLK;
	}
// 释放设备文件名的i 节点。
	iput (inode);
// 如果设备是根文件系统，则不能被卸载，返回出错号。
	if (dev == ROOT_DEV)
		return -EBUSY;
// 如果取设备的超级块失败，或者该设备文件系统没有安装过，则返回出错码。
	if (!(sb = get_super (dev)) || !(sb->s_imount))
		return -ENOENT;
// 如果超级块所指明的被安装到的i 节点没有置位其安装标志，则显示警告信息。
	if (!sb->s_imount->i_mount)
		printk ("Mounted inode has i_mount=0\n");
// 查找i 节点表，看是否有进程在使用该设备上的文件，如果有则返回忙出错码。
	for (inode = inode_table + 0; inode < inode_table + NR_INODE; inode++)
		if (inode->i_dev == dev && inode->i_count)
			return -EBUSY;
// 复位被安装到的i 节点的安装标志，释放该i 节点。
	sb->s_imount->i_mount = 0;
	iput (sb->s_imount);
// 置超级块中被安装i 节点字段为空，并释放设备文件系统的根i 节点，置超级块中被安装系统
// 根i 节点指针为空。
	sb->s_imount = NULL;
	iput (sb->s_isup);
	sb->s_isup = NULL;
// 释放该设备的超级块以及位图占用的缓冲块，并对该设备执行高速缓冲与设备上数据的同步操作。
	put_super (dev);
	sync_dev (dev);
	return 0;
}

//// 安装文件系统调用函数。
// 参数dev_name 是设备文件名，dir_name 是安装到的目录名，rw_flag 被安装文件的读写标志。
// 将被加载的地方必须是一个目录名，并且对应的i 节点没有被其它程序占用。
int
sys_mount (char *dev_name, char *dir_name, int rw_flag)
{
	struct m_inode *dev_i, *dir_i;
	struct super_block *sb;
	int dev;

// 首先根据设备文件名找到对应的i 节点，并取其中的设备号。
// 对于块特殊设备文件，设备号在i 节点的i_zone[0]中。
	if (!(dev_i = namei (dev_name)))
		return -ENOENT;
	dev = dev_i->i_zone[0];
// 如果不是块设备文件，则释放刚取得的i 节点dev_i，返回出错码。
	if (!S_ISBLK (dev_i->i_mode))
	{
		iput (dev_i);
		return -EPERM;
	}
// 释放该设备文件的i 节点dev_i。
	iput (dev_i);
// 根据给定的目录文件名找到对应的i 节点dir_i。
	if (!(dir_i = namei (dir_name)))
		return -ENOENT;
// 如果该i 节点的引用计数不为1（仅在这里引用），或者该i 节点的节点号是根文件系统的节点
// 号1，则释放该i 节点，返回出错码。
	if (dir_i->i_count != 1 || dir_i->i_num == ROOT_INO)
	{
		iput (dir_i);
		return -EBUSY;
	}
// 如果该节点不是一个目录文件节点，则也释放该i 节点，返回出错码。
	if (!S_ISDIR (dir_i->i_mode))
	{
		iput (dir_i);
		return -EPERM;
	}
// 读取将安装文件系统的超级块，如果失败则也释放该i 节点，返回出错码。
	if (!(sb = read_super (dev)))
	{
		iput (dir_i);
		return -EBUSY;
	}
// 如果将要被安装的文件系统已经安装在其它地方，则释放该i 节点，返回出错码。
	if (sb->s_imount)
	{
		iput (dir_i);
		return -EBUSY;
	}
// 如果将要安装到的i 节点已经安装了文件系统(安装标志已经置位)，则释放该i 节点，返回出错码。
	if (dir_i->i_mount)
	{
		iput (dir_i);
		return -EPERM;
	}
// 被安装文件系统超级块的“被安装到i 节点”字段指向安装到的目录名的i 节点。
	sb->s_imount = dir_i;
// 设置安装位置i 节点的安装标志和节点已修改标志。/* 注意！这里没有iput(dir_i) */
	dir_i->i_mount = 1;		/* 这将在umount 内操作 */
	dir_i->i_dirt = 1;		/* NOTE! we don't iput(dir_i) */
	return 0;			/* we do that in umount */
}

//// 安装根文件系统。
// 该函数是在系统开机初始化设置时(sys_setup())调用的。( kernel/blk_drv/hd.c, 157 )
void
mount_root (void)
{
	int i, free;
	struct super_block *p;
	struct m_inode *mi;

// 如果磁盘i 节点结构不是32 个字节，则出错，死机。该判断是用于防止修改源代码时的不一致性。
	if (32 != sizeof (struct d_inode))
		panic ("bad i-node size");
// 初始化文件表数组（共64 项，也即系统同时只能打开64 个文件），将所有文件结构中的引用计数
// 设置为0。[??为什么放在这里初始化？]
	for (i = 0; i < NR_FILE; i++)
		file_table[i].f_count = 0;
// 如果根文件系统所在设备是软盘的话，就提示“插入根文件系统盘，并按回车键”，并等待按键。
	if (MAJOR (ROOT_DEV) == 2)
	{
		printk ("Insert root floppy and press ENTER");
		wait_for_keypress ();
	}
// 初始化超级块数组（共8 项）。
	for (p = &super_block[0]; p < &super_block[NR_SUPER]; p++)
	{
		p->s_dev = 0;
		p->s_lock = 0;
		p->s_wait = NULL;
	}
// 如果读根设备上超级块失败，则显示信息，并死机。
	if (!(p = read_super (ROOT_DEV)))
		panic ("Unable to mount root");
//从设备上读取文件系统的根i 节点(1)，如果失败则显示出错信息，死机。
	if (!(mi = iget (ROOT_DEV, ROOT_INO)))
		panic ("Unable to read root i-node");
// 该i 节点引用次数递增3 次。因为下面266-268 行上也引用了该i 节点。
	mi->i_count += 3;		/* NOTE! it is logically used 4 times, not 1 */
/* 注意！从逻辑上讲，它已被引用了4 次，而不是1 次 */
// 置该超级块的被安装文件系统i 节点和被安装到的i 节点为该i 节点。
	p->s_isup = p->s_imount = mi;
// 设置当前进程的当前工作目录和根目录i 节点。此时当前进程是1 号进程。
	current->pwd = mi;
	current->root = mi;
// 统计该设备上空闲块数。首先令i 等于超级块中表明的设备逻辑块总数。
	free = 0;
	i = p->s_nzones;
// 然后根据逻辑块位图中相应比特位的占用情况统计出空闲块数。这里宏函数set_bit()只是在测试
// 比特位，而非设置比特位。"i&8191"用于取得i 节点号在当前块中的偏移值。"i>>13"是将i 除以
// 8192，也即除一个磁盘块包含的比特位数。
	while (--i >= 0)
		if (!set_bit (i & 8191, p->s_zmap[i >> 13]->b_data))
			free++;
// 显示设备上空闲逻辑块数/逻辑块总数。
	printk ("%d/%d free blocks\n\r", free, p->s_nzones);
// 统计设备上空闲i 节点数。首先令i 等于超级块中表明的设备上i 节点总数+1。加1 是将0 节点
// 也统计进去。
	free = 0;
	i = p->s_ninodes + 1;
// 然后根据i 节点位图中相应比特位的占用情况计算出空闲i 节点数。
	while (--i >= 0)
		if (!set_bit (i & 8191, p->s_imap[i >> 13]->b_data))
			free++;
// 显示设备上可用的空闲i 节点数/i 节点总数。
	printk ("%d/%d free inodes\n\r", free, p->s_ninodes);
}
