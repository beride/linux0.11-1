/* passed
* linux/kernel/chr_drv/tty_ioctl.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

#include <errno.h>		// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。
#include <termios.h>		// 终端输入输出函数头文件。主要定义控制异步通信口的终端接口。

#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/tty.h>		// tty 头文件，定义了有关tty_io，串行通信方面的参数、常数。

#include <asm/io.h>		// io 头文件。定义硬件端口输入/输出宏汇编语句。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。
#include <asm/system.h>		// 系统头文件。定义了设置或修改描述符/中断门等的嵌入式汇编宏。

// 这是波特率因子数组（或称为除数数组）。波特率与波特率因子的对应关系参见列表后的说明。
static unsigned short quotient[] = {
	0, 2304, 1536, 1047, 857,
	768, 576, 384, 192, 96,
	64, 48, 24, 12, 6, 3
};

//// 修改传输速率。
// 参数：tty - 终端对应的tty 数据结构。
// 在除数锁存标志DLAB(线路控制寄存器位7)置位情况下，通过端口0x3f8 和0x3f9 向UART 分别写入
// 波特率因子低字节和高字节。
static void
change_speed (struct tty_struct *tty)
{
	unsigned short port, quot;

// 对于串口终端，其tty 结构的读缓冲队列data 字段存放的是串行端口号(0x3f8 或0x2f8)。
	if (!(port = tty->read_q.data))
		return;
// 从tty 的termios 结构控制模式标志集中取得设置的波特率索引号，据此从波特率因子数组中取得
// 对应的波特率因子值。CBAUD 是控制模式标志集中波特率位屏蔽码。
	quot = quotient[tty->termios.c_cflag & CBAUD];
	cli ();			// 关中断。
	outb_p (0x80, port + 3);	/* set DLAB */// 首先设置除数锁定标志DLAB。
	outb_p (quot & 0xff, port);	/* LS of divisor */// 输出因子低字节。
	outb_p (quot >> 8, port + 1);	/* MS of divisor */// 输出因子高字节。
	outb (0x03, port + 3);	/* reset DLAB */// 复位DLAB。
	sti ();			// 开中断。
}

//// 刷新tty 缓冲队列。
// 参数：gueue - 指定的缓冲队列指针。
// 令缓冲队列的头指针等于尾指针，从而达到清空缓冲区(零字符)的目的。
static void
flush (struct tty_queue *queue)
{
	cli ();
	queue->head = queue->tail;
	sti ();
}

//// 等待字符发送出去。
static void
wait_until_sent (struct tty_struct *tty)
{
/* do nothing - not implemented *//* 什么都没做 - 还未实现 */
}

//// 发送BREAK 控制符。
static void
send_break (struct tty_struct *tty)
{
/* do nothing - not implemented *//* 什么都没做 - 还未实现 */
}

//// 取终端termios 结构信息。
// 参数：tty - 指定终端的tty 结构指针；termios - 用户数据区termios 结构缓冲区指针。
// 返回0 。
static int
get_termios (struct tty_struct *tty, struct termios *termios)
{
	int i;

// 首先验证一下用户的缓冲区指针所指内存区是否足够，如不够则分配内存。
	verify_area (termios, sizeof (*termios));
// 复制指定tty 结构中的termios 结构信息到用户 termios 结构缓冲区。
	for (i = 0; i < (sizeof (*termios)); i++)
		put_fs_byte (((char *) &tty->termios)[i], i + (char *) termios);
	return 0;
}

//// 设置终端termios 结构信息。
// 参数：tty - 指定终端的tty 结构指针；termios - 用户数据区termios 结构指针。
// 返回0 。
static int
set_termios (struct tty_struct *tty, struct termios *termios)
{
	int i;

// 首先复制用户数据区中termios 结构信息到指定tty 结构中。
	for (i = 0; i < (sizeof (*termios)); i++)
		((char *) &tty->termios)[i] = get_fs_byte (i + (char *) termios);
// 用户有可能已修改了tty 的串行口传输波特率，所以根据termios 结构中的控制模式标志c_cflag
// 修改串行芯片UART 的传输波特率。
	change_speed (tty);
	return 0;
}

