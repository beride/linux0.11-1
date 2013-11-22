/* passed
 *  linux/fs/buffer.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

/*
 * 'buffer.c'用于实现缓冲区高速缓存功能。通过不让中断过程改变缓冲区，而是让调用者
 * 来执行，避免了竞争条件（当然除改变数据以外）。注意！由于中断可以唤醒一个调用者，
 * 因此就需要开关中断指令（cli-sti）序列来检测等待调用返回。但需要非常地快(希望是这样)。
 */

/*
 * 注意！这里有一个程序应不属于这里：检测软盘是否更换。但我想这里是
 * 放置该程序最好的地方了，因为它需要使已更换软盘缓冲失效。
 */

// 标准参数头文件。以宏的形式定义变量参数列表。主要说明了-个
// 类型(va_list)和三个宏(va_start, va_arg 和va_end)，用于
// vsprintf、vprintf、vfprintf 函数。
#include <stdarg.h>
// 内核配置头文件。定义键盘语言和硬盘类型（HD_TYPE）可选项。
#include <linux/config.h>
// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/sched.h>
// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/kernel.h>
// 系统头文件。定义了设置或修改描述符/中断门等的嵌入式汇编宏。
#include <asm/system.h>
// io 头文件。定义硬件端口输入/输出宏汇编语句。
#include <asm/io.h>

extern int end;		 //由连接程序ld 生成的位于程序末端的变量。
extern void put_super(int);
extern void invalidate_inodes(int);

struct buffer_head * start_buffer = (struct buffer_head *)(&end);
struct buffer_head * hash_table[NR_HASH] = {0};	// NR_HASH = 307 项。
static struct buffer_head * free_list = 0;
static struct task_struct * buffer_wait = NULL;
int NR_BUFFERS = 0;

//// 等待指定缓冲区解锁。
static _inline void wait_on_buffer(struct buffer_head * bh)
{
	cli();		// 关中断。
	while (bh->b_lock)	// 如果已被上锁，则进程进入睡眠，等待其解锁。
		sleep_on(&bh->b_wait);
	sti();		// 开中断。
}

//// 系统调用。同步设备和内存高速缓冲中数据。
int sys_sync(void)//passed
{
	int i;
	struct buffer_head * bh;

	sync_inodes();		/* 将i 节点写入高速缓冲 */
// 扫描所有高速缓冲区，对于已被修改的缓冲块产生写盘请求，将缓冲中数据与设备中同步。
	bh = start_buffer;
	for (i=0 ; i<NR_BUFFERS ; i++,bh++) {
		wait_on_buffer(bh);		// 等待缓冲区解锁（如果已上锁的话）。
		if (bh->b_dirt)
			ll_rw_block(WRITE,bh);	// 产生写设备块请求。
	}
	return 0;
}

//// 对指定设备进行高速缓冲数据与设备上数据的同步操作。
int sync_dev(int dev)
{
	int i;
	struct buffer_head * bh;

	bh = start_buffer;
	for (i=0 ; i<NR_BUFFERS ; i++,bh++) {
		if (bh->b_dev != dev)
			continue;
		wait_on_buffer(bh);
		if (bh->b_dev == dev && bh->b_dirt)
			ll_rw_block(WRITE,bh);
	}
	sync_inodes();			// 将i 节点数据写入高速缓冲。
	bh = start_buffer;
	for (i=0 ; i<NR_BUFFERS ; i++,bh++) {
		if (bh->b_dev != dev)
			continue;
		wait_on_buffer(bh);
		if (bh->b_dev == dev && bh->b_dirt)
			ll_rw_block(WRITE,bh);
	}
	return 0;
}

//// 使指定设备在高速缓冲区中的数据无效。
// 扫描高速缓冲中的所有缓冲块，对于指定设备的缓冲区，复位其有效(更新)标志和已修改标志。
void _inline invalidate_buffers(int dev)
{
	int i;
	struct buffer_head * bh;

	bh = start_buffer;
	for (i=0 ; i<NR_BUFFERS ; i++,bh++) {
		if (bh->b_dev != dev)		// 如果不是指定设备的缓冲块，则
			continue;				// 继续扫描下一块。
		wait_on_buffer(bh);			// 等待该缓冲区解锁（如果已被上锁）。
// 由于进程执行过睡眠等待，所以需要再判断一下缓冲区是否是指定设备的。
		if (bh->b_dev == dev)
			bh->b_uptodate = bh->b_dirt = 0;
	}
}

