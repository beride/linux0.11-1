/* passed
* linux/kernel/exit.c
*
* (C) 1991 Linus Torvalds
*/
#include <set_seg.h>

#include <errno.h>		// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)
#include <signal.h>		// 信号头文件。定义信号符号常量，信号结构以及信号操作函数原型。
#include <sys/wait.h>		// 等待调用头文件。定义系统调用wait()和waitpid()及相关常数符号。

#include <linux/sched.h>	// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>	// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/tty.h>		// tty 头文件，定义了有关tty_io，串行通信方面的参数、常数。
#include <asm/segment.h>	// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

int sys_pause (void);
int sys_close (int fd);

//// 释放指定进程(任务)。
void release (struct task_struct *p)
{
  int i;

  if (!p)
    return;
  for (i = 1; i < NR_TASKS; i++)	// 扫描任务数组，寻找指定任务。
    if (task[i] == p)
	{
		task[i] = NULL;		// 置空该任务项并释放相关内存页。
		free_page ((long) p);
		schedule ();		// 重新调度。
		return;
	}
  panic ("trying to release non-existent task");	// 指定任务若不存在则死机。
}

//// 向指定任务(*p)发送信号(sig)，权限为priv。
static _inline int
send_sig (long sig, struct task_struct *p, int priv)
{
// 若信号不正确或任务指针为空则出错退出。
  if (!p || sig < 1 || sig > 32)
    return -EINVAL;
// 若有权或进程有效用户标识符(euid)就是指定进程的euid 或者是超级用户，则在进程位图中添加
// 该信号，否则出错退出。其中suser()定义为(current->euid==0)，用于判断是否超级用户。
  if (priv || (current->euid == p->euid) || suser ())
    p->signal |= (1 << (sig - 1));
  else
    return -EPERM;
  return 0;
}

//// 终止会话(session)。
static void kill_session (void)
{
  struct task_struct **p = NR_TASKS + task;	// 指针*p 首先指向任务数组最末端。

// 对于所有的任务（除任务0 以外），如果其会话等于当前进程的会话就向它发送挂断进程信号。
  while (--p > &FIRST_TASK)
  {
    if (*p && (*p)->session == current->session)
	  (*p)->signal |= 1 << (SIGHUP - 1);	// 发送挂断进程信号。
  }
}

/*
* 为了向进程组等发送信号，XXX 需要检查许可。kill()的许可机制非常巧妙!
*/
//// kill()系统调用可用于向任何进程或进程组发送任何信号。
// 如果pid 值>0，则信号被发送给pid。
// 如果pid=0，那么信号就会被发送给当前进程的进程组中的所有进程。
// 如果pid=-1，则信号sig 就会发送给除第一个进程外的所有进程。
// 如果pid < -1，则信号sig 将发送给进程组-pid 的所有进程。
// 如果信号sig 为0，则不发送信号，但仍会进行错误检查。如果成功则返回0。
int sys_kill (int pid, int sig)
{
  struct task_struct **p = NR_TASKS + task;
  int err, retval = 0;

  if (!pid)
    while (--p > &FIRST_TASK)
	{
		if (*p && (*p)->pgrp == current->pid)
		  if (err = send_sig (sig, *p, 1))
			retval = err;
	}
  else if (pid > 0)
    while (--p > &FIRST_TASK)
    {
	if (*p && (*p)->pid == pid)
	  if (err = send_sig (sig, *p, 0))
	    retval = err;
    }
  else if (pid == -1)
    while (--p > &FIRST_TASK)
      if (err = send_sig (sig, *p, 0))
		retval = err;
      else
		while (--p > &FIRST_TASK)
		  if (*p && (*p)->pgrp == -pid)
			if (err = send_sig (sig, *p, 0))
			  retval = err;
  return retval;
}

//// 通知父进程 -- 向进程pid 发送信号SIGCHLD：子进程将停止或终止。
// 如果没有找到父进程，则自己释放。
static void tell_father (int pid)
{
  int i;

  if (pid)
    for (i = 0; i < NR_TASKS; i++)
    {
		if (!task[i])
		  continue;
		if (task[i]->pid != pid)
		  continue;
		task[i]->signal |= (1 << (SIGCHLD - 1));
		return;
    }
/* if we don't find any fathers, we just release ourselves */
/* This is not really OK. Must change it to make father 1 */
  printk ("BAD BAD - no father found\n\r");
  release (current);		// 如果没有找到父进程，则自己释放。
}

