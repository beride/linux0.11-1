// 不知道为什么在这里定义memcpy，string.h文件里已经有一个了。
/*
* 注意!!!memcpy(dest,src,n)假设段寄存器ds=es=通常数据段。在内核中使用的
* 所有函数都基于该假设（ds=es=内核空间，fs=局部数据空间，gs=null）,具有良好
* 行为的应用程序也是这样（ds=es=用户数据空间）。如果任何用户程序随意改动了
* es 寄存器而出错，则并不是由于系统程序错误造成的。
*/
//// 内存块复制。从源地址src 处开始复制n 个字节到目的地址dest 处。
// 参数：dest - 复制的目的地址，src - 复制的源地址，n - 复制字节数。
// %0 - edi(目的地址dest)，%1 - esi(源地址src)，%2 - ecx(字节数n)，
/*extern _inline void* memcpy(void * dest, const void * src, int n)
{
	_asm{
		pushf
		mov esi,src
		mov edi,dest
		mov ecx,n	// 共复制ecx(n)字节。
		cld
		rep movsb // 从ds:[esi]复制到es:[edi]，并且esi++，edi++。
		popf
	}
	return dest;
}
#define memcpy(dest,src,n) ({ \
void * _res = dest; \
__asm__ ( "cld;rep;movsb" \
:: "D" ((long)(_res)), "S" ((long)(src)), "c" ((long) (n)) \
: "di", "si", "cx"); \
_res; \
})*/

