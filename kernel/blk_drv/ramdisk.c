/* passed
* linux/kernel/blk_drv/ramdisk.c
*
* Written by Theodore Ts'o, 12/2/91
*/
#include <set_seg.h>

/* 由Theodore Ts'o 编制，12/2/91
*/
// Theodore Ts'o (Ted Ts'o)是linux 社区中的著名人物。Linux 在世界范围内的流行也有他很大的
// 功劳，早在Linux 操作系统刚问世时，他就怀着极大的热情为linux 的发展提供了maillist，并
// 在北美洲地区最早设立了linux 的ftp 站点（tsx-11.mit.edu），而且至今仍然为广大linux 用户
// 提供服务。他对linux 作出的最大贡献之一是提出并实现了ext2 文件系统。该文件系统已成为
// linux 世界中事实上的文件系统标准。最近他又推出了ext3 文件系统，大大提高了文件系统的
// 稳定性和访问效率。作为对他的推崇，第97 期（2002 年5 月）的linuxjournal 期刊将他作为
// 了封面人物，并对他进行了采访。目前，他为IBM linux 技术中心工作，并从事着有关LSB
// (Linux Standard Base)等方面的工作。(他的主页：http://thunk.org/tytso/)

#include <string.h>		// 字符串头文件。主要定义了一些有关字符串操作的嵌入函数。

#include <linux/config.h>	// 内核配置头文件。定义键盘语言和硬盘类型（HD_TYPE）可选项。
#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/fs.h>		// 文件系统头文件。定义文件表结构（file,buffer_head,m_inode 等）。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/system.h>		// 系统头文件。定义了设置或修改描述符/中断门等的嵌入式汇编宏。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。
#include <asm/memory.h>		// 内存拷贝头文件。含有memcpy()嵌入式汇编宏函数。

#define MAJOR_NR 1		// 内存主设备号是1。
#include "blk.h"

char *rd_start = 0;			// 虚拟盘在内存中的起始位置。在52 行初始化函数rd_init()中
// 确定。参见(init/main.c,124)（缩写rd_代表ramdisk_）。
int rd_length = 0;		// 虚拟盘所占内存大小（字节）。

// 执行虚拟盘(ramdisk)读写操作。程序结构与do_hd_request()类似(kernel/blk_drv/hd.c,294)。
void
do_rd_request (void)
{
	int len;
	char *addr;

	INIT_REQUEST;			// 检测请求的合法性(参见kernel/blk_drv/blk.h,127)。
// 下面语句取得ramdisk 的起始扇区对应的内存起始位置和内存长度。
// 其中sector << 9 表示sector * 512，CURRENT 定义为(blk_dev[MAJOR_NR].current_request)。
	addr = rd_start + (CURRENT->sector << 9);
	len = CURRENT->nr_sectors << 9;
// 如果子设备号不为1 或者对应内存起始位置>虚拟盘末尾，则结束该请求，并跳转到repeat 处
// （定义在28 行的INIT_REQUEST 内开始处）。
	if ((MINOR (CURRENT->dev) != 1) || (addr + len > rd_start + rd_length))
	{
		end_request (0);
		goto repeat;
	}
// 如果是写命令(WRITE)，则将请求项中缓冲区的内容复制到addr 处，长度为len 字节。
	if (CURRENT->cmd == WRITE)
	{
		(void) memcpy (addr, CURRENT->buffer, len);
// 如果是读命令(READ)，则将addr 开始的内容复制到请求项中缓冲区中，长度为len 字节。
	}
	else if (CURRENT->cmd == READ)
	{
		(void) memcpy (CURRENT->buffer, addr, len);
// 否则显示命令不存在，死机。
	}
	else
		panic ("unknown ramdisk-command");
// 请求项成功后处理，置更新标志。并继续处理本设备的下一请求项。
	end_request (1);
	goto repeat;
}

