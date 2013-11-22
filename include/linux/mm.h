#ifndef _MM_H
#define _MM_H

#define PAGE_SIZE 4096		// 定义内存页面的大小(字节数)。

// 取空闲页面函数。返回页面地址。扫描页面映射数组mem_map[]取空闲页面。
extern unsigned long get_free_page (void);
// 在指定物理地址处放置一页面。在页目录和页表中放置指定页面信息。
extern unsigned long put_page (unsigned long page, unsigned long address);
// 释放物理地址addr 开始的一页面内存。修改页面映射数组mem_map[]中引用次数信息。
extern void free_page (unsigned long addr);

#endif