//// 程序退出处理程序。在系统调用的中断处理程序中被调用。
int do_exit (long code)		// code 是错误码。
{
  int i;

// 释放当前进程代码段和数据段所占的内存页(free_page_tables()在mm/memory.c,105 行)。
  free_page_tables (get_base (current->ldt[1]), get_limit (0x0f));
  free_page_tables (get_base (current->ldt[2]), get_limit (0x17));
// 如果当前进程有子进程，就将子进程的father 置为1(其父进程改为进程1)。如果该子进程已经
// 处于僵死(ZOMBIE)状态，则向进程1 发送子进程终止信号SIGCHLD。
  for (i = 0; i < NR_TASKS; i++)
    if (task[i] && task[i]->father == current->pid)
      {
	task[i]->father = 1;
	if (task[i]->state == TASK_ZOMBIE)
/* assumption task[1] is always init */
	  (void) send_sig (SIGCHLD, task[1], 1);
      }
// 关闭当前进程打开着的所有文件。
  for (i = 0; i < NR_OPEN; i++)
    if (current->filp[i])
      sys_close (i);
// 对当前进程工作目录pwd、根目录root 以及运行程序的i 节点进行同步操作，并分别置空。
  iput (current->pwd);
  current->pwd = NULL;
  iput (current->root);
  current->root = NULL;
  iput (current->executable);
  current->executable = NULL;
// 如果当前进程是领头(leader)进程并且其有控制的终端，则释放该终端。
  if (current->leader && current->tty >= 0)
    tty_table[current->tty].pgrp = 0;
// 如果当前进程上次使用过协处理器，则将last_task_used_math 置空。
  if (last_task_used_math == current)
    last_task_used_math = NULL;
// 如果当前进程是leader 进程，则终止所有相关进程。
  if (current->leader)
    kill_session ();
// 把当前进程置为僵死状态，并设置退出码。
  current->state = TASK_ZOMBIE;
  current->exit_code = code;
// 通知父进程，也即向父进程发送信号SIGCHLD -- 子进程将停止或终止。
  tell_father (current->father);
  schedule ();			// 重新调度进程的运行。
  return (-1);			/* just to suppress warnings */
}

//// 系统调用exit()。终止进程。
int sys_exit (int error_code)
{
  return do_exit ((error_code & 0xff) << 8);
}

//// 系统调用waitpid()。挂起当前进程，直到pid 指定的子进程退出（终止）或者收到要求终止
// 该进程的信号，或者是需要调用一个信号句柄（信号处理程序）。如果pid 所指的子进程早已
// 退出（已成所谓的僵死进程），则本调用将立刻返回。子进程使用的所有资源将释放。
// 如果pid > 0, 表示等待进程号等于pid 的子进程。
// 如果pid = 0, 表示等待进程组号等于当前进程的任何子进程。
// 如果pid < -1, 表示等待进程组号等于pid 绝对值的任何子进程。
// [ 如果pid = -1, 表示等待任何子进程。]
// 若options = WUNTRACED，表示如果子进程是停止的，也马上返回。
// 若options = WNOHANG，表示如果没有子进程退出或终止就马上返回。
// 如果stat_addr 不为空，则就将状态信息保存到那里。
int sys_waitpid (pid_t pid, unsigned long *stat_addr, int options)
{
  int flag, code;
  struct task_struct **p;

  verify_area (stat_addr, 4);
repeat:
  flag = 0;
  for (p = &LAST_TASK; p > &FIRST_TASK; --p)
  {				// 从任务数组末端开始扫描所有任务。
      if (!*p || *p == current)	// 跳过空项和本进程项。
		continue;
      if ((*p)->father != current->pid)	// 如果不是当前进程的子进程则跳过。
		continue;
      if (pid > 0)
		{			// 如果指定的pid>0，但扫描的进程pid
		  if ((*p)->pid != pid)	// 与之不等，则跳过。
			continue;
		}
      else if (!pid)
		{			// 如果指定的pid=0，但扫描的进程组号
		  if ((*p)->pgrp != current->pgrp)	// 与当前进程的组号不等，则跳过。
			continue;
		}
      else if (pid != -1)
		{			// 如果指定的pid<-1，但扫描的进程组号
		  if ((*p)->pgrp != -pid)	// 与其绝对值不等，则跳过。
			continue;
		}
    switch ((*p)->state)
	{
	case TASK_STOPPED:
	  if (!(options & WUNTRACED))
	    continue;
	  put_fs_long (0x7f, stat_addr);	// 置状态信息为0x7f。
	  return (*p)->pid;	// 退出，返回子进程的进程号。
	case TASK_ZOMBIE:
	  current->cutime += (*p)->utime;	// 更新当前进程的子进程用户
	  current->cstime += (*p)->stime;	// 态和核心态运行时间。
	  flag = (*p)->pid;
	  code = (*p)->exit_code;	// 取子进程的退出码。
	  release (*p);		// 释放该子进程。
	  put_fs_long (code, stat_addr);	// 置状态信息为退出码值。
	  return flag;		// 退出，返回子进程的pid.
	default:
	  flag = 1;		// 如果子进程不在停止或僵死状态，则flag=1。
	  continue;
	}
  }
  if (flag)
  {				// 如果子进程没有处于退出或僵死状态，
	  if (options & WNOHANG)	// 并且options = WNOHANG，则立刻返回。
		return 0;
	  current->state = TASK_INTERRUPTIBLE;	// 置当前进程为可中断等待状态。
	  schedule ();		// 重新调度。
	  if (!(current->signal &= ~(1 << (SIGCHLD - 1))))	// 又开始执行本进程时，
		goto repeat;		// 如果进程没有收到除SIGCHLD 的信号，则还是重复处理。
	  else
		return -EINTR;		// 退出，返回出错码。
  }
  return -ECHILD;
}