/*
 * 该子程序检查一个软盘是否已经被更换，如果已经更换就使高速缓冲中与该软驱
 * 对应的所有缓冲区无效。该子程序相对来说较慢，所以我们要尽量少使用它。
 * 所以仅在执行'mount'或'open'时才调用它。我想这是将速度和实用性相结合的
 * 最好方法。若在操作过程当中更换软盘，会导致数据的丢失，这是咎由自取 :-)
 *
 * 注意！尽管目前该子程序仅用于软盘，以后任何可移动介质的块设备都将使用该
 * 程序，mount/open 操作是不需要知道是否是软盘或其它什么特殊介质的。
 */
//// 检查磁盘是否更换，如果已更换就使对应高速缓冲区无效。
void check_disk_change(int dev)
{
	int i;

// 是软盘设备吗？如果不是则退出。
	if (MAJOR(dev) != 2)
		return;
// 测试对应软盘是否已更换，如果没有则退出。
	if (!floppy_change(dev & 0x03))
		return;
// 软盘已经更换，所以释放对应设备的i 节点位图和逻辑块位图所占的高速缓冲区；并使该设备的
// i 节点和数据块信息所占的高速缓冲区无效。
	for (i=0 ; i<NR_SUPER ; i++)
		if (super_block[i].s_dev == dev)
			put_super(super_block[i].s_dev);
	invalidate_inodes(dev);
	invalidate_buffers(dev);
}

// hash 函数和hash 表项的计算宏定义。
#define _hashfn(dev,block) (((unsigned)(dev^block))%NR_HASH)
#define hash(dev,block) hash_table[_hashfn(dev,block)]

//// 从hash 队列和空闲缓冲队列中移走指定的缓冲块。
static _inline void remove_from_queues(struct buffer_head * bh)
{
/* 从hash 队列中移除缓冲块 */
	if (bh->b_next)
		bh->b_next->b_prev = bh->b_prev;
	if (bh->b_prev)
		bh->b_prev->b_next = bh->b_next;
// 如果该缓冲区是该队列的头一个块，则让hash 表的对应项指向本队列中的下一个缓冲区。
	if (hash(bh->b_dev,bh->b_blocknr) == bh)
		hash(bh->b_dev,bh->b_blocknr) = bh->b_next;
/* 从空闲缓冲区表中移除缓冲块 */
	if (!(bh->b_prev_free) || !(bh->b_next_free))
		panic("Free block list corrupted");
	bh->b_prev_free->b_next_free = bh->b_next_free;
	bh->b_next_free->b_prev_free = bh->b_prev_free;
// 如果空闲链表头指向本缓冲区，则让其指向下一缓冲区。
	if (free_list == bh)
		free_list = bh->b_next_free;
}

//// 将指定缓冲区插入空闲链表尾并放入hash 队列中。
static _inline void insert_into_queues(struct buffer_head * bh)
{
/* 放在空闲链表末尾处 */
	bh->b_next_free = free_list;
	bh->b_prev_free = free_list->b_prev_free;
	free_list->b_prev_free->b_next_free = bh;
	free_list->b_prev_free = bh;
/* 如果该缓冲块对应一个设备，则将其插入新hash 队列中 */
	bh->b_prev = NULL;
	bh->b_next = NULL;
	if (!bh->b_dev)
		return;
	bh->b_next = hash(bh->b_dev,bh->b_blocknr);
	hash(bh->b_dev,bh->b_blocknr) = bh;
	bh->b_next->b_prev = bh;
}

//// 在高速缓冲中寻找给定设备和指定块的缓冲区块。
// 如果找到则返回缓冲区块的指针，否则返回NULL。
static struct buffer_head * find_buffer(int dev, int block)
{		
	struct buffer_head * tmp;

	for (tmp = hash(dev,block) ; tmp != NULL ; tmp = tmp->b_next)
		if (tmp->b_dev==dev && tmp->b_blocknr==block)
			return tmp;
	return NULL;
}

/*
 * 代码为什么会是这样子的？我听见你问... 原因是竞争条件。由于我们没有对
 * 缓冲区上锁（除非我们正在读取它们中的数据），那么当我们（进程）睡眠时
 * 缓冲区可能会发生一些问题（例如一个读错误将导致该缓冲区出错）。目前
 * 这种情况实际上是不会发生的，但处理的代码已经准备好了。
 */