/* 返回内存虚拟盘ramdisk 所需的内存量 */
// 虚拟盘初始化函数。确定虚拟盘在内存中的起始地址，长度。并对整个虚拟盘区清零。
long
rd_init (long mem_start, int length)
{
	int i;
	char *cp;

	blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;	// do_rd_request()。
	rd_start = (char *) mem_start;
	rd_length = length;
	cp = rd_start;
	for (i = 0; i < length; i++)
		*cp++ = '\0';
	return (length);
}

/*
* 如果根文件系统设备(root device)是ramdisk 的话，则尝试加载它。root device 原先是指向
* 软盘的，我们将它改成指向ramdisk。
*/
//// 加载根文件系统到ramdisk。
void
rd_load (void)
{
	struct buffer_head *bh;
	struct super_block s;
	int block = 256;		/* Start at block 256 */
	int i = 1;
	int nblocks;
	char *cp;			/* Move pointer */

	if (!rd_length)		// 如果ramdisk 的长度为零，则退出。
		return;
	printk ("Ram disk: %d bytes, starting at 0x%x\n", rd_length, (int) rd_start);	// 显示ramdisk 的大小以及内存起始位置。
	if (MAJOR (ROOT_DEV) != 2)	// 如果此时根文件设备不是软盘，则退出。
		return;
// 读软盘块256+1,256,256+2。breada()用于读取指定的数据块，并标出还需要读的块，然后返回
// 含有数据块的缓冲区指针。如果返回NULL，则表示数据块不可读(fs/buffer.c,322)。
// 这里block+1 是指磁盘上的超级块。
	bh = breada (ROOT_DEV, block + 1, block, block + 2, -1);
	if (!bh)
	{
		printk ("Disk error while looking for ramdisk!\n");
		return;
	}
// 将s 指向缓冲区中的磁盘超级块。(d_super_block 磁盘中超级块结构)。
	*((struct d_super_block *) &s) = *((struct d_super_block *) bh->b_data);
	brelse (bh);			// [?? 为什么数据没有复制就立刻释放呢？]
	if (s.s_magic != SUPER_MAGIC)	// 如果超级块中魔数不对，则说明不是minix 文件系统。
		return;/* 磁盘中没有ramdisk 映像文件，退出执行通常的软盘引导 */
// 块数 = 逻辑块数(区段数) * 2^(每区段块数的次方)。
// 如果数据块数大于内存中虚拟盘所能容纳的块数，则不能加载，显示出错信息并返回。否则显示
// 加载数据块信息。
	nblocks = s.s_nzones << s.s_log_zone_size;
	if (nblocks > (rd_length >> BLOCK_SIZE_BITS))
	{
		printk ("Ram disk image too big! (%d blocks, %d avail)\n",
				nblocks, rd_length >> BLOCK_SIZE_BITS);
		return;
	}
	printk ("Loading %d bytes into ram disk... 0000k",
	nblocks << BLOCK_SIZE_BITS);
// cp 指向虚拟盘起始处，然后将磁盘上的根文件系统映象文件复制到虚拟盘上。
	cp = rd_start;
	while (nblocks)
	{
		if (nblocks > 2)		// 如果需读取的块数多于3 快则采用超前预读方式读数据块。
			bh = breada (ROOT_DEV, block, block + 1, block + 2, -1);
		else			// 否则就单块读取。
			bh = bread (ROOT_DEV, block);
		if (!bh)
		{
			printk ("I/O error on block %d, aborting load\n", block);
			return;
		}
		(void) memcpy (cp, bh->b_data, BLOCK_SIZE);	// 将缓冲区中的数据复制到cp 处。
		brelse (bh);		// 释放缓冲区。
		printk ("\010\010\010\010\010%4dk", i);	// 打印加载块计数值。
		cp += BLOCK_SIZE;		// 虚拟盘指针前移。
		block++;
		nblocks--;
		i++;
	}
	printk ("\010\010\010\010\010done \n");
	ROOT_DEV = 0x0101;		// 修改ROOT_DEV 使其指向虚拟盘ramdisk。
}
