/*
 *  linux/lib/open.c
 *
 *  open()系统调用用于将一个文件名转换成一个文件描述符。当调用成功时，返回的文件描述符
 * 将是进程没有打开的最小数值的描述符。该调用创建一个新的打开文件，并不与任何其它进程
 * 共享。在执行exec函数时，该新的文件描述符将始终保持着打开状态。文件的读写指针被设置
 * 在文件开始位置。参数flag 是0_RDONLY、O_WRONLY、O_RDWR 之一，分别代表文件只读打开、只
 * 写打开和读写打开方式，可以与其它一些标志一起使用。(参见fs/open.c，138 行)
 */
#include <set_seg.h>

#define __LIBRARY__
#include <unistd.h>// Linux 标准头文件。定义了各种符号常数和类型，并申明了各种函数。
								// 如定义了__LIBRARY__，则还包括系统调用号和内嵌汇编_syscall0()等。
#include <stdarg.h>// 标准参数头文件。以宏的形式定义变量参数列表。主要说明了-个
								// 类型(va_list)和三个宏(va_start, va_arg 和va_end)，用于
								// vsprintf、vprintf、vfprintf 函数。

//// 打开文件函数。
// 打开并有可能创建一个文件。
// 参数：filename - 文件名；flag - 文件打开标志；...
// 返回：文件描述符，若出错则置出错码，并返回-1。
int open(const char * filename, int flag, ...)
{
	register int res;
	va_list arg;

// 利用va_start()宏函数，取得flag 后面参数的指针，然后调用系统中断int 0x80，功能open 进行
// 文件打开操作。
// %0 - eax(返回的描述符或出错码)；%1 - eax(系统中断调用功能号__NR_open)；
// %2 - ebx(文件名filename)；%3 - ecx(打开文件标志flag)；%4 - edx(后随参数文件属性mode)。
	va_start(arg,flag);
	res = va_arg(arg,int);
	_asm{
		mov eax,__NR_open
		mov ebx,filename
		mov ecx,flag
		mov edx,res
		int 0x80
		mov res,eax
	}
/*	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_open),"b" (filename),"c" (flag),
		"d" (va_arg(arg,int)));*/
// 系统中断调用返回值大于或等于0，表示是一个文件描述符，则直接返回之。
	if (res>=0)
		return res;
// 否则说明返回值小于0，则代表一个出错码。设置该出错码并返回-1。
	errno = -res;
	return -1;
}
