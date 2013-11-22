/* passed
 *  linux/fs/pipe.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

#include <signal.h>// 信号头文件。定义信号符号常量，信号结构以及信号操作函数原型。

#include <linux/sched.h>// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
									// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/mm.h>	/* 使用其中的get_free_page */
									// 内存管理头文件。含有页面大小定义和一些页面释放函数原型。
#include <asm/segment.h>// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

//// 管道读操作函数。
// 参数inode 是管道对应的i 节点，buf 是数据缓冲区指针，count 是读取的字节数。
int read_pipe(struct m_inode * inode, char * buf, int count)
{
	int chars, size, read = 0;

// 若欲读取的字节计数值count 大于0，则循环执行以下操作。
	while (count>0) {
// 若当前管道中没有数据(size=0)，则唤醒等待该节点的进程，如果已没有写管道者，则返回已读
// 字节数，退出。否则在该i 节点上睡眠，等待信息。
		while (!(size=PIPE_SIZE(*inode))) {
			wake_up(&inode->i_wait);
			if (inode->i_count != 2) /* are there any writers? */
				return read;
			sleep_on(&inode->i_wait);
		}
// 取管道尾到缓冲区末端的字节数chars。如果其大于还需要读取的字节数count，则令其等于count。
// 如果chars 大于当前管道中含有数据的长度size，则令其等于size。
		chars = PAGE_SIZE-PIPE_TAIL(*inode);
		if (chars > count)
			chars = count;
		if (chars > size)
			chars = size;
// 读字节计数减去此次可读的字节数chars，并累加已读字节数。
		count -= chars;
		read += chars;
// 令size 指向管道尾部，调整当前管道尾指针（前移chars 字节）。
		size = PIPE_TAIL(*inode);
		PIPE_TAIL(*inode) += chars;
		PIPE_TAIL(*inode) &= (PAGE_SIZE-1);
// 将管道中的数据复制到用户缓冲区中。对于管道i 节点，其i_size 字段中是管道缓冲块指针。
		while (chars-->0)
			put_fs_byte(((char *)inode->i_size)[size++],buf++);
	}
// 唤醒等待该管道i 节点的进程，并返回读取的字节数。
	wake_up(&inode->i_wait);
	return read;
}
	
//// 管道写操作函数。
// 参数inode 是管道对应的i 节点，buf 是数据缓冲区指针，count 是将写入管道的字节数。
int write_pipe(struct m_inode * inode, char * buf, int count)
{
	int chars, size, written = 0;

// 若将写入的字节计数值count 还大于0，则循环执行以下操作。
	while (count>0) {
// 若当前管道中没有已经满了(size=0)，则唤醒等待该节点的进程，如果已没有读管道者，则向进程
// 发送SIGPIPE 信号，并返回已写入的字节数并退出。若写入0 字节，则返回-1。否则在该i 节点上
// 睡眠，等待管道腾出空间。
		while (!(size=(PAGE_SIZE-1)-PIPE_SIZE(*inode))) {
			wake_up(&inode->i_wait);
			if (inode->i_count != 2) { /* no readers */
				current->signal |= (1<<(SIGPIPE-1));
				return written?written:-1;
			}
			sleep_on(&inode->i_wait);
		}
// 取管道头部到缓冲区末端空间字节数chars。如果其大于还需要写入的字节数count，则令其等于
// count。如果chars 大于当前管道中空闲空间长度size，则令其等于size。
		chars = PAGE_SIZE-PIPE_HEAD(*inode);
		if (chars > count)
			chars = count;
		if (chars > size)
			chars = size;
// 写入字节计数减去此次可写入的字节数chars，并累加已写字节数到written。
		count -= chars;
		written += chars;
// 令size 指向管道数据头部，调整当前管道数据头部指针（前移chars 字节）。
		size = PIPE_HEAD(*inode);
		PIPE_HEAD(*inode) += chars;
		PIPE_HEAD(*inode) &= (PAGE_SIZE-1);
// 从用户缓冲区复制chars 个字节到管道中。对于管道i 节点，其i_size 字段中是管道缓冲块指针。
		while (chars-->0)
			((char *)inode->i_size)[size++]=get_fs_byte(buf++);
	}
// 唤醒等待该i 节点的进程，返回已写入的字节数，退出。
	wake_up(&inode->i_wait);
	return written;
}

//// 创建管道系统调用函数。
// 在fildes 所指的数组中创建一对文件句柄(描述符)。这对文件句柄指向一管道i 节点。fildes[0]
// 用于读管道中数据，fildes[1]用于向管道中写入数据。
// 成功时返回0，出错时返回-1。
int sys_pipe(unsigned long * fildes)
{
	struct m_inode * inode;
	struct file * f[2];
	int fd[2];
	int i,j;

// 从系统文件表中取两个空闲项（引用计数字段为0 的项），并分别设置引用计数为1。
	j=0;
	for(i=0;j<2 && i<NR_FILE;i++)
		if (!file_table[i].f_count)
			(f[j++]=i+file_table)->f_count++;
// 如果只有一个空闲项，则释放该项(引用计数复位)。
	if (j==1)
		f[0]->f_count=0;
// 如果没有找到两个空闲项，则返回-1。
	if (j<2)
		return -1;
// 针对上面取得的两个文件结构项，分别分配一文件句柄，并使进程的文件结构指针分别指向这两个
// 文件结构。
	j=0;
	for(i=0;j<2 && i<NR_OPEN;i++)
		if (!current->filp[i]) {
			current->filp[ fd[j]=i ] = f[j];
			j++;
		}
// 如果只有一个空闲文件句柄，则释放该句柄。
	if (j==1)
		current->filp[fd[0]]=NULL;
// 如果没有找到两个空闲句柄，则释放上面获取的两个文件结构项（复位引用计数值），并返回-1。
	if (j<2) {
		f[0]->f_count=f[1]->f_count=0;
		return -1;
	}
// 申请管道i 节点，并为管道分配缓冲区（1 页内存）。如果不成功，则相应释放两个文件句柄和文
// 件结构项，并返回-1。
	if (!(inode=get_pipe_inode())) {
		current->filp[fd[0]] =
			current->filp[fd[1]] = NULL;
		f[0]->f_count = f[1]->f_count = 0;
		return -1;
	}
// 初始化两个文件结构，都指向同一个i 节点，读写指针都置零。第1 个文件结构的文件模式置为读，
// 第2 个文件结构的文件模式置为写。
	f[0]->f_inode = f[1]->f_inode = inode;
	f[0]->f_pos = f[1]->f_pos = 0;
	f[0]->f_mode = 1;		/* read */
	f[1]->f_mode = 2;		/* write */
// 将文件句柄数组复制到对应的用户数组中，并返回0，退出。
	put_fs_long(fd[0],0+fildes);
	put_fs_long(fd[1],1+fildes);
	return 0;
}
