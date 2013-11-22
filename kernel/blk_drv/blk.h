#ifndef _BLK_H
#define _BLK_H

#define NR_BLK_DEV 7		// 块设备的数量。
/*
* 下面定义的NR_REQUEST 是请求队列中所包含的项数。
* 注意，读操作仅使用这些项低端的2/3；读操作优先处理。
*
* 32 项好象是一个合理的数字：已经足够从电梯算法中获得好处，
* 但当缓冲区在队列中而锁住时又不显得是很大的数。64 就看上
* 去太大了（当大量的写/同步操作运行时很容易引起长时间的暂停）。
*/
#define NR_REQUEST 32

/*
* OK，下面是request 结构的一个扩展形式，因而当实现以后，我们就可以在分页请求中
* 使用同样的request 结构。在分页处理中，'bh'是NULL，而'waiting'则用于等待读/写的完成。
*/
// 下面是请求队列中项的结构。其中如果dev=-1，则表示该项没有被使用。
struct request
{
  int dev;			/* -1 if no request */// 使用的设备号。
  int cmd;			/* READ or WRITE */// 命令(READ 或WRITE)。
  int errors;			//操作时产生的错误次数。
  unsigned long sector;		// 起始扇区。(1 块=2 扇区)
  unsigned long nr_sectors;	// 读/写扇区数。
  char *buffer;			// 数据缓冲区。
  struct task_struct *waiting;	// 任务等待操作执行完成的地方。
  struct buffer_head *bh;	// 缓冲区头指针(include/linux/fs.h,68)。
  struct request *next;		// 指向下一请求项。
};

/*
* 下面的定义用于电梯算法：注意读操作总是在写操作之前进行。
* 这是很自然的：读操作对时间的要求要比写严格得多。
*/
#define IN_ORDER(s1,s2) \
((s1)->cmd<(s2)->cmd || (s1)->cmd==(s2)->cmd && \
((s1)->dev < (s2)->dev || ((s1)->dev == (s2)->dev && \
(s1)->sector < (s2)->sector)))

// 块设备结构。
struct blk_dev_struct
{
  void (*request_fn) (void);	// 请求操作的函数指针。
  struct request *current_request;	// 请求信息结构。
};

extern struct blk_dev_struct blk_dev[NR_BLK_DEV];	// 块设备数组，每种块设备占用一项。
extern struct request request[NR_REQUEST];	// 请求队列数组。
extern struct task_struct *wait_for_request;	// 等待请求的任务结构。

#ifdef MAJOR_NR			// 主设备号。

/*
* 需要时加入条目。目前块设备仅支持硬盘和软盘（还有虚拟盘）。
*/

#if (MAJOR_NR == 1)		// RAM 盘的主设备号是1。根据这里的定义可以推理内存块主设备号也为1。
/* ram disk *//* RAM 盘（内存虚拟盘） */
#define DEVICE_NAME "ramdisk"	// 设备名称ramdisk。
#define DEVICE_REQUEST do_rd_request	// 设备请求函数do_rd_request()。
#define DEVICE_NR(device) ((device) & 7)	// 设备号(0--7)。
#define DEVICE_ON(device)	// 开启设备。虚拟盘无须开启和关闭。
#define DEVICE_OFF(device)	// 关闭设备。

#elif (MAJOR_NR == 2)		// 软驱的主设备号是2。
/* floppy */
#define DEVICE_NAME "floppy"	// 设备名称floppy。
#define DEVICE_INTR do_floppy	// 设备中断处理程序do_floppy()。
#define DEVICE_REQUEST do_fd_request	// 设备请求函数do_fd_request()。
#define DEVICE_NR(device) ((device) & 3)	// 设备号（0--3）。
#define DEVICE_ON(device) floppy_on(DEVICE_NR(device))	// 开启设备函数floppyon()。
#define DEVICE_OFF(device) floppy_off(DEVICE_NR(device))	// 关闭设备函数floppyoff()。

#elif (MAJOR_NR == 3)		// 硬盘主设备号是3。
/* harddisk */
#define DEVICE_NAME "harddisk"	// 硬盘名称harddisk。
#define DEVICE_INTR do_hd	// 设备中断处理程序do_hd()。
#define DEVICE_REQUEST do_hd_request	// 设备请求函数do_hd_request()。
#define DEVICE_NR(device) (MINOR(device)/5)	// 设备号（0--1）。每个硬盘可以有4 个分区。
#define DEVICE_ON(device)	// 硬盘一直在工作，无须开启和关闭。
#define DEVICE_OFF(device)

#elif
/* unknown blk device *//* 未知块设备 */
#error "unknown blk device"

#endif

#define CURRENT (blk_dev[MAJOR_NR].current_request)	// CURRENT 为指定主设备号的当前请求结构。
#define CURRENT_DEV DEVICE_NR(CURRENT->dev)	// CURRENT_DEV 为CURRENT 的设备号。

#ifdef DEVICE_INTR
void (*DEVICE_INTR) (void) = NULL;
#endif
static void (DEVICE_REQUEST) (void);

// 释放锁定的缓冲区。
extern _inline void
unlock_buffer (struct buffer_head *bh)
{
  if (!bh->b_lock)		// 如果指定的缓冲区bh 并没有被上锁，则显示警告信息。
    printk (DEVICE_NAME ": free buffer being unlocked\n");
  bh->b_lock = 0;		// 否则将该缓冲区解锁。
  wake_up (&bh->b_wait);	// 唤醒等待该缓冲区的进程。
}

// 结束请求。
extern _inline void
end_request (int uptodate)
{
  DEVICE_OFF (CURRENT->dev);	// 关闭设备。
  if (CURRENT->bh)
    {				// CURRENT 为指定主设备号的当前请求结构。
      CURRENT->bh->b_uptodate = uptodate;	// 置更新标志。
      unlock_buffer (CURRENT->bh);	// 解锁缓冲区。
    }
  if (!uptodate)
    {				// 如果更新标志为0 则显示设备错误信息。
      printk (DEVICE_NAME " I/O error\n\r");
      printk ("dev %04x, block %d\n\r", CURRENT->dev, CURRENT->bh->b_blocknr);
    }
  wake_up (&CURRENT->waiting);	// 唤醒等待该请求项的进程。
  wake_up (&wait_for_request);	// 唤醒等待请求的进程。
  CURRENT->dev = -1;		// 释放该请求项。
  CURRENT = CURRENT->next;	// 从请求链表中删除该请求项。
}

// 定义初始化请求宏。
#define INIT_REQUEST \
repeat: \
if (!CURRENT) 			/* 如果当前请求结构指针为null 则返回。*/\
return; \
if (MAJOR (CURRENT->dev) != MAJOR_NR) 	/* 如果当前设备的主设备号不对则死机。*/ \
    panic (DEVICE_NAME ": request list destroyed"); \
if (CURRENT->bh) \
{ \
    if (!CURRENT->bh->b_lock) 	/* 如果在进行请求操作时缓冲区没锁定则死机。*/\
	panic (DEVICE_NAME ": block not locked"); \
}

#endif

#endif