struct buffer_head * get_hash_table(int dev, int block)
{
	struct buffer_head * bh;

	for (;;) {
		// 在高速缓冲中寻找给定设备和指定块的缓冲区，如果没有找到则返回NULL，退出。
		if (!(bh=find_buffer(dev,block)))
			return NULL;
		// 对该缓冲区增加引用计数，并等待该缓冲区解锁（如果已被上锁）。
		bh->b_count++;
		wait_on_buffer(bh);
		// 由于经过了睡眠状态，因此有必要再验证该缓冲区块的正确性，并返回缓冲区头指针。
		if (bh->b_dev == dev && bh->b_blocknr == block)
			return bh;
// 如果该缓冲区所属的设备号或块号在睡眠时发生了改变，则撤消对它的引用计数，重新寻找。
		bh->b_count--;
	}
}

/*
 * OK，下面是getblk 函数，该函数的逻辑并不是很清晰，同样也是因为要考虑
 * 竞争条件问题。其中大部分代码很少用到，(例如重复操作语句)，因此它应该
 * 比看上去的样子有效得多。
 *
 * 算法已经作了改变：希望能更好，而且一个难以琢磨的错误已经去除。
 */
// 下面宏定义用于同时判断缓冲区的修改标志和锁定标志，并且定义修改标志的权重要比锁定标志大。
#define BADNESS(bh) (((bh)->b_dirt<<1)+(bh)->b_lock)
//// 取高速缓冲中指定的缓冲区。
// 检查所指定的缓冲区是否已经在高速缓冲中，如果不在，就需要在高速缓冲中建立一个对应的新项。
// 返回相应缓冲区头指针。
struct buffer_head * getblk(int dev,int block)
{
	struct buffer_head * tmp, * bh;

repeat:
	// 搜索hash 表，如果指定块已经在高速缓冲中，则返回对应缓冲区头指针，退出。
	if (bh = get_hash_table(dev,block))
		return bh;
// 扫描空闲数据块链表，寻找空闲缓冲区。
// 首先让tmp 指向空闲链表的第一个空闲缓冲区头。
	tmp = free_list;
	do {
// 如果该缓冲区正被使用（引用计数不等于0），则继续扫描下一项。
		if (tmp->b_count)
			continue;
// 如果缓冲头指针bh 为空，或者tmp 所指缓冲头的标志(修改、锁定)权重小于bh 头标志的权重，
// 则让bh 指向该tmp 缓冲区头。如果该tmp 缓冲区头表明缓冲区既没有修改也没有锁定标志置位，
// 则说明已为指定设备上的块取得对应的高速缓冲区，则退出循环。
		if (!bh || BADNESS(tmp)<BADNESS(bh)) {
			bh = tmp;
			if (!BADNESS(tmp))
				break;
		}
/* 重复操作直到找到适合的缓冲区 */
	} while ((tmp = tmp->b_next_free) != free_list);
// 如果所有缓冲区都正被使用（所有缓冲区的头部引用计数都>0），
// 则睡眠，等待有空闲的缓冲区可用。
	if (!bh) {
		sleep_on(&buffer_wait);
		goto repeat;
	}
	// 等待该缓冲区解锁（如果已被上锁的话）。
	wait_on_buffer(bh);
	// 如果该缓冲区又被其它任务使用的话，只好重复上述过程。
	if (bh->b_count)
		goto repeat;
// 如果该缓冲区已被修改，则将数据写盘，并再次等待缓冲区解锁。如果该缓冲区又被其它任务使用
// 的话，只好再重复上述过程。
	while (bh->b_dirt) {
		sync_dev(bh->b_dev);
		wait_on_buffer(bh);
		if (bh->b_count)
			goto repeat;
	}
/* 注意！！当进程为了等待该缓冲块而睡眠时，其它进程可能已经将该缓冲块 */
/* 加入进高速缓冲中，所以要对此进行检查。 */
// 在高速缓冲hash 表中检查指定设备和块的缓冲区是否已经被加入进去。如果是的话，就再次重复
// 上述过程。
	if (find_buffer(dev,block))
		goto repeat;
/* OK，最终我们知道该缓冲区是指定参数的唯一一块， */
/* 而且还没有被使用(b_count=0)，未被上锁(b_lock=0)，并且是干净的（未被修改的） */
// 于是让我们占用此缓冲区。置引用计数为1，复位修改标志和有效(更新)标志。
	bh->b_count=1;
	bh->b_dirt=0;
	bh->b_uptodate=0;
// 从hash 队列和空闲块链表中移出该缓冲区头，让该缓冲区用于指定设备和其上的指定块。
	remove_from_queues(bh);
	bh->b_dev=dev;
	bh->b_blocknr=block;
// 然后根据此新的设备号和块号重新插入空闲链表和hash 队列新位置处。并最终返回缓冲头指针。
	insert_into_queues(bh);
	return bh;
}