//// 读取termio 结构中的信息。
// 参数：tty - 指定终端的tty 结构指针；termio - 用户数据区termio 结构缓冲区指针。
// 返回0。
static int
get_termio (struct tty_struct *tty, struct termio *termio)
{
	int i;
	struct termio tmp_termio;

// 首先验证一下用户的缓冲区指针所指内存区是否足够，如不够则分配内存。
	verify_area (termio, sizeof (*termio));
// 将termios 结构的信息复制到termio 结构中。目的是为了其中模式标志集的类型进行转换，也即
// 从termios 的长整数类型转换为termio 的短整数类型。
	tmp_termio.c_iflag = tty->termios.c_iflag;
	tmp_termio.c_oflag = tty->termios.c_oflag;
	tmp_termio.c_cflag = tty->termios.c_cflag;
	tmp_termio.c_lflag = tty->termios.c_lflag;
// 两种结构的c_line 和c_cc[]字段是完全相同的。
	tmp_termio.c_line = tty->termios.c_line;
	for (i = 0; i < NCC; i++)
		tmp_termio.c_cc[i] = tty->termios.c_cc[i];
// 最后复制指定tty 结构中的termio 结构信息到用户 termio 结构缓冲区。
	for (i = 0; i < (sizeof (*termio)); i++)
		put_fs_byte (((char *) &tmp_termio)[i], i + (char *) termio);
	return 0;
}

/*
* This only works as the 386 is low-byt-first
*/
/*
* 下面的termio 设置函数仅在386 低字节在前的方式下可用。
*/
//// 设置终端termio 结构信息。
// 参数：tty - 指定终端的tty 结构指针；termio - 用户数据区termio 结构指针。
// 将用户缓冲区termio 的信息复制到终端的termios 结构中。返回0 。
static int
set_termio (struct tty_struct *tty, struct termio *termio)
{
	int i;
	struct termio tmp_termio;

// 首先复制用户数据区中termio 结构信息到临时termio 结构中。
	for (i = 0; i < (sizeof (*termio)); i++)
		((char *) &tmp_termio)[i] = get_fs_byte (i + (char *) termio);
// 再将termio 结构的信息复制到tty 的termios 结构中。目的是为了其中模式标志集的类型进行转换，
// 也即从termio 的短整数类型转换成termios 的长整数类型。
	*(unsigned short *) &tty->termios.c_iflag = tmp_termio.c_iflag;
	*(unsigned short *) &tty->termios.c_oflag = tmp_termio.c_oflag;
	*(unsigned short *) &tty->termios.c_cflag = tmp_termio.c_cflag;
	*(unsigned short *) &tty->termios.c_lflag = tmp_termio.c_lflag;
// 两种结构的c_line 和c_cc[]字段是完全相同的。
	tty->termios.c_line = tmp_termio.c_line;
	for (i = 0; i < NCC; i++)
		tty->termios.c_cc[i] = tmp_termio.c_cc[i];
// 用户可能已修改了tty 的串行口传输波特率，所以根据termios 结构中的控制模式标志集c_cflag
// 修改串行芯片UART 的传输波特率。
	change_speed (tty);
	return 0;
}

