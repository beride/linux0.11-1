/* passed
* linux/kernel/panic.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

/*
* This function is used through-out the kernel (includeinh mm and fs)
* to indicate a major problem.
*/
/*
* 该函数在整个内核中使用（包括在 头文件*.h, 内存管理程序mm 和文件系统fs 中），
* 用以指出主要的出错问题。
*/
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。

void sys_sync (void);		/* 实际上是整型int (fs/buffer.c,44) */

// 该函数用来显示内核中出现的重大错误信息，并运行文件系统同步函数，然后进入死循环 -- 死机。
// 如果当前进程是任务0 的话，还说明是交换任务出错，并且还没有运行文件系统同步函数。
void panic (const char *s)
{
	printk ("Kernel panic: %s\n\r", s);
	if (current == task[0])
		printk ("In swapper task - not syncing\n\r");
	else
		sys_sync ();
	for (;;);
}
