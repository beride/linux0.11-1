/* passed
 *  linux/fs/file_table.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

// 文件系统头文件。定义文件表结构（file,buffer_head,m_inode 等）。
#include <linux/fs.h>

struct file file_table[NR_FILE] = {0};// 文件表数组(64 项)。
