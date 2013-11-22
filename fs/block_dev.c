/* passed
 *  linux/fs/block_dev.c
 *
 *  (C) 1991  Linus Torvalds
 * 
 *    block_dev.c 程序属于块设备文件数据访问操作类程序。该文件包括block_read()和
 * block_write()两个块设备读写函数。这两个函数是供系统调用函数read()和write()调用的，
 * 其它地方没有引用。
*/
#include <set_seg.h>

// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。
#include <errno.h>
// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/sched.h>
// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/kernel.h>
// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。
#include <asm/segment.h>
// 系统头文件。定义了设置或修改描述符/中断门等的嵌入式汇编宏。
#include <asm/system.h>

//// 数据块写函数- 向指定设备从给定偏移处写入指定长度字节数据。
// 参数：dev - 设备号；pos - 设备文件中偏移量指针；buf - 用户地址空间中缓冲区地址；
//       count - 要传送的字节数。
// 对于内核来说，写操作是向高速缓冲区中写入数据，什么时候数据最终写入设备是由高速缓冲管理
// 程序决定并处理的。另外，因为设备是以块为单位进行读写的，因此对于写开始位置不处于块起始
// 处时，需要先将开始字节所在的整个块读出，然后将需要写的数据从写开始处填写满该块，再将完
// 整的一块数据写盘（即交由高速缓冲程序去处理）。
int block_write(int dev, long * pos, char * buf, int count)
{
// 由pos 地址换算成开始读写块的块序号block。并求出需读第1 字节在该块中的偏移位置offset。
	int block = *pos >> BLOCK_SIZE_BITS;
	int offset = *pos & (BLOCK_SIZE-1);
	int chars;
	int written = 0;
	struct buffer_head * bh;
	register char * p;

// 针对要写入的字节数count，循环执行以下操作，直到全部写入。
	while (count > 0) {
// 计算在该块中可写入的字节数。如果需要写入的字节数填不满一块，则只需写count 字节。
		chars = BLOCK_SIZE - offset;
		if (chars > count)
			chars=count;
// 如果正好要写1 块数据，则直接申请1 块高速缓冲块，否则需要读入将被修改的数据块，并预读
// 下两块数据，然后将块号递增1。
		if (chars == BLOCK_SIZE)
			bh = getblk(dev,block);
		else
			bh = breada(dev,block,block+1,block+2,-1);
		block++;
// 如果缓冲块操作失败，则返回已写字节数，如果没有写入任何字节，则返回出错号（负数）。
		if (!bh)
			return written?written:-EIO;
// p 指向读出数据块中开始写的位置。若最后写入的数据不足一块，则需从块开始填写（修改）所需
// 的字节，因此这里需置offset 为零。
		p = offset + bh->b_data;
		offset = 0;
// 将文件中偏移指针前移已写字节数。累加已写字节数chars。传送计数值减去此次已传送字节数。
		*pos += chars;
		written += chars;
		count -= chars;
// 从用户缓冲区复制chars 字节到p 指向的高速缓冲区中开始写入的位置。
		while (chars-- > 0)
			*(p++) = get_fs_byte(buf++);
// 置该缓冲区块已修改标志，并释放该缓冲区（也即该缓冲区引用计数递减1）。
		bh->b_dirt = 1;
		brelse(bh);
	}
	return written;		// 返回已写入的字节数，正常退出。
}

//// 数据块读函数- 从指定设备和位置读入指定字节数的数据到高速缓冲中。
int block_read(int dev, unsigned long * pos, char * buf, int count)
{
// 由pos 地址换算成开始读写块的块序号block。并求出需读第1 字节在该块中的偏移位置offset。
	int block = *pos >> BLOCK_SIZE_BITS;
	int offset = *pos & (BLOCK_SIZE-1);
	int chars;
	int read = 0;
	struct buffer_head * bh;
	register char * p;

// 针对要读入的字节数count，循环执行以下操作，直到全部读入。
	while (count>0) {
// 计算在该块中需读入的字节数。如果需要读入的字节数不满一块，则只需读count 字节。
		chars = BLOCK_SIZE-offset;
		if (chars > count)
			chars = count;
// 读入需要的数据块，并预读下两块数据，如果读操作出错，则返回已读字节数，如果没有读入任何
// 字节，则返回出错号。然后将块号递增1。
		if (!(bh = breada(dev,block,block+1,block+2,-1)))
			return read?read:-EIO;
		block++;
// p 指向从设备读出数据块中需要读取的开始位置。若最后需要读取的数据不足一块，则需从块开始
// 读取所需的字节，因此这里需将offset 置零。
		p = offset + bh->b_data;
		offset = 0;
// 将文件中偏移指针前移已读出字节数chars。累加已读字节数。传送计数值减去此次已传送字节数。
		*pos += chars;
		read += chars;
		count -= chars;
// 从高速缓冲区中p 指向的开始位置复制chars 字节数据到用户缓冲区，并释放该高速缓冲区。
		while (chars-->0)
			put_fs_byte(*(p++),buf++);
		brelse(bh);
	}
	return read;		// 返回已读取的字节数，正常退出。
}
