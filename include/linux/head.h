#ifndef _HEAD_H
#define _HEAD_H

typedef struct desc_struct
{				// 定义了段描述符的数据结构。该结构仅说明每个描述
  unsigned long a, b;		// 符是由8 个字节构成，每个描述符表共有256 项。
}
desc_table[256];

extern unsigned long pg_dir[1024];	// 内存页目录数组。每个目录项为4 字节。从物理地址0 开始。
//#define pg_dir ((unsigned long*)0)
extern desc_table idt, gdt;	// 中断描述符表，全局描述符表。

#define GDT_NUL 0		// 全局描述符表的第0 项，不用。
#define GDT_CODE 1		// 第1 项，是内核代码段描述符项。
#define GDT_DATA 2		// 第2 项，是内核数据段描述符项。
#define GDT_TMP 3		// 第3 项，系统段描述符，Linux 没有使用。

#define LDT_NUL 0		// 每个局部描述符表的第0 项，不用。
#define LDT_CODE 1		// 第1 项，是用户程序代码段描述符项。
#define LDT_DATA 2		// 第2 项，是用户程序数据段描述符项。

#endif