//// tty 终端设备的ioctl 函数。
// 参数：dev - 设备号；cmd - ioctl 命令；arg - 操作参数指针。
int
tty_ioctl (int dev, int cmd, int arg)
{
	struct tty_struct *tty;
// 首先取tty 的子设备号。如果主设备号是5(tty 终端)，则进程的tty 字段即是子设备号；如果进程
// 的tty 子设备号是负数，表明该进程没有控制终端，也即不能发出该ioctl 调用，出错死机。
	if (MAJOR (dev) == 5)
	{
		dev = current->tty;
		if (dev < 0)
			panic ("tty_ioctl: dev<0");
// 否则直接从设备号中取出子设备号。
	}
	else
		dev = MINOR (dev);
// 子设备号可以是0(控制台终端)、1(串口1 终端)、2(串口2 终端)。
// 让tty 指向对应子设备号的tty 结构。
	tty = dev + tty_table;
// 根据tty 的ioctl 命令进行分别处理。
	switch (cmd)
	{
	case TCGETS:
//取相应终端termios 结构中的信息。
		return get_termios (tty, (struct termios *) arg);
	case TCSETSF:
// 在设置termios 的信息之前，需要先等待输出队列中所有数据处理完，并且刷新(清空)输入队列。
// 再设置。
		flush (&tty->read_q);	/* fallthrough */
	case TCSETSW:
// 在设置终端termios 的信息之前，需要先等待输出队列中所有数据处理完(耗尽)。对于修改参数
// 会影响输出的情况，就需要使用这种形式。
		wait_until_sent (tty);	/* fallthrough */
	case TCSETS:
// 设置相应终端termios 结构中的信息。
		return set_termios (tty, (struct termios *) arg);
	case TCGETA:
// 取相应终端termio 结构中的信息。
		return get_termio (tty, (struct termio *) arg);
	case TCSETAF:
// 在设置termio 的信息之前，需要先等待输出队列中所有数据处理完，并且刷新(清空)输入队列。
// 再设置。
		flush (&tty->read_q);	/* fallthrough */
	case TCSETAW:
// 在设置终端termio 的信息之前，需要先等待输出队列中所有数据处理完(耗尽)。对于修改参数
// 会影响输出的情况，就需要使用这种形式。
		wait_until_sent (tty);	/* fallthrough *//* 继续执行 */
	case TCSETA:
// 设置相应终端termio 结构中的信息。
		return set_termio (tty, (struct termio *) arg);
	case TCSBRK:
// 等待输出队列处理完毕(空)，如果参数值是0，则发送一个break。
		if (!arg)
		{
			wait_until_sent (tty);
			send_break (tty);
		}
		return 0;
	case TCXONC:
// 开始/停止控制。如果参数值是0，则挂起输出；如果是1，则重新开启挂起的输出；如果是2，则挂起
// 输入；如果是3，则重新开启挂起的输入。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TCFLSH:
//刷新已写输出但还没发送或已收但还没有读数据。如果参数是0，则刷新(清空)输入队列；如果是1，
// 则刷新输出队列；如果是2，则刷新输入和输出队列。
		if (arg == 0)
			flush (&tty->read_q);
		else if (arg == 1)
			flush (&tty->write_q);
		else if (arg == 2)
		{
			flush (&tty->read_q);
			flush (&tty->write_q);
		}
		else
			return -EINVAL;
		return 0;
	case TIOCEXCL:
// 设置终端串行线路专用模式。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCNXCL:
// 复位终端串行线路专用模式。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCSCTTY:
// 设置tty 为控制终端。(TIOCNOTTY - 禁止tty 为控制终端)。
		return -EINVAL;		/* set controlling term NI *//* 设置控制终端NI */
	case TIOCGPGRP:		// NI - Not Implemented。
// 读取指定终端设备进程的组id。首先验证用户缓冲区长度，然后复制tty 的pgrp 字段到用户缓冲区。
		verify_area ((void *) arg, 4);
		put_fs_long (tty->pgrp, (unsigned long *) arg);
		return 0;
	case TIOCSPGRP:
// 设置指定终端设备进程的组id。
		tty->pgrp = get_fs_long ((unsigned long *) arg);
		return 0;
	case TIOCOUTQ:
// 返回输出队列中还未送出的字符数。首先验证用户缓冲区长度，然后复制队列中字符数给用户。
		verify_area ((void *) arg, 4);
		put_fs_long (CHARS (tty->write_q), (unsigned long *) arg);
		return 0;
	case TIOCINQ:
// 返回输入队列中还未读取的字符数。首先验证用户缓冲区长度，然后复制队列中字符数给用户。
		verify_area ((void *) arg, 4);
		put_fs_long (CHARS (tty->secondary), (unsigned long *) arg);
		return 0;
	case TIOCSTI:
// 模拟终端输入。该命令以一个指向字符的指针作为参数，并假装该字符是在终端上键入的。用户必须
// 在该控制终端上具有超级用户权限或具有读许可权限。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCGWINSZ:
// 读取终端设备窗口大小信息（参见termios.h 中的winsize 结构）。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCSWINSZ:
// 设置终端设备窗口大小信息（参见winsize 结构）。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCMGET:
// 返回modem 状态控制引线的当前状态比特位标志集（参见termios.h 中185-196 行）。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCMBIS:
// 设置单个modem 状态控制引线的状态(true 或false)。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCMBIC:
// 复位单个modem 状态控制引线的状态。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCMSET:
// 设置modem 状态引线的状态。如果某一比特位置位，则modem 对应的状态引线将置为有效。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCGSOFTCAR:
// 读取软件载波检测标志(1 - 开启；0 - 关闭)。
		return -EINVAL;		/* not implemented *//* 未实现 */
	case TIOCSSOFTCAR:
// 设置软件载波检测标志(1 - 开启；0 - 关闭)。
		return -EINVAL;		/* not implemented *//* 未实现 */
	default:
		return -EINVAL;
	}
}
