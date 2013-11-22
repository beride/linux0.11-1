/* passed
* linux/kernel/serial.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

/*
* serial.c
*
* This module implements the rs232 io functions
* void rs_write(struct tty_struct * queue);
* void rs_init(void);
* and all interrupts pertaining to serial IO.
*/
/*
* serial.c
* 该程序用于实现rs232 的输入输出功能
* void rs_write(struct tty_struct *queue);
* void rs_init(void);
* 以及与传输IO 有关系的所有中断处理程序。
*/

#include <linux/tty.h>		// tty 头文件，定义了有关tty_io，串行通信方面的参数、常数。
#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <asm/system.h>		// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。
#include <asm/io.h>		// io 头文件。定义硬件端口输入/输出宏汇编语句。

#define WAKEUP_CHARS (TTY_BUF_SIZE/4)	// 当写队列中含有WAKEUP_CHARS 个字符时，就开始发送。

extern void rs1_interrupt (void);	// 串行口1 的中断处理程序(rs_io.s, 34)。
extern void rs2_interrupt (void);	// 串行口2 的中断处理程序(rs_io.s, 38)。

//// 初始化串行端口
// port: 串口1 - 0x3F8，串口2 - 0x2F8。
static void init (int port)
{
	outb_p (0x80, port + 3);	/* set DLAB of line control reg */
/* 设置线路控制寄存器的DLAB 位(位7) */
	outb_p (0x30, port);		/* LS of divisor (48 -> 2400 bps */
/* 发送波特率因子低字节，0x30->2400bps */
	outb_p (0x00, port + 1);	/* MS of divisor */
/* 发送波特率因子高字节，0x00 */
	outb_p (0x03, port + 3);	/* reset DLAB */
/* 复位DLAB 位，数据位为8 位 */
	outb_p (0x0b, port + 4);	/* set DTR,RTS, OUT_2 */
/* 设置DTR，RTS，辅助用户输出2 */
	outb_p (0x0d, port + 1);	/* enable all intrs but writes */
/* 除了写(写保持空)以外，允许所有中断源中断 */
	(void) inb (port);		/* read data port to reset things (?) */
/* 读数据口，以进行复位操作(?) */
}

//// 初始化串行中断程序和串行接口。
void
rs_init (void)
{
	set_intr_gate (0x24, rs1_interrupt);	// 设置串行口1 的中断门向量(硬件IRQ4 信号)。
	set_intr_gate (0x23, rs2_interrupt);	// 设置串行口2 的中断门向量(硬件IRQ3 信号)。
	init (tty_table[1].read_q.data);	// 初始化串行口1(.data 是端口号)。
	init (tty_table[2].read_q.data);	// 初始化串行口2。
	outb (inb_p (0x21) & 0xE7, 0x21);	// 允许主8259A 芯片的IRQ3，IRQ4 中断信号请求。
}

/*
* 在tty_write()已将数据放入输出(写)队列时会调用下面的子程序。必须首先
* 检查写队列是否为空，并相应设置中断寄存器。
*/
//// 串行数据发送输出。
// 实际上只是开启串行发送保持寄存器已空中断标志，在UART 将数据发送出去后允许发中断信号。
void rs_write (struct tty_struct *tty)
{
	cli ();			// 关中断。
// 如果写队列不空，则从0x3f9(或0x2f9) 首先读取中断允许寄存器内容，添上发送保持寄存器
// 中断允许标志（位1）后，再写回该寄存器。
	if (!EMPTY (tty->write_q))
		outb (inb_p (tty->write_q.data + 1) | 0x02, tty->write_q.data + 1);
	sti ();			// 开中断。
}
