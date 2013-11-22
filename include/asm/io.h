//// 硬件端口字节输出函数。
// 参数：value - 欲输出字节；port - 端口。
#define outb(value,port) _outb((unsigned char)(value),(unsigned short)(port))
void _inline _outb(unsigned char value,unsigned short port) //passed
{
	_asm mov dx,port ;
	_asm mov al,value ;
	_asm out dx,al ;
}
//__asm__ ( "outb %%al,%%dx":: "a" (value), "d" (port))


//// 硬件端口字节输入函数。
// 参数：port - 端口。返回读取的字节。
#define inb(port) _inb((unsigned short)(port))
unsigned char _inline _inb(unsigned short port) 
{ 
//	unsigned char _v; 
	_asm mov dx,port ;
	_asm in al,dx ;
//	_asm mov _v,al ;
//	return _v;
}
/*
unsigned char _v; \
__asm__ volatile ( "inb %%dx,%%al": "=a" (_v): "d" (port)); \
_v; \
})*/

//// 带延迟的硬件端口字节输出函数。
// 参数：value - 欲输出字节；port - 端口。
/*
#define outb_p(value,port) \
_asm {\
	_asm mov al,value \
	_asm mov dx,port \
	_asm out dx,al \
	_asm jmp l1 \
_asm l1: jmp l2 \
_asm l2: \
}
__asm__ ( "outb %%al,%%dx\n" \
"\tjmp 1f\n" \
"1:\tjmp 1f\n" \
"1:":: "a" (value), "d" (port))
*/
#define outb_p(value,port) _outb_p((unsigned char)(value),(unsigned short)(port))
_inline void _outb_p(unsigned char value, unsigned short port)
{
	_asm mov al,value 
	_asm mov dx,port 
	_asm out dx,al 
	_asm jmp l1 
	_asm l1: jmp l2 
	_asm l2: 
}

//// 带延迟的硬件端口字节输入函数。
// 参数：port - 端口。返回读取的字节。
/*
#define inb_p(port) { \
volatile unsigned char _v; \
_asm { \
	_asm mov dx,port \
	_asm in al,dx \
	_asm mov _v,al \
	_asm jmp l1 \
_asm l1: jmp l2 \
_asm l2: \
} \
_v; \
}
unsigned char _v; \
__asm__ volatile ( "inb %%dx,%%al\n" \
"\tjmp 1f\n" \
"1:\tjmp 1f\n" \
"1:": "=a" (_v): "d" (port)); \
_v; \
})*/
#define inb_p(port) _inb_p((unsigned short)(port))
_inline unsigned char _inb_p(unsigned short port)
{
//	volatile unsigned char _v; 
	_asm { 
		mov dx,port 
		in al,dx 
//		mov _v,al 
		jmp l1 
	l1: jmp l2 
	l2: 
	}
//	return _v;
}
