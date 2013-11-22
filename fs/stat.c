/* passed
* linux/fs/stat.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

#include <errno.h>		// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。
#include <sys/stat.h>		// 文件状态头文件。含有文件或文件系统状态结构stat{}和常量。

#include <linux/fs.h>		// 文件系统头文件。定义文件表结构（file,buffer_head,m_inode 等）。
#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

//// 复制文件状态信息。
// 参数inode 是文件对应的i 节点，statbuf 是stat 文件状态结构指针，用于存放取得的状态信息。
static void
cp_stat (struct m_inode *inode, struct stat *statbuf)
{
	struct stat tmp;
	int i;

// 首先验证(或分配)存放数据的内存空间。
	verify_area (statbuf, sizeof (*statbuf));
// 然后临时复制相应节点上的信息。
	tmp.st_dev = inode->i_dev;	// 文件所在的设备号。
	tmp.st_ino = inode->i_num;	// 文件i 节点号。
	tmp.st_mode = inode->i_mode;	// 文件属性。
	tmp.st_nlink = inode->i_nlinks;	// 文件的连接数。
	tmp.st_uid = inode->i_uid;	// 文件的用户id。
	tmp.st_gid = inode->i_gid;	// 文件的组id。
	tmp.st_rdev = inode->i_zone[0];	// 设备号(如果文件是特殊的字符文件或块文件)。
	tmp.st_size = inode->i_size;	// 文件大小（字节数）（如果文件是常规文件）。
	tmp.st_atime = inode->i_atime;	// 最后访问时间。
	tmp.st_mtime = inode->i_mtime;	// 最后修改时间。
	tmp.st_ctime = inode->i_ctime;	// 最后节点修改时间。
// 最后将这些状态信息复制到用户缓冲区中。
	for (i = 0; i < sizeof (tmp); i++)
		put_fs_byte (((char *) &tmp)[i], &((char *) statbuf)[i]);
}

//// 文件状态系统调用函数 - 根据文件名获取文件状态信息。
// 参数filename 是指定的文件名，statbuf 是存放状态信息的缓冲区指针。
// 返回0，若出错则返回出错码。
int
sys_stat (char *filename, struct stat *statbuf)
{
	struct m_inode *inode;

// 首先根据文件名找出对应的i 节点，若出错则返回错误码。
	if (!(inode = namei (filename)))
		return -ENOENT;
// 将i 节点上的文件状态信息复制到用户缓冲区中，并释放该i 节点。
	cp_stat (inode, statbuf);
	iput (inode);
	return 0;
}

//// 文件状态系统调用 - 根据文件句柄获取文件状态信息。
// 参数fd 是指定文件的句柄(描述符)，statbuf 是存放状态信息的缓冲区指针。
// 返回0，若出错则返回出错码。
int
sys_fstat (unsigned int fd, struct stat *statbuf)
{
	struct file *f;
	struct m_inode *inode;

// 如果文件句柄值大于一个程序最多打开文件数NR_OPEN，或者该句柄的文件结构指针为空，或者
// 对应文件结构的i 节点字段为空，则出错，返回出错码并退出。
	if (fd >= NR_OPEN || !(f = current->filp[fd]) || !(inode = f->f_inode))
		return -EBADF;
// 将i 节点上的文件状态信息复制到用户缓冲区中。
	cp_stat (inode, statbuf);
	return 0;
}
