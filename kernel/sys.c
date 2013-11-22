/* passed
* linux/kernel/sys.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

#include <errno.h>		// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。

#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/tty.h>		// tty 头文件，定义了有关tty_io，串行通信方面的参数、常数。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。
#include <sys/times.h>		// 定义了进程中运行时间的结构tms 以及times()函数原型。
#include <sys/utsname.h>	// 系统名称结构头文件。

// 返回日期和时间。
int sys_ftime ()
{
	return -ENOSYS;
}

//
int sys_break ()
{
	return -ENOSYS;
}

// 用于当前进程对子进程进行调试(degugging)。
int sys_ptrace ()
{
	return -ENOSYS;
}

// 改变并打印终端行设置。
int sys_stty ()
{
	return -ENOSYS;
}

// 取终端行设置信息。
int sys_gtty ()
{
	return -ENOSYS;
}

// 修改文件名。
int sys_rename ()
{
	return -ENOSYS;
}

//
int sys_prof ()
{
	return -ENOSYS;
}

// 设置当前任务的实际以及/或者有效组ID（gid）。如果任务没有超级用户特权，
// 那么只能互换其实际组ID 和有效组ID。如果任务具有超级用户特权，就能任意设置有效的和实际
// 的组ID。保留的gid（saved gid）被设置成与有效gid 同值。
int sys_setregid (int rgid, int egid)
{
	if (rgid > 0)
	{
		if ((current->gid == rgid) || suser ())
			current->gid = rgid;
		else
			return (-EPERM);
	}
	if (egid > 0)
	{
		if ((current->gid == egid) || (current->egid == egid) || 
								(current->sgid == egid) || suser ())
			current->egid = egid;
		else
			return (-EPERM);
    }
	return 0;
}

// 设置进程组号(gid)。如果任务没有超级用户特权，它可以使用setgid()将其有效gid
// （effective gid）设置为成其保留gid(saved gid)或其实际gid(real gid)。如果任务有
// 超级用户特权，则实际gid、有效gid 和保留gid 都被设置成参数指定的gid。
int sys_setgid (int gid)
{
	return (sys_setregid (gid, gid));
}

// 打开或关闭进程计帐功能。
int sys_acct ()
{
	return -ENOSYS;
}

// 映射任意物理内存到进程的虚拟地址空间。
int sys_phys ()
{
	return -ENOSYS;
}

int sys_lock ()
{
	return -ENOSYS;
}

int sys_mpx ()
{
	return -ENOSYS;
}

int sys_ulimit ()
{
	return -ENOSYS;
}

// 返回从1970 年1 月1 日00:00:00 GMT 开始计时的时间值（秒）。如果tloc 不为null，则时间值
// 也存储在那里。
int sys_time (long *tloc)
{
	int i;

	i = CURRENT_TIME;
	if (tloc)
	{
		verify_area (tloc, 4);	// 验证内存容量是否够（这里是4 字节）。
		put_fs_long (i, (unsigned long *) tloc);	// 也放入用户数据段tloc 处。
	}
	return i;
}

/*
* Unprivileged users may change the real user id to the effective uid
* or vice versa.
*/
/*
* 无特权的用户可以见实际用户标识符(real uid)改成有效用户标识符(effective uid)，反之也然。
*/
// 设置任务的实际以及/或者有效用户ID（uid）。如果任务没有超级用户特权，那么只能互换其
// 实际用户ID 和有效用户ID。如果任务具有超级用户特权，就能任意设置有效的和实际的用户ID。
// 保留的uid（saved uid）被设置成与有效uid 同值。
int sys_setreuid (int ruid, int euid)
{
	int old_ruid = current->uid;

	if (ruid > 0)
	{
		if ((current->euid == ruid) || (old_ruid == ruid) || suser ())
			current->uid = ruid;
		else
			return (-EPERM);
	}
	if (euid > 0)
	{
		if ((old_ruid == euid) || (current->euid == euid) || suser ())
			current->euid = euid;
		else
		{
			current->uid = old_ruid;
			return (-EPERM);
		}
	}
	return 0;
}

// 设置任务用户号(uid)。如果任务没有超级用户特权，它可以使用setuid()将其有效uid
// （effective uid）设置成其保留uid(saved uid)或其实际uid(real uid)。如果任务有
// 超级用户特权，则实际uid、有效uid 和保留uid 都被设置成参数指定的uid。
int sys_setuid (int uid)
{
	return (sys_setreuid (uid, uid));
}

// 设置系统时间和日期。参数tptr 是从1970 年1 月1 日00:00:00 GMT 开始计时的时间值（秒）。
// 调用进程必须具有超级用户权限。
int sys_stime (long *tptr)
{
	if (!suser ())		// 如果不是超级用户则出错返回（许可）。
		return -EPERM;
	startup_time = get_fs_long ((unsigned long *) tptr) - jiffies / HZ;
	return 0;
}