//// 释放指定的缓冲区。
// 等待该缓冲区解锁。引用计数递减1。唤醒等待空闲缓冲区的进程。
void brelse(struct buffer_head * buf)
{
	if (!buf)		// 如果缓冲头指针无效则返回。
		return;
	wait_on_buffer(buf);
	if (!(buf->b_count--))
		panic("Trying to free free buffer");
	wake_up(&buffer_wait);
}

/*
 * 从设备上读取指定的数据块并返回含有数据的缓冲区。如果指定的块不存在
 * 则返回NULL。
 */
//// 从指定设备上读取指定的数据块。
struct buffer_head * bread(int dev,int block)
{
	struct buffer_head * bh;

// 在高速缓冲中申请一块缓冲区。如果返回值是NULL 指针，表示内核出错，死机。
	if (!(bh=getblk(dev,block)))
		panic("bread: getblk returned NULL\n");
// 如果该缓冲区中的数据是有效的（已更新的）可以直接使用，则返回。
	if (bh->b_uptodate)
		return bh;
// 否则调用ll_rw_block()函数，产生读设备块请求。并等待缓冲区解锁。
	ll_rw_block(READ,bh);
	wait_on_buffer(bh);
// 如果该缓冲区已更新，则返回缓冲区头指针，退出。
	if (bh->b_uptodate)
		return bh;
// 否则表明读设备操作失败，释放该缓冲区，返回NULL 指针，退出。
	brelse(bh);
	return NULL;
}

//// 复制内存块。
// 从from 地址复制一块数据到to 位置。
extern __inline void COPYBLK(char* from, char* to)
{_asm{
	pushf
	mov ecx,BLOCK_SIZE/4
	mov esi,from
	mov edi,to
	cld
	rep movsd
	popf
}}
/*#define COPYBLK(from,to) \
__asm__("cld\n\t" \
	"rep\n\t" \
	"movsl\n\t" \
	::"c" (BLOCK_SIZE/4),"S" (from),"D" (to) \
	:"cx","di","si")*/

/*
 * bread_page 一次读四个缓冲块内容读到内存指定的地址。它是一个完整的函数，
 * 因为同时读取四块可以获得速度上的好处，不用等着读一块，再读一块了。
 */
//// 读设备上一个页面（4 个缓冲块）的内容到内存指定的地址。
void bread_page(unsigned long address,int dev,int b[4])
{
	struct buffer_head * bh[4];
	int i;

// 循环执行4 次，读一页内容。
	for (i=0 ; i<4 ; i++)
		if (b[i]) {
// 取高速缓冲中指定设备和块号的缓冲区，如果该缓冲区数据无效则产生读设备请求。
			if (bh[i] = getblk(dev,b[i]))
				if (!bh[i]->b_uptodate)
					ll_rw_block(READ,bh[i]);
		} else
			bh[i] = NULL;
// 将4 块缓冲区上的内容顺序复制到指定地址处。
	for (i=0 ; i<4 ; i++,address += BLOCK_SIZE)
		if (bh[i]) {
			wait_on_buffer(bh[i]);	// 等待缓冲区解锁(如果已被上锁的话)。
			if (bh[i]->b_uptodate)	// 如果该缓冲区中数据有效的话，则复制。
				COPYBLK(bh[i]->b_data,(char *)address);
			brelse(bh[i]);		// 释放该缓冲区。
		}
}

/*
 * OK，breada 可以象bread 一样使用，但会另外预读一些块。该函数参数列表
 * 需要使用一个负数来表明参数列表的结束。
 */
