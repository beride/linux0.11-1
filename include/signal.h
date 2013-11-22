#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/types.h>		// 类型头文件。定义了基本的系统数据类型。

typedef int sig_atomic_t;	// 定义信号原子操作类型。
typedef unsigned int sigset_t;	/* 32 bits */// 定义信号集类型。

#define _NSIG 32		// 定义信号种类 -- 32 种。
#define NSIG _NSIG		// NSIG = _NSIG

// 以下这些是Linux 0.11 内核中定义的信号。
#define SIGHUP 1		// Hang Up -- 挂断控制终端或进程。
#define SIGINT 2		// Interrupt -- 来自键盘的中断。
#define SIGQUIT 3		// Quit -- 来自键盘的退出。
#define SIGILL 4		// Illeagle -- 非法指令。
#define SIGTRAP 5		// Trap -- 跟踪断点。
#define SIGABRT 6		// Abort -- 异常结束。
#define SIGIOT 6		// IO Trap -- 同上。
#define SIGUNUSED 7		// Unused -- 没有使用。
#define SIGFPE 8		// FPE -- 协处理器出错。
#define SIGKILL 9		// Kill -- 强迫进程终止。
#define SIGUSR1 10		// User1 -- 用户信号1，进程可使用。
#define SIGSEGV 11		// Segment Violation -- 无效内存引用。
#define SIGUSR2 12		// User2 -- 用户信号2，进程可使用。
#define SIGPIPE 13		// Pipe -- 管道写出错，无读者。
#define SIGALRM 14		// Alarm -- 实时定时器报警。
#define SIGTERM 15		// Terminate -- 进程终止。
#define SIGSTKFLT 16		// Stack Fault -- 栈出错（协处理器）。
#define SIGCHLD 17		// Child -- 子进程停止或被终止。
#define SIGCONT 18		// Continue -- 恢复进程继续执行。
#define SIGSTOP 19		// Stop -- 停止进程的执行。
#define SIGTSTP 20		// TTY Stop -- tty 发出停止进程，可忽略。
#define SIGTTIN 21		// TTY In -- 后台进程请求输入。
#define SIGTTOU 22		// TTY Out -- 后台进程请求输出。

/* Ok, I haven't implemented sigactions, but trying to keep headers POSIX */
/* OK，我还没有实现sigactions 的编制，但在头文件中仍希望遵守POSIX 标准 */
#define SA_NOCLDSTOP 1		// 当子进程处于停止状态，就不对SIGCHLD 处理。
#define SA_NOMASK 0x40000000	// 不阻止在指定的信号处理程序(信号句柄)中再收到该信号。
#define SA_ONESHOT 0x80000000	// 信号句柄一旦被调用过就恢复到默认处理句柄。

// 以下参数用于sigprocmask()-- 改变阻塞信号集(屏蔽码)。这些参数可以改变该函数的行为。
#define SIG_BLOCK 0		/* for blocking signals */
// 在阻塞信号集中加上给定的信号集。
#define SIG_UNBLOCK 1		/* for unblocking signals */
// 从阻塞信号集中删除指定的信号集。
#define SIG_SETMASK 2		/* for setting the signal mask */
// 设置阻塞信号集（信号屏蔽码）。

#define SIG_DFL ((void (*)(int))0)	/* default signal handling */
// 默认的信号处理程序（信号句柄）。
#define SIG_IGN ((void (*)(int))1)	/* ignore signal */
// 忽略信号的处理程序。

// 下面是sigaction 的数据结构。
// sa_handler 是对应某信号指定要采取的行动。可以是上面的SIG_DFL，或者是SIG_IGN 来忽略
// 该信号，也可以是指向处理该信号函数的一个指针。
// sa_mask 给出了对信号的屏蔽码，在信号程序执行时将阻塞对这些信号的处理。
// sa_flags 指定改变信号处理过程的信号集。它是由37-39 行的位标志定义的。
// sa_restorer 恢复过程指针，是用于保存原返回的过程指针。
// 另外，引起触发信号处理的信号也将被阻塞，除非使用了SA_NOMASK 标志。
struct sigaction
{
  void (*sa_handler) (int);
  sigset_t sa_mask;
  int sa_flags;
  void (*sa_restorer) (void);
};

// 为信号_sig 安装一个新的信号处理程序（信号句柄），与sigaction()类似。
void (*signal (int _sig, void (*_func) (int))) (int);
// 向当前进程发送一个信号。其作用等价于kill(getpid(),sig)。
int raise (int sig);
// 可用于向任何进程组或进程发送任何信号。
int kill (pid_t pid, int sig);
// 向信号集中添加信号。
int sigaddset (sigset_t * mask, int signo);
// 从信号集中去除指定的信号。
int sigdelset (sigset_t * mask, int signo);
// 从信号集中清除指定信号集。
int sigemptyset (sigset_t * mask);
// 向信号集中置入所有信号。
int sigfillset (sigset_t * mask);
// 判断一个信号是否是信号集中的。1 -- 是， 0 -- 不是， -1 -- 出错。
int sigismember (sigset_t * mask, int signo);	/* 1 - is, 0 - not, -1 error */
// 对set 中的信号进行检测，看是否有挂起的信号。
int sigpending (sigset_t * set);
// 改变目前的被阻塞信号集（信号屏蔽码）。
int sigprocmask (int how, sigset_t * set, sigset_t * oldset);
// 用sigmask 临时替换进程的信号屏蔽码,然后暂停该进程直到收到一个信号。
int sigsuspend (sigset_t * sigmask);
// 用于改变进程在收到指定信号时所采取的行动。
int sigaction (int sig, struct sigaction *act, struct sigaction *oldact);

#endif /* _SIGNAL_H */
