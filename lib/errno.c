/*
 *  linux/lib/errno.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

// 该程序仅定义了一个出错号变量errno。用于在函数调用失败时存放出错号。
int errno = 0;
