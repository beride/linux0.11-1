/*
* 'tty.h'中定义了tty_io.c 程序使用的某些结构和其它一些定义。
*
* 注意！在修改这里的定义时，一定要检查rs_io.s 或con_io.s 程序中不会出现问题。
* 在系统中有些常量是直接写在程序中的（主要是一些tty_queue 中的偏移值）。
*/
#ifndef _TTY_H
#define _TTY_H

#include <termios.h>		// 终端输入输出函数头文件。主要定义控制异步通信口的终端接口。

#define TTY_BUF_SIZE 1024	// tty 缓冲区大小。

// tty 等待队列数据结构。
struct tty_queue
{
  unsigned long data;		// 等待队列缓冲区中当前数据指针字符数[??]）。
// 对于串口终端，则存放串行端口地址。
  unsigned long head;		// 缓冲区中数据头指针。
  unsigned long tail;		// 缓冲区中数据尾指针。
  struct task_struct *proc_list;	// 等待进程列表。
  char buf[TTY_BUF_SIZE];	// 队列的缓冲区。
};

// 以下定义了tty 等待队列中缓冲区操作宏函数。（tail 在前，head 在后）。
// a 缓冲区指针前移1 字节，并循环。
#define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
// a 缓冲区指针后退1 字节，并循环。
#define DEC(a) ((a) = ((a)-1) & (TTY_BUF_SIZE-1))
// 清空指定队列的缓冲区。
#define EMPTY(a) ((a).head == (a).tail)
// 缓冲区还可存放字符的长度（空闲区长度）。
#define LEFT(a) (((a).tail-(a).head-1)&(TTY_BUF_SIZE-1))
// 缓冲区中最后一个位置。
#define LAST(a) ((a).buf[(TTY_BUF_SIZE-1)&((a).head-1)])
// 缓冲区满（如果为1 的话）。
#define FULL(a) (!LEFT(a))
// 缓冲区中已存放字符的长度。
#define CHARS(a) (((a).head-(a).tail)&(TTY_BUF_SIZE-1))
// 从queue 队列项缓冲区中取一字符(从tail 处，并且tail+=1)。
#define GETCH(queue,c) \
(void)(c=(queue).buf[(queue).tail],INC((queue).tail))
//(void)({c=(queue).buf[(queue).tail];INC((queue).tail);})
// 往queue 队列项缓冲区中放置一字符（在head 处，并且head+=1）。
#define PUTCH(c,queue) \
(void)( (queue).buf[(queue).head]=(c), INC((queue).head) )
//(void)({(queue).buf[(queue).head]=(c);INC((queue).head);})

// 判断终端键盘字符类型。
#define INTR_CHAR(tty) ((tty)->termios.c_cc[VINTR])	// 中断符。
#define QUIT_CHAR(tty) ((tty)->termios.c_cc[VQUIT])	// 退出符。
#define ERASE_CHAR(tty) ((tty)->termios.c_cc[VERASE])	// 削除符。
#define KILL_CHAR(tty) ((tty)->termios.c_cc[VKILL])	// 终止符。
#define EOF_CHAR(tty) ((tty)->termios.c_cc[VEOF])	// 文件结束符。
#define START_CHAR(tty) ((tty)->termios.c_cc[VSTART])	// 开始符。
#define STOP_CHAR(tty) ((tty)->termios.c_cc[VSTOP])	// 结束符。
#define SUSPEND_CHAR(tty) ((tty)->termios.c_cc[VSUSP])	// 挂起符。

// tty 数据结构。
struct tty_struct
{
  struct termios termios;	// 终端io 属性和控制字符数据结构。
  int pgrp;			// 所属进程组。
  int stopped;			// 停止标志。
  void (*write) (struct tty_struct * tty);	// tty 写函数指针。
  struct tty_queue read_q;	// tty 读队列。
  struct tty_queue write_q;	// tty 写队列。
  struct tty_queue secondary;	// tty 辅助队列(存放规范模式字符序列)，
};				// 可称为规范(熟)模式队列。

extern struct tty_struct tty_table[];	// tty 结构数组。

/* intr=^C quit=^| erase=del kill=^U
eof=^D vtime=\0 vmin=\1 sxtc=\0
start=^Q stop=^S susp=^Z eol=\0
reprint=^R discard=^U werase=^W lnext=^V
eol2=\0
*/
/* 中断intr=^C 退出quit=^| 删除erase=del 终止kill=^U
* 文件结束eof=^D vtime=\0 vmin=\1 sxtc=\0
* 开始start=^Q 停止stop=^S 挂起susp=^Z 行结束eol=\0
* 重显reprint=^R 丢弃discard=^U werase=^W lnext=^V
* 行结束eol2=\0
*/
// 控制字符对应的ASCII 码值。[8 进制]
#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"

void rs_init (void);		// 异步串行通信初始化。(kernel/chr_drv/serial.c, 37)
void con_init (void);		// 控制终端初始化。 (kernel/chr_drv/console.c, 617)
void tty_init (void);		// tty 初始化。 (kernel/chr_drv/tty_io.c, 105)

int tty_read (unsigned c, char *buf, int n);	// (kernel/chr_drv/tty_io.c, 230)
int tty_write (unsigned c, char *buf, int n);	// (kernel/chr_drv/tty_io.c, 290)

void rs_write (struct tty_struct *tty);	// (kernel/chr_drv/serial.c, 53)
void con_write (struct tty_struct *tty);	// (kernel/chr_drv/console.c, 445)

void copy_to_cooked (struct tty_struct *tty);	// (kernel/chr_drv/tty_io.c, 145)

#endif