// 获取当前任务时间。tms 结构中包括用户时间、系统时间、子进程用户时间、子进程系统时间。
int sys_times (struct tms *tbuf)
{
	if (tbuf)
	{
		verify_area (tbuf, sizeof *tbuf);
		put_fs_long (current->utime, (unsigned long *) &tbuf->tms_utime);
		put_fs_long (current->stime, (unsigned long *) &tbuf->tms_stime);
		put_fs_long (current->cutime, (unsigned long *) &tbuf->tms_cutime);
		put_fs_long (current->cstime, (unsigned long *) &tbuf->tms_cstime);
	}
	return jiffies;
}

// 当参数end_data_seg 数值合理，并且系统确实有足够的内存，而且进程没有超越其最大数据段大小
// 时，该函数设置数据段末尾为end_data_seg 指定的值。该值必须大于代码结尾并且要小于堆栈
// 结尾16KB。返回值是数据段的新结尾值（如果返回值与要求值不同，则表明有错发生）。
// 该函数并不被用户直接调用，而由libc 库函数进行包装，并且返回值也不一样。
int sys_brk (unsigned long end_data_seg)
{
	if (end_data_seg >= current->end_code &&	// 如果参数>代码结尾，并且
	end_data_seg < current->start_stack - 16384)	// 小于堆栈-16KB，
		current->brk = end_data_seg;	// 则设置新数据段结尾值。
	return current->brk;		// 返回进程当前的数据段结尾值。
}

/*
* This needs some heave checking ...
* I just haven't get the stomach for it. I also don't fully
* understand sessions/pgrp etc. Let somebody who does explain it.
*/
/*
* 下面代码需要某些严格的检查…
* 我只是没有胃口来做这些。我也不完全明白sessions/pgrp 等。还是让了解它们的人来做吧。
*/
// 将参数pid 指定进程的进程组ID 设置成pgid。如果参数pid=0，则使用当前进程号。如果
// pgid 为0，则使用参数pid 指定的进程的组ID 作为pgid。如果该函数用于将进程从一个
// 进程组移到另一个进程组，则这两个进程组必须属于同一个会话(session)。在这种情况下，
// 参数pgid 指定了要加入的现有进程组ID，此时该组的会话ID 必须与将要加入进程的相同(193 行)。
int sys_setpgid (int pid, int pgid)
{
	int i;

	if (!pid)			// 如果参数pid=0，则使用当前进程号。
		pid = current->pid;
	if (!pgid)			// 如果pgid 为0，则使用当前进程pid 作为pgid。
		pgid = current->pid;	// [??这里与POSIX 的描述有出入]
	for (i = 0; i < NR_TASKS; i++)	// 扫描任务数组，查找指定进程号的任务。
	if (task[i] && task[i]->pid == pid)
	{
		if (task[i]->leader)	// 如果该任务已经是首领，则出错返回。
			return -EPERM;
		if (task[i]->session != current->session)	// 如果该任务的会话ID
			return -EPERM;	// 与当前进程的不同，则出错返回。
		task[i]->pgrp = pgid;	// 设置该任务的pgrp。
		return 0;
	}
	return -ESRCH;
}

// 返回当前进程的组号。与getpgid(0)等同。
int sys_getpgrp (void)
{
	return current->pgrp;
}

// 创建一个会话(session)（即设置其leader=1），并且设置其会话=其组号=其进程号。
int sys_setsid (void)
{
	if (current->leader && !suser ())	// 如果当前进程已是会话首领并且不是超级用户
		return -EPERM;		// 则出错返回。
	current->leader = 1;		// 设置当前进程为新会话首领。
	current->session = current->pgrp = current->pid;	// 设置本进程session = pid。
	current->tty = -1;		// 表示当前进程没有控制终端。
	return current->pgrp;		// 返回会话ID。
}

// 获取系统信息。其中utsname 结构包含5 个字段，分别是：本版本操作系统的名称、网络节点名称、
// 当前发行级别、版本级别和硬件类型名称。
int sys_uname (struct utsname *name)
{
	static struct utsname thisname = {	// 这里给出了结构中的信息，这种编码肯定会改变。
		"linux .0", "nodename", "release ", "version ", "machine "
	};
	int i;

	if (!name)
		return -ERROR;		// 如果存放信息的缓冲区指针为空则出错返回。
	verify_area (name, sizeof *name);	// 验证缓冲区大小是否超限（超出已分配的内存等）。
	for (i = 0; i < sizeof *name; i++)	// 将utsname 中的信息逐字节复制到用户缓冲区中。
		put_fs_byte (((char *) &thisname)[i], i + (char *) name);
	return 0;
}

// 设置当前进程创建文件属性屏蔽码为mask & 0777。并返回原屏蔽码。
int sys_umask (int mask)
{
	int old = current->umask;

	current->umask = mask & 0777;
	return (old);
}
