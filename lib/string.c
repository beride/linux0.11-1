/*
 *  linux/lib/string.c
 *
 *  (C) 1991  Linus Torvalds
 * 所有字符串操作函数已经在string.h 中实现，因此string.c 程序仅包含string.h 头文件。


#ifndef __GNUC__            // 需要GNU 的C 编译器编译。
#error "I want gcc!"
#endif
 */
#include <set_seg.h>

#define extern
#define inline
#define __LIBRARY__
#include <string.h>
