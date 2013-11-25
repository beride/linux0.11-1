/*
 *  linux/lib/setsid.c
 *
 *     该程序包括一个setsid()系统调用函数。如果调用的进程不是一个组的领导时，该函数用于
 * 创建一个新会话。则调用进程将成为该新会话的领导、新进程组的组领导，并且没有控制终端。
 * 调用进程的组id 和会话id 被设置成进程的PID(进程标识符)。调用进程将成为新进程组和新会
 * 话中的唯一进程。
 */
#include <set_seg.h>

#define __LIBRARY__
// Linux 标准头文件。定义了各种符号常数和类型，并申明了各种函数。
// 如定义了__LIBRARY__，则还包括系统调用号和内嵌汇编_syscall0()等。
#include <unistd.h>

//// 创建一个会话并设置进程组号。
// 下面系统调用宏对应于函数：pid_t setsid()。
// 返回：调用进程的会话标识符(session ID)。
_syscall0(pid_t,setsid)
