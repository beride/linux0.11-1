/*
 *  linux/lib/close.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

#define __LIBRARY__
// Linux 标准头文件。定义了各种符号常数和类型，并申明了各种函数。
// 如定义了__LIBRARY__，则还包括系统调用号和内嵌汇编_syscall0()等。
#include <unistd.h>

// 关闭文件函数。
// 下面该调用宏函数对应：int close(int fd)。直接调用了系统中断int 0x80，参数是__NR_close。
// 其中fd 是文件描述符。
_syscall1(int,close,int,fd)
