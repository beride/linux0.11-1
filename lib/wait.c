/*
 *  linux/lib/wait.c
 *
 *  (C) 1991  Linus Torvalds
 * 
 *     该程序包括函数waitpid()和wait()。这两个函数允许进程获取与其子进程之一的状态信息。
 * 各种选项允许获取已经终止或停止的子进程状态信息。如果存在两个或两个以上子进程的状态
 * 信息，则报告的顺序是不指定的。
 *     wait()将挂起当前进程，直到其子进程之一退出（终止），或者收到要求终止该进程的信号，
 * 或者是需要调用一个信号句柄（信号处理程序）
 *     waitpid()挂起当前进程，直到pid 指定的子进程退出（终止）或者收到要求终止该进程的
 * 信号，或者是需要调用一个信号句柄（信号处理程序）。
 *     如果pid= -1，options=0，则waitpid()的作用与wait()函数一样。否则其行为将随pid 和
 * options 参数的不同而不同。（参见kernel/exit.c,142）
 */
#include <set_seg.h>

#define __LIBRARY__
// Linux 标准头文件。定义了各种符号常数和类型，并申明了各种函数。
// 如定义了__LIBRARY__，则还包括系统调用号和内嵌汇编_syscall0()等。
#include <unistd.h>
// 等待调用头文件。定义系统调用wait()和waitpid()及相关常数符号。
#include <sys/wait.h>

//// 等待进程终止系统调用函数。
// 该下面宏结构对应于函数：pid_t waitpid(pid_t pid, int * wait_stat, int options)
//
// 参数：pid - 等待被终止进程的进程id，或者是用于指定特殊情况的其它特定数值；
//       wait_stat - 用于存放状态信息；options - WNOHANG 或WUNTRACED 或是0。
_syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)

//// wait()系统调用。直接调用waitpid()函数。
pid_t wait(int * wait_stat)
{
	return waitpid(-1,wait_stat,0);
}