//// 从指定设备读取指定的一些块。
// 成功时返回第1 块的缓冲区头指针，否则返回NULL。
struct buffer_head * breada(int dev,int first, ...)
{
	va_list args;
	struct buffer_head * bh, *tmp;

// 取可变参数表中第1 个参数（块号）。
	va_start(args,first);
// 取高速缓冲中指定设备和块号的缓冲区。如果该缓冲区数据无效，则发出读设备数据块请求。
	if (!(bh=getblk(dev,first)))
		panic("bread: getblk returned NULL\n");
	if (!bh->b_uptodate)
		ll_rw_block(READ,bh);
// 然后顺序取可变参数表中其它预读块号，并作与上面同样处理，但不引用。
	while ((first=va_arg(args,int))>=0) {
		tmp=getblk(dev,first);
		if (tmp) {
			if (!tmp->b_uptodate)
				ll_rw_block(READA,bh);
			tmp->b_count--;
		}
	}
// 可变参数表中所有参数处理完毕。等待第1 个缓冲区解锁（如果已被上锁）。
	va_end(args);
	wait_on_buffer(bh);
// 如果缓冲区中数据有效，则返回缓冲区头指针，退出。否则释放该缓冲区，返回NULL，退出。
	if (bh->b_uptodate)
		return bh;
	brelse(bh);
	return (NULL);
}

//// 缓冲区初始化函数。
// 参数buffer_end 是指定的缓冲区内存的末端。对于系统有16MB 内存，则缓冲区末端设置为4MB。
// 对于系统有8MB 内存，缓冲区末端设置为2MB。
void buffer_init(long buffer_end)
{
	struct buffer_head * h = start_buffer;
	void * b;
	int i;

// 如果缓冲区高端等于1Mb，则由于从640KB-1MB 被显示内存和BIOS 占用，因此实际可用缓冲区内存
// 高端应该是640KB。否则内存高端一定大于1MB。
	if (buffer_end == 1<<20)
		b = (void *) (640*1024);
	else
		b = (void *) buffer_end;
// 这段代码用于初始化缓冲区，建立空闲缓冲区环链表，并获取系统中缓冲块的数目。
// 操作的过程是从缓冲区高端开始划分1K 大小的缓冲块，与此同时在缓冲区低端建立描述该缓冲块
// 的结构buffer_head，并将这些buffer_head 组成双向链表。
// h 是指向缓冲头结构的指针，而h+1 是指向内存地址连续的下一个缓冲头地址，也可以说是指向h
// 缓冲头的末端外。为了保证有足够长度的内存来存储一个缓冲头结构，需要b 所指向的内存块
// 地址>= h 缓冲头的末端，也即要>=h+1。
	while ( (b = (char*)b - BLOCK_SIZE) >= ((void *) (h+1)) ) {
		h->b_dev = 0;			// 使用该缓冲区的设备号。
		h->b_dirt = 0;			// 脏标志，也即缓冲区修改标志。
		h->b_count = 0;			// 该缓冲区引用计数。
		h->b_lock = 0;			// 缓冲区锁定标志。
		h->b_uptodate = 0;		// 缓冲区更新标志（或称数据有效标志）。
		h->b_wait = NULL;		// 指向等待该缓冲区解锁的进程。
		h->b_next = NULL;		// 指向具有相同hash 值的下一个缓冲头。
		h->b_prev = NULL;		// 指向具有相同hash 值的前一个缓冲头。
		h->b_data = (char *) b;	// 指向对应缓冲区数据块（1024 字节）。
		h->b_prev_free = h-1;	// 指向链表中前一项。
		h->b_next_free = h+1;	// 指向链表中下一项。
		h++;					// h 指向下一新缓冲头位置。
		NR_BUFFERS++;			// 缓冲区块数累加。
		if (b == (void *) 0x100000)		// 如果地址b 递减到等于1MB，则跳过384KB，
			b = (void *) 0xA0000;		// 让b 指向地址0xA0000(640KB)处。
	}
	h--;			// 让h 指向最后一个有效缓冲头。
	free_list = start_buffer;		// 让空闲链表头指向头一个缓冲区头。
	free_list->b_prev_free = h;		// 链表头的b_prev_free 指向前一项（即最后一项）。
	h->b_next_free = free_list;		// h 的下一项指针指向第一项，形成一个环链。
	// 初始化hash 表（哈希表、散列表），置表中所有的指针为NULL。
	for (i=0;i<NR_HASH;i++)
		hash_table[i]=NULL;
}	
