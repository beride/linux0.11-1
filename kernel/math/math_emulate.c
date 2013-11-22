/* passed
* linux/kernel/math/math_emulate.c
*
* (C) 1991 Linus Torvalds
*/
/*
注意：signal.c和fork.c文件的编译选项内不能有vc变量优化选项/Og，因为这两个文件
	内的函数参数内包含了函数返回地址等内容。如果加了/Og选项，编译器就会在认为
	这些参数不再使用后占用该内存，导致函数返回时出错。
	math/math_emulate.c照理也应该这样，不过好像它没有把eip等参数优化掉:)
*/
#include <set_seg.h>

/*
* This directory should contain the math-emulation code.
* Currently only results in a signal.
*/
/*
* 该目录里应该包含数学仿真代码。目前仅产生一个信号。
*/

#include <signal.h>		// 信号头文件。定义信号符号常量，信号结构以及信号操作函数原型。

#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

//// 协处理器仿真函数。
// 中断处理程序调用的C 函数，参见(kernel/math/system_call.s，169 行)。
void
math_emulate (long edi, long esi, long ebp, long sys_call_ret,
	      long eax, long ebx, long ecx, long edx,
	      unsigned short fs, unsigned short es, unsigned short ds,
	      unsigned long eip, unsigned short cs, unsigned long eflags,
	      unsigned short ss, unsigned long esp)
{
	unsigned char first, second;

/* 0x0007 means user code space */
/* 0x0007 表示用户代码空间 */
// 选择符0x000F 表示在局部描述符表中描述符索引值=1，即代码空间。如果段寄存器cs 不等于0x000F
// 则表示cs 一定是内核代码选择符，是在内核代码空间，则出错，显示此时的cs:eip 值，并显示信息
// “内核中需要数学仿真”，然后进入死机状态。
	if (cs != 0x000F)
	{
		printk ("math_emulate: %04x:%08x\n\r", cs, eip);
		panic ("Math emulation needed in kernel");
	}
// 取用户数据区堆栈数据first 和second，显示这些数据，并给进程设置浮点异常信号SIGFPE。
	first = get_fs_byte ((char *) ((*&eip)++));
	second = get_fs_byte ((char *) ((*&eip)++));
	printk ("%04x:%08x %02x %02x\n\r", cs, eip - 2, first, second);
	current->signal |= 1 << (SIGFPE - 1);
}

//// 协处理器出错处理函数。
// 中断处理程序调用的C 函数，参见(kernel/math/system_call.s，145 行)。
void
math_error (void)
{
// 协处理器指令。(以非等待形式)清除所有异常标志、忙标志和状态字位7。
//  __asm__ ("fnclex");
	_asm fnclex;
// 如果上个任务使用过协处理器，则向上个任务发送协处理器异常信号。
	if (last_task_used_math)
		last_task_used_math->signal |= 1 << (SIGFPE - 1);
}
