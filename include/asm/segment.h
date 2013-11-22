//// 读取fs 段中指定地址处的字节。
// 参数：addr - 指定的内存地址。
// %0 - (返回的字节_v)；%1 - (内存地址addr)。
// 返回：返回内存fs:[addr]处的字节。
extern _inline unsigned char
get_fs_byte (const char *addr)
{
//  unsigned register char _v;

// __asm__ ("movb %%fs:%1,%0": "=r" (_v):"m" (*addr));
  _asm mov ebx,addr
  _asm mov al,byte ptr fs:[ebx];
//  _asm mov _v,al;
//  return _v;
}

//// 读取fs 段中指定地址处的字。
// 参数：addr - 指定的内存地址。
// %0 - (返回的字_v)；%1 - (内存地址addr)。
// 返回：返回内存fs:[addr]处的字。
extern _inline unsigned short
get_fs_word (const unsigned short *addr)
{
//  unsigned short _v;

//__asm__ ("movw %%fs:%1,%0": "=r" (_v):"m" (*addr));
  _asm mov ebx,addr
  _asm mov ax,word ptr fs:[ebx];
//  _asm mov _v,ax;
//  return _v;
}

//// 读取fs 段中指定地址处的长字(4 字节)。
// 参数：addr - 指定的内存地址。
// %0 - (返回的长字_v)；%1 - (内存地址addr)。
// 返回：返回内存fs:[addr]处的长字。
extern _inline unsigned long
get_fs_long (const unsigned long *addr)
{
//  unsigned long _v;

//__asm__ ("movl %%fs:%1,%0": "=r" (_v):"m" (*addr));
  _asm mov ebx,addr
  _asm mov eax,dword ptr fs:[ebx];
//  _asm mov _v,eax;
//  return _v;
}

//// 将一字节存放在fs 段中指定内存地址处。
// 参数：val - 字节值；addr - 内存地址。
// %0 - 寄存器(字节值val)；%1 - (内存地址addr)。
extern _inline void
put_fs_byte (char val, char *addr)//passed
{
//  __asm__ ("movb %0,%%fs:%1"::"r" (val), "m" (*addr));
	_asm mov ebx,addr
	_asm mov al,val;
	_asm mov byte ptr fs:[ebx],al;
}

//// 将一字存放在fs 段中指定内存地址处。
// 参数：val - 字值；addr - 内存地址。
// %0 - 寄存器(字值val)；%1 - (内存地址addr)。
extern _inline void
put_fs_word (short val, short *addr)
{
//  __asm__ ("movw %0,%%fs:%1"::"r" (val), "m" (*addr));
	_asm mov ebx,addr
	_asm mov ax,val;
	_asm mov word ptr fs:[ebx],ax;
}

//// 将一长字存放在fs 段中指定内存地址处。
// 参数：val - 长字值；addr - 内存地址。
// %0 - 寄存器(长字值val)；%1 - (内存地址addr)。
extern _inline void
put_fs_long (long val, unsigned long *addr)
{
//  __asm__ ("movl %0,%%fs:%1"::"r" (val), "m" (*addr));
	_asm mov ebx,addr
	_asm mov eax,val;
	_asm mov dword ptr fs:[ebx],eax;
}

/*
* 比我更懂GNU 汇编的人应该仔细检查下面的代码。这些代码能使用，但我不知道是否
* 含有一些小错误。
* --- TYT，1991 年11 月24 日
* [ 这些代码没有错误，Linus ]
*/

//// 取fs 段寄存器值(选择符)。
// 返回：fs 段寄存器值。
extern _inline unsigned short
get_fs ()
{
//  unsigned short _v;
//__asm__ ("mov %%fs,%%ax": "=a" (_v):);
  _asm mov ax,fs;
//  _asm mov _v,ax;
//  return _v;
}

//// 取ds 段寄存器值。
// 返回：ds 段寄存器值。
extern _inline unsigned short
get_ds ()
{
//  unsigned short _v;
//__asm__ ("mov %%ds,%%ax": "=a" (_v):);
  _asm mov ax,fs;
//  _asm mov _v,ax;
//  return _v;
}

//// 设置fs 段寄存器。
// 参数：val - 段值（选择符）。
extern _inline void
set_fs (unsigned long val)
{
//  __asm__ ("mov %0,%%fs"::"a" ((unsigned short) val));
	_asm mov eax,val;
	_asm mov fs,ax;
}
