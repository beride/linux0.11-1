#ifndef _STRING_H_
#define _STRING_H_
/*
  其实vc编译器本身包含了这些函数定义，我们完全可以把string.h内的函数定义
  都注释掉，然后为每个.c文件编译时加上/Ox编译选项即可。
*/

#ifndef NULL
#define NULL 0
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

extern char *strerror (int errno);
/*
* 这个字符串头文件以内嵌函数的形式定义了所有字符串操作函数。使用gcc 时，同时
* 假定了ds=es=数据空间，这应该是常规的。绝大多数字符串函数都是经手工进行大量
* 优化的，尤其是函数strtok、strstr、str[c]spn。它们应该能正常工作，但却不是那
* 么容易理解。所有的操作基本上都是使用寄存器集来完成的，这使得函数即快有整洁。
* 所有地方都使用了字符串指令，这又使得代码“稍微”难以理解?
*
* (C) 1991 Linus Torvalds
*/

//// 将一个字符串(src)拷贝到另一个字符串(dest)，直到遇到NULL 字符后停止。
// 参数：dest - 目的字符串指针，src - 源字符串指针。
// %0 - esi(src)，%1 - edi(dest)。
extern _inline char *
strcpy (char *dest, const char *src)
{
	_asm{
		pushf
		mov esi,src
		mov edi,dest
		cld		// 清方向位。
	l1:	lodsb	// 加载DS:[esi]处1 字节->al，并更新esi。
		stosb		// 存储字节al->ES:[edi]，并更新edi。
		test al,al	// 刚存储的字节是0？
		jne l1	// 不是则跳转到标号l1 处，否则结束。
		popf
	}
	return dest;			// 返回目的字符串指针。
}
/*extern _inline char *
strcpy (char *dest, const char *src)
{
  __asm__ ("cld\n"		// 清方向位。
	   "1:\tlodsb\n\t"	// 加载DS:[esi]处1 字节??al，并更新esi。
	   "stosb\n\t"		// 存储字节al??ES:[edi]，并更新edi。
	   "testb %%al,%%al\n\t"	// 刚存储的字节是0？
	   "jne 1b"		// 不是则向后跳转到标号1 处，否则结束。
::"S" (src), "D" (dest):"si", "di", "ax");
  return dest;			// 返回目的字符串指针。
}*/

//// 拷贝源字符串count 个字节到目的字符串。
// 如果源串长度小于count 个字节，就附加空字符(NULL)到目的字符串。
// 参数：dest - 目的字符串指针，src - 源字符串指针，count - 拷贝字节数。
// %0 - esi(src)，%1 - edi(dest)，%2 - ecx(count)。
static _inline char *
strncpy (char *dest, const char *src, int count)
{
	_asm{
		pushf
		mov esi,src
		mov edi,dest
		mov ecx,count
		cld		// 清方向位。
	l1:	dec ecx	// 寄存器ecx--（count--）。
		js l2		// 如果count<0 则向前跳转到标号l2，结束。
		lodsb		// 取ds:[esi]处1 字节->al，并且esi++。
		stosb		// 存储该字节->es:[edi]，并且edi++。
		test al,al	// 该字节是0？
		jne l1		// 不是，则向前跳转到标号l1 处继续拷贝。
		rep stosb		// 否则，在目的串中存放剩余个数的空字符。
	l2:	popf
	}
	return dest;			// 返回目的字符串指针。
}
/*extern _inline char *
strncpy (char *dest, const char *src, int count)
{
  __asm__ ("cld\n"		// 清方向位。
	   "1:\tdecl %2\n\t"	// 寄存器ecx--（count--）。
	   "js 2f\n\t"		// 如果count<0 则向前跳转到标号2，结束。
	   "lodsb\n\t"		// 取ds:[esi]处1 字节??al，并且esi++。
	   "stosb\n\t"		// 存储该字节??es:[edi]，并且edi++。
	   "testb %%al,%%al\n\t"	// 该字节是0？
	   "jne 1b\n\t"		// 不是，则向前跳转到标号1 处继续拷贝。
	   "rep\n\t"		// 否则，在目的串中存放剩余个数的空字符。
"stosb\n" "2:"::"S" (src), "D" (dest), "c" (count):"si", "di", "ax",
	   "cx");
  return dest;			// 返回目的字符串指针。
}*/

//// 将源字符串拷贝到目的字符串的末尾处。
// 参数：dest - 目的字符串指针，src - 源字符串指针。
// %0 - esi(src)，%1 - edi(dest)，%2 - eax(0)，%3 - ecx(-1)。
extern _inline char *
strcat (char *dest, const char *src)
{
	_asm {
		pushf
		mov esi,src
		mov edi,dest
		xor al,al
		mov ecx,0xffffffff
		cld		// 清方向位。
		repne scasb		// 比较al 与es:[edi]字节，并更新edi++，
						// 直到找到目的串中是0 的字节，此时edi 已经指向后1 字节。
		dec edi		// 让es:[edi]指向0 值字节。
	l1: lodsb	// 取源字符串字节ds:[esi]->al，并esi++。
		stosb		// 将该字节存到es:[edi]，并edi++。
		test al,al	// 该字节是0？
		jne l1	// 不是，则向后跳转到标号1 处继续拷贝，否则结束。
		popf
	}
	return dest;			// 返回目的字符串指针。
}
/*extern _inline char *
strcat (char *dest, const char *src)
{
  __asm__ ("cld\n\t"		// 清方向位。
	   "repne\n\t"		// 比较al 与es:[edi]字节，并更新edi++，
	   "scasb\n\t"		// 直到找到目的串中是0 的字节，此时edi 已经指向后1 字节。
	   "decl %1\n"		// 让es:[edi]指向0 值字节。
	   "1:\tlodsb\n\t"	// 取源字符串字节ds:[esi]??al，并esi++。
	   "stosb\n\t"		// 将该字节存到es:[edi]，并edi++。
	   "testb %%al,%%al\n\t"	// 该字节是0？
	   "jne 1b"		// 不是，则向后跳转到标号1 处继续拷贝，否则结束。
::"S" (src), "D" (dest), "a" (0), "c" (0xffffffff):"si", "di", "ax",
	   "cx");
  return dest;			// 返回目的字符串指针。
}*/

//// 将源字符串的count 个字节复制到目的字符串的末尾处，最后添一空字符。
// 参数：dest - 目的字符串，src - 源字符串，count - 欲复制的字节数。
// %0 - esi(src)，%1 - edi(dest)，%2 - eax(0)，%3 - ecx(-1)，%4 - (count)。
static _inline char *
strncat (char *dest, const char *src, int count)
{
	_asm {
		pushf
		mov esi,src
		mov edi,dest
		xor al,al
		mov ecx,0xffffffff
		cld		// 清方向位。
		repne scasb		// 比较al 与es:[edi]字节，edi++。直到找到目的串的末端0 值字节。
		dec edi	// edi 指向该0 值字节。
		mov ecx,count	// 欲复制字节数??ecx。
	l1: dec ecx	// ecx--（从0 开始计数）。
		js l2		// ecx <0 ?，是则向前跳转到标号l2 处。
		lodsb		// 否则取ds:[esi]处的字节->al，esi++。
		stosb		// 存储到es:[edi]处，edi++。
		test al,al	// 该字节值为0？
		jne l1		// 不是则向后跳转到标号1 处，继续复制。
	l2: xor al,al	// 将al 清零。
		stosb		// 存到es:[edi]处。
		popf
	}
  return dest;			// 返回目的字符串指针。
}
/*extern _inline char *
strncat (char *dest, const char *src, int count)
{
  __asm__ ("cld\n\t"		// 清方向位。
	   "repne\n\t"		// 比较al 与es:[edi]字节，edi++。
	   "scasb\n\t"		// 直到找到目的串的末端0 值字节。
	   "decl %1\n\t"	// edi 指向该0 值字节。
	   "movl %4,%3\n"	// 欲复制字节数??ecx。
	   "1:\tdecl %3\n\t"	// ecx--（从0 开始计数）。
	   "js 2f\n\t"		// ecx <0 ?，是则向前跳转到标号2 处。
	   "lodsb\n\t"		// 否则取ds:[esi]处的字节??al，esi++。
	   "stosb\n\t"		// 存储到es:[edi]处，edi++。
	   "testb %%al,%%al\n\t"	// 该字节值为0？
	   "jne 1b\n"		// 不是则向后跳转到标号1 处，继续复制。
	   "2:\txorl %2,%2\n\t"	// 将al 清零。
	   "stosb"		// 存到es:[edi]处。
::"S" (src), "D" (dest), "a" (0), "c" (0xffffffff), "g" (count):"si", "di", "ax",
	   "cx");
  return dest;			// 返回目的字符串指针。
}*/

//// 将一个字符串与另一个字符串进行比较。
// 参数：csrc - 字符串1，ct - 字符串2。
// %0 - eax(__res)返回值，%1 - edi(csrc)字符串1 指针，%2 - esi(ct)字符串2 指针。
// 返回：如果串1 > 串2，则返回1；串1 = 串2，则返回0；串1 < 串2，则返回-1。
extern _inline int
strcmp (const char *csrc, const char *ct)
{
//  register int __res;	// __res 是寄存器变量(eax)。
  _asm{
	  pushf
	  mov edi,csrc
	  mov esi,ct
	  cld		// 清方向位。
  l1: lodsb	// 取字符串2 的字节ds:[esi]??al，并且esi++。
	  scasb		// al 与字符串1 的字节es:[edi]作比较，并且edi++。
	  jne l2		// 如果不相等，则向前跳转到标号2。
	  test al,al	// 该字节是0 值字节吗（字符串结尾）？
	  jne l1		// 不是，则向后跳转到标号1，继续比较。
	  xor eax,eax	// 是，则返回值eax 清零，
	  jmp l3		// 向前跳转到标号3，结束。
  l2: mov eax,1	// eax 中置1。
	  jl l3		// 若前面比较中串2 字符<串1 字符，则返回正值，结束。
	  neg eax	// 否则eax = -eax，返回负值，结束。
//  l3: mov __res,eax
  l3: popf
  }
//  return __res;			// 返回比较结果。
}
/*extern _inline int
strcmp (const char *csrc, const char *ct)
{
  register int __res __asm__ ("ax");	// __res 是寄存器变量(eax)。
  __asm__ ("cld\n"		// 清方向位。
	   "1:\tlodsb\n\t"	// 取字符串2 的字节ds:[esi]??al，并且esi++。
	   "scasb\n\t"		// al 与字符串1 的字节es:[edi]作比较，并且edi++。
	   "jne 2f\n\t"		// 如果不相等，则向前跳转到标号2。
	   "testb %%al,%%al\n\t"	// 该字节是0 值字节吗（字符串结尾）？
	   "jne 1b\n\t"		// 不是，则向后跳转到标号1，继续比较。
	   "xorl %%eax,%%eax\n\t"	// 是，则返回值eax 清零，
	   "jmp 3f\n"		// 向前跳转到标号3，结束。
	   "2:\tmovl $1,%%eax\n\t"	// eax 中置1。
	   "jl 3f\n\t"		// 若前面比较中串2 字符<串1 字符，则返回正值，结束。
	   "negl %%eax\n"	// 否则eax = -eax，返回负值，结束。
"3:": "=a" (__res): "D" (csrc), "S" (ct):"si", "di");
  return __res;			// 返回比较结果。
}*/

//// 字符串1 与字符串2 的前count 个字符进行比较。
// 参数：csrc - 字符串1，ct - 字符串2，count - 比较的字符数。
// %0 - eax(__res)返回值，%1 - edi(csrc)串1 指针，%2 - esi(ct)串2 指针，%3 - ecx(count)。
// 返回：如果串1 > 串2，则返回1；串1 = 串2，则返回0；串1 < 串2，则返回-1。
static _inline int
strncmp (const char *csrc, const char *ct, int count)
{
//  register int __res;	// __res 是寄存器变量(eax)。
  _asm{
	  pushf
      mov edi,csrc
	  mov esi,ct
	  mov ecx,count
	  cld		// 清方向位。
  l1: dec ecx	// count--。
	  js l2		// 如果count<0，则向前跳转到标号2。
	  lodsb		// 取串2 的字符ds:[esi]??al，并且esi++。
	  scasb		// 比较al 与串1 的字符es:[edi]，并且edi++。
	  jne l3		// 如果不相等，则向前跳转到标号3。
	  test al,al	// 该字符是NULL 字符吗？
	  jne l1		// 不是，则向后跳转到标号1，继续比较。
  l2: xor eax,eax	// 是NULL 字符，则eax 清零（返回值）。
	  jmp l4		// 向前跳转到标号4，结束。
  l3: mov eax,1	// eax 中置1。
	  jl l4		// 如果前面比较中串2 字符<串2 字符，则返回1，结束。
	  neg eax	// 否则eax = -eax，返回负值，结束。
//  l4: mov __res,eax
  l4: popf
  }
//  return __res;			// 返回比较结果。
}
/*extern _inline int
strncmp (const char *csrc, const char *ct, int count)
{
  register int __res __asm__ ("ax");	// __res 是寄存器变量(eax)。
  __asm__ ("cld\n"		// 清方向位。
	   "1:\tdecl %3\n\t"	// count--。
	   "js 2f\n\t"		// 如果count<0，则向前跳转到标号2。
	   "lodsb\n\t"		// 取串2 的字符ds:[esi]??al，并且esi++。
	   "scasb\n\t"		// 比较al 与串1 的字符es:[edi]，并且edi++。
	   "jne 3f\n\t"		// 如果不相等，则向前跳转到标号3。
	   "testb %%al,%%al\n\t"	// 该字符是NULL 字符吗？
	   "jne 1b\n"		// 不是，则向后跳转到标号1，继续比较。
	   "2:\txorl %%eax,%%eax\n\t"	// 是NULL 字符，则eax 清零（返回值）。
	   "jmp 4f\n"		// 向前跳转到标号4，结束。
	   "3:\tmovl $1,%%eax\n\t"	// eax 中置1。
	   "jl 4f\n\t"		// 如果前面比较中串2 字符<串2 字符，则返回1，结束。
	   "negl %%eax\n"	// 否则eax = -eax，返回负值，结束。
"4:": "=a" (__res): "D" (csrc), "S" (ct), "c" (count):"si", "di",
	   "cx");
  return __res;			// 返回比较结果。
}*/

//// 在字符串中寻找第一个匹配的字符。
// 参数：s - 字符串，c - 欲寻找的字符。
// %0 - eax(__res)，%1 - esi(字符串指针s)，%2 - eax(字符c)。
// 返回：返回字符串中第一次出现匹配字符的指针。若没有找到匹配的字符，则返回空指针。
static _inline char *
strchr (const char *s, char c)
{
//  register char *__res;	// __res 是寄存器变量(eax)。
  _asm{
	  pushf
	  mov esi,s
	  mov ah,c
	  cld		// 清方向位。
  l1: lodsb	// 取字符串中字符ds:[esi]->al，并且esi++。
	  cmp al,ah	// 字符串中字符al 与指定字符ah 相比较。
	  je l2		// 若相等，则向前跳转到标号2 处。
	  test al,al	// al 中字符是NULL 字符吗？（字符串结尾？）
	  jne l1		// 若不是，则向后跳转到标号1，继续比较。
	  mov esi,1	// 是，则说明没有找到匹配字符，esi 置1。
  l2: mov eax,esi	// 将指向匹配字符后一个字节处的指针值放入eax
	  dec eax		// 将指针调整为指向匹配的字符。
//	  mov __res,eax
	  popf
  }
//  return __res;			// 返回指针。
}
/*extern _inline char *
strchr (const char *s, char c)
{
  register char *__res __asm__ ("ax");	// __res 是寄存器变量(eax)。
  __asm__ ("cld\n\t"		// 清方向位。
	   "movb %%al,%%ah\n"	// 将欲比较字符移到ah。
	   "1:\tlodsb\n\t"	// 取字符串中字符ds:[esi]??al，并且esi++。
	   "cmpb %%ah,%%al\n\t"	// 字符串中字符al 与指定字符ah 相比较。
	   "je 2f\n\t"		// 若相等，则向前跳转到标号2 处。
	   "testb %%al,%%al\n\t"	// al 中字符是NULL 字符吗？（字符串结尾？）
	   "jne 1b\n\t"		// 若不是，则向后跳转到标号1，继续比较。
	   "movl $1,%1\n"	// 是，则说明没有找到匹配字符，esi 置1。
	   "2:\tmovl %1,%0\n\t"	// 将指向匹配字符后一个字节处的指针值放入eax
	   "decl %0"		// 将指针调整为指向匹配的字符。
: "=a" (__res): "S" (s), "" (c):"si");
  return __res;			// 返回指针。
}*/

//// 寻找字符串中指定字符最后一次出现的地方。（反向搜索字符串）
// 参数：s - 字符串，c - 欲寻找的字符。
// %0 - edx(__res)，%1 - edx(0)，%2 - esi(字符串指针s)，%3 - eax(字符c)。
// 返回：返回字符串中最后一次出现匹配字符的指针。若没有找到匹配的字符，则返回空指针。
static _inline char *
strrchr (const char *s, char c)
{
//  register char *__res;	// __res 是寄存器变量(edx)。
  _asm{
	  pushf
	  xor edx,edx
	  mov esi,s
	  mov ah,c
	  cld		// 清方向位。
  l1: lodsb	// 取字符串中字符ds:[esi]->al，并且esi++。
	  cmp al,ah	// 字符串中字符al 与指定字符ah 作比较。
	  jne l2		// 若不相等，则向前跳转到标号2 处。
	  mov edx,esi	// 将字符指针保存到edx 中。
	  dec edx		// 指针后退一位，指向字符串中匹配字符处。
  l2: test al,al	// 比较的字符是0 吗（到字符串尾）？
	  jne l1	// 不是则向后跳转到标号1 处，继续比较。
//	  mov __res,edx
      mov eax,edx
	  popf
  }
//  return __res;			// 返回指针。
}
/*extern _inline char *
strrchr (const char *s, char c)
{
  register char *__res __asm__ ("dx");	// __res 是寄存器变量(edx)。
  __asm__ ("cld\n\t"		// 清方向位。
	   "movb %%al,%%ah\n"	// 将欲寻找的字符移到ah。
	   "1:\tlodsb\n\t"	// 取字符串中字符ds:[esi]??al，并且esi++。
	   "cmpb %%ah,%%al\n\t"	// 字符串中字符al 与指定字符ah 作比较。
	   "jne 2f\n\t"		// 若不相等，则向前跳转到标号2 处。
	   "movl %%esi,%0\n\t"	// 将字符指针保存到edx 中。
	   "decl %0\n"		// 指针后退一位，指向字符串中匹配字符处。
	   "2:\ttestb %%al,%%al\n\t"	// 比较的字符是0 吗（到字符串尾）？
	   "jne 1b"		// 不是则向后跳转到标号1 处，继续比较。
: "=d" (__res): "" (0), "S" (s), "a" (c):"ax", "si");
  return __res;			// 返回指针。
}*/

//// 在字符串1 中寻找第1 个字符序列，该字符序列中的任何字符都包含在字符串2 中。
// 参数：csrc - 字符串1 指针，ct - 字符串2 指针。
// %0 - esi(__res)，%1 - eax(0)，%2 - ecx(-1)，%3 - esi(串1 指针csrc)，%4 - (串2 指针ct)。
// 返回字符串1 中包含字符串2 中任何字符的首个字符序列的长度值。
extern _inline int
strspn (const char *csrc, const char *ct)
{
  register char *__res;	// __res 是寄存器变量(esi)。
  _asm{
	  pushf
	  xor al,al
	  mov ebx,ct
	  mov edi,ebx	// 首先计算串2 的长度。串2 指针放入edi 中。
	  mov ecx,0xffffffff
	  cld		// 清方向位。
	  repne scasb// 比较al(0)与串2 中的字符（es:[edi]），并edi++。如果不相等就继续比较(ecx 逐步递减)。
	  not ecx	// ecx 中每位取反。
	  dec ecx	// ecx--，得串2 的长度值。-> ecx
	  mov edx,ecx	// 将串2 的长度值暂放入edx 中。
	  mov esi,csrc
  l1: lodsb	// 取串1 字符ds:[esi]->al，并且esi++。
	  test al,al	// 该字符等于0 值吗（串1 结尾）？
	  je l2		// 如果是，则向前跳转到标号2 处。
	  mov edi,ebx	// 取串2 头指针放入edi 中。
	  mov ecx,edx	// 再将串2 的长度值放入ecx 中。
	  repne scasb	// 比较al 与串2 中字符es:[edi]，并且edi++。如果不相等就继续比较。
	  je l1		// 如果相等，则向后跳转到标号1 处。
  l2: dec esi	// esi--，指向最后一个包含在串2 中的字符。
	  mov __res,esi
	  popf
  }
  return __res - csrc;		// 返回字符序列的长度值。
}
/*extern _inline int
strspn (const char *csrc, const char *ct)
{
  register char *__res __asm__ ("si");	// __res 是寄存器变量(esi)。
  __asm__ ("cld\n\t"		// 清方向位。
	   "movl %4,%%edi\n\t"	// 首先计算串2 的长度。串2 指针放入edi 中。
	   "repne\n\t"		// 比较al(0)与串2 中的字符（es:[edi]），并edi++。
	   "scasb\n\t"		// 如果不相等就继续比较(ecx 逐步递减)。
	   "notl %%ecx\n\t"	// ecx 中每位取反。
	   "decl %%ecx\n\t"	// ecx--，得串2 的长度值。
	   "movl %%ecx,%%edx\n"	// 将串2 的长度值暂放入edx 中。
	   "1:\tlodsb\n\t"	// 取串1 字符ds:[esi]??al，并且esi++。
	   "testb %%al,%%al\n\t"	// 该字符等于0 值吗（串1 结尾）？
	   "je 2f\n\t"		// 如果是，则向前跳转到标号2 处。
	   "movl %4,%%edi\n\t"	// 取串2 头指针放入edi 中。
	   "movl %%edx,%%ecx\n\t"	// 再将串2 的长度值放入ecx 中。
	   "repne\n\t"		// 比较al 与串2 中字符es:[edi]，并且edi++。
	   "scasb\n\t"		// 如果不相等就继续比较。
	   "je 1b\n"		// 如果相等，则向后跳转到标号1 处。
	   "2:\tdecl %0"	// esi--，指向最后一个包含在串2 中的字符。
: "=S" (__res): "a" (0), "c" (0xffffffff), "" (csrc), "g" (ct):"ax", "cx", "dx",
	   "di");
  return __res - csrc;		// 返回字符序列的长度值。
}*/

//// 寻找字符串1 中不包含字符串2 中任何字符的首个字符序列。
// 参数：csrc - 字符串1 指针，ct - 字符串2 指针。
// %0 - esi(__res)，%1 - eax(0)，%2 - ecx(-1)，%3 - esi(串1 指针csrc)，%4 - (串2 指针ct)。
// 返回字符串1 中不包含字符串2 中任何字符的首个字符序列的长度值。
extern _inline int
strcsrcpn (const char *csrc, const char *ct)
{
  register char *__res;	// __res 是寄存器变量(esi)。
	_asm{
		pushf
		xor al,al
		mov ecx,0xffffffff
		mov ebx,ct
		mov edi,ebx	// 首先计算串2 的长度。串2 指针放入edi 中。
		cld		// 清方向位。
		repne scasb// 比较al(0)与串2 中的字符（es:[edi]），并edi++。如果不相等就继续比较(ecx 逐步递减)。
		not ecx	// ecx 中每位取反。
		dec ecx	// ecx--，得串2 的长度值。
		mov edx,ecx	// 将串2 的长度值暂放入edx 中。
		mov esi,csrc
	l1: lodsb	// 取串1 字符ds:[esi]->al，并且esi++。
		test al,al	// 该字符等于0 值吗（串1 结尾）？
		je l2		// 如果是，则向前跳转到标号2 处。
		mov edi,ebx	// 取串2 头指针放入edi 中。
		mov ecx,edx 	// 再将串2 的长度值放入ecx 中。
		repne scasb	// 比较al 与串2 中字符es:[edi]，并且edi++。如果不相等就继续比较。
		jne l1		// 如果不相等，则向后跳转到标号1 处。
	l2: dec esi
		mov __res,esi	// esi--，指向最后一个包含在串2 中的字符。
		popf
	}
  return __res - csrc;		// 返回字符序列的长度值。
}
/*extern _inline int
strcsrcpn (const char *csrc, const char *ct)
{
  register char *__res __asm__ ("si");	// __res 是寄存器变量(esi)。
  __asm__ ("cld\n\t"		// 清方向位。
	   "movl %4,%%edi\n\t"	// 首先计算串2 的长度。串2 指针放入edi 中。
	   "repne\n\t"		// 比较al(0)与串2 中的字符（es:[edi]），并edi++。
	   "scasb\n\t"		// 如果不相等就继续比较(ecx 逐步递减)。
	   "notl %%ecx\n\t"	// ecx 中每位取反。
	   "decl %%ecx\n\t"	// ecx--，得串2 的长度值。
	   "movl %%ecx,%%edx\n"	// 将串2 的长度值暂放入edx 中。
	   "1:\tlodsb\n\t"	// 取串1 字符ds:[esi]??al，并且esi++。
	   "testb %%al,%%al\n\t"	// 该字符等于0 值吗（串1 结尾）？
	   "je 2f\n\t"		// 如果是，则向前跳转到标号2 处。
	   "movl %4,%%edi\n\t"	// 取串2 头指针放入edi 中。
	   "movl %%edx,%%ecx\n\t"	// 再将串2 的长度值放入ecx 中。
	   "repne\n\t"		// 比较al 与串2 中字符es:[edi]，并且edi++。
	   "scasb\n\t"		// 如果不相等就继续比较。
	   "jne 1b\n"		// 如果不相等，则向后跳转到标号1 处。
	   "2:\tdecl %0"	// esi--，指向最后一个包含在串2 中的字符。
: "=S" (__res): "a" (0), "c" (0xffffffff), "" (csrc), "g" (ct):"ax", "cx", "dx",
	   "di");
  return __res - csrc;		// 返回字符序列的长度值。
}*/

//// 在字符串1 中寻找首个包含在字符串2 中的任何字符。
// 参数：csrc - 字符串1 的指针，ct - 字符串2 的指针。
// %0 -esi(__res)，%1 -eax(0)，%2 -ecx(0xffffffff)，%3 -esi(串1 指针csrc)，%4 -(串2 指针ct)。
// 返回字符串1 中首个包含字符串2 中字符的指针。
extern _inline char *
strpbrk (const char *csrc, const char *ct)
{
//  register char *__res;	// __res 是寄存器变量(esi)。
	_asm{
		pushf
		xor al,al
		mov ebx,ct
		mov edi,ebx	// 首先计算串2 的长度。串2 指针放入edi 中。
		mov ecx,0xffffffff
		cld		// 清方向位。
		repne scasb// 比较al(0)与串2 中的字符（es:[edi]），并edi++。如果不相等就继续比较(ecx 逐步递减)。
		not ecx	// ecx 中每位取反。
		dec ecx	// ecx--，得串2 的长度值。
		mov edx,ecx	// 将串2 的长度值暂放入edx 中。
		mov esi,csrc
	l1: lodsb	// 取串1 字符ds:[esi]??al，并且esi++。
		test al,al	// 该字符等于0 值吗（串1 结尾）？
		je l2		// 如果是，则向前跳转到标号2 处。
		mov edi,ebx	// 取串2 头指针放入edi 中。
		mov ecx,edx	// 再将串2 的长度值放入ecx 中。
		repne scasb		// 比较al 与串2 中字符es:[edi]，并且edi++。如果不相等就继续比较。
		jne l1	// 如果不相等，则向后跳转到标号1 处。
		dec esi	// esi--，指向一个包含在串2 中的字符。
		jmp l3		// 向前跳转到标号3 处。
	l2: xor esi,esi	// 没有找到符合条件的，将返回值为NULL。
//	l3: mov __res,esi
	l3: mov eax,esi
		popf
	}
//  return __res;			// 返回指针值。
}
/*extern _inline char *
strpbrk (const char *csrc, const char *ct)
{
  register char *__res __asm__ ("si");	// __res 是寄存器变量(esi)。
  __asm__ ("cld\n\t"		// 清方向位。
	   "movl %4,%%edi\n\t"	// 首先计算串2 的长度。串2 指针放入edi 中。
	   "repne\n\t"		// 比较al(0)与串2 中的字符（es:[edi]），并edi++。
	   "scasb\n\t"		// 如果不相等就继续比较(ecx 逐步递减)。
	   "notl %%ecx\n\t"	// ecx 中每位取反。
	   "decl %%ecx\n\t"	// ecx--，得串2 的长度值。
	   "movl %%ecx,%%edx\n"	// 将串2 的长度值暂放入edx 中。
	   "1:\tlodsb\n\t"	// 取串1 字符ds:[esi]??al，并且esi++。
	   "testb %%al,%%al\n\t"	// 该字符等于0 值吗（串1 结尾）？
	   "je 2f\n\t"		// 如果是，则向前跳转到标号2 处。
	   "movl %4,%%edi\n\t"	// 取串2 头指针放入edi 中。
	   "movl %%edx,%%ecx\n\t"	// 再将串2 的长度值放入ecx 中。
	   "repne\n\t"		// 比较al 与串2 中字符es:[edi]，并且edi++。
	   "scasb\n\t"		// 如果不相等就继续比较。
	   "jne 1b\n\t"		// 如果不相等，则向后跳转到标号1 处。
	   "decl %0\n\t"	// esi--，指向一个包含在串2 中的字符。
	   "jmp 3f\n"		// 向前跳转到标号3 处。
	   "2:\txorl %0,%0\n"	// 没有找到符合条件的，将返回值为NULL。
"3:": "=S" (__res): "a" (0), "c" (0xffffffff), "" (csrc), "g" (ct):"ax", "cx", "dx",
	   "di");
  return __res;			// 返回指针值。
}*/

//// 在字符串1 中寻找首个匹配整个字符串2 的字符串。
// 参数：csrc - 字符串1 的指针，ct - 字符串2 的指针。
// %0 -eax(__res)，%1 -eax(0)，%2 -ecx(0xffffffff)，%3 -esi(串1 指针csrc)，%4 -(串2 指针ct)。
// 返回：返回字符串1 中首个匹配字符串2 的字符串指针。
extern _inline char *
strstr (const char *csrc, const char *ct)
{
//  register char *__res;	// __res 是寄存器变量(eax)。
	_asm {
		pushf
		mov ebx,ct
		mov edi,ebx	// 首先计算串2 的长度。串2 指针放入edi 中。
		mov ecx,0xffffffff
		xor al,al	// al = 0
		cld 		// 清方向位。
		repne scasb// 比较al(0)与串2 中的字符（es:[edi]），并edi++。如果不相等就继续比较(ecx 逐步递减)。
		not ecx	// ecx 中每位取反。
		dec ecx
// 注意！如果搜索串为空，将设置Z 标志 // 得串2 的长度值。
		mov edx,ecx	// 将串2 的长度值暂放入edx 中。
		mov esi,csrc
	l1: mov edi,ebx	// 取串2 头指针放入edi 中。
		mov ecx,edx	// 再将串2 的长度值放入ecx 中。
		mov eax,esi	// 将串1 的指针复制到eax 中。
		repe cmpsb// 比较串1 和串2 字符(ds:[esi],es:[edi])，esi++,edi++。若对应字符相等就一直比较下去。
		je l2
// 对空串同样有效，见上面 // 若全相等，则转到标号2。
		xchg esi,eax	// 串1 头指针->esi，比较结果的串1 指针->eax。
		inc esi	// 串1 头指针指向下一个字符。
		cmp [eax-1],0	// 串1 指针(eax-1)所指字节是0 吗？
		jne l1	// 不是则跳转到标号1，继续从串1 的第2 个字符开始比较。
		xor eax,eax	// 清eax，表示没有找到匹配。
//	l2: mov __res,eax
	l2:	popf
	}
//  return __res;			// 返回比较结果。
}
/*extern _inline char *
strstr (const char *csrc, const char *ct)
{
  register char *__res __asm__ ("ax");	// __res 是寄存器变量(eax)。
  __asm__ ("cld\n\t" \		// 清方向位。
	   "movl %4,%%edi\n\t"	// 首先计算串2 的长度。串2 指针放入edi 中。
	   "repne\n\t"		// 比较al(0)与串2 中的字符（es:[edi]），并edi++。
	   "scasb\n\t"		// 如果不相等就继续比较(ecx 逐步递减)。
	   "notl %%ecx\n\t"	// ecx 中每位取反。
	   "decl %%ecx\n\t"	// NOTE! This also sets Z if searchstring=''
// 注意！如果搜索串为空，将设置Z 标志 // 得串2 的长度值。
	   "movl %%ecx,%%edx\n"	// 将串2 的长度值暂放入edx 中。
	   "1:\tmovl %4,%%edi\n\t"	// 取串2 头指针放入edi 中。
	   "movl %%esi,%%eax\n\t"	// 将串1 的指针复制到eax 中。
	   "movl %%edx,%%ecx\n\t"	// 再将串2 的长度值放入ecx 中。
	   "repe\n\t"		// 比较串1 和串2 字符(ds:[esi],es:[edi])，esi++,edi++。
	   "cmpsb\n\t"		// 若对应字符相等就一直比较下去。
	   "je 2f\n\t"		// also works for empty string, see above 
// 对空串同样有效，见上面 // 若全相等，则转到标号2。
	   "xchgl %%eax,%%esi\n\t"	// 串1 头指针??esi，比较结果的串1 指针??eax。
	   "incl %%esi\n\t"	// 串1 头指针指向下一个字符。
	   "cmpb $0,-1(%%eax)\n\t"	// 串1 指针(eax-1)所指字节是0 吗？
	   "jne 1b\n\t"		// 不是则跳转到标号1，继续从串1 的第2 个字符开始比较。
	   "xorl %%eax,%%eax\n\t"	// 清eax，表示没有找到匹配。
"2:": "=a" (__res): "" (0), "c" (0xffffffff), "S" (csrc), "g" (ct):"cx", "dx", "di",
	   "si");
  return __res;			// 返回比较结果。
}*/

//// 计算字符串长度。
// 参数：s - 字符串。
// %0 - ecx(__res)，%1 - edi(字符串指针s)，%2 - eax(0)，%3 - ecx(0xffffffff)。
// 返回：返回字符串的长度。
extern _inline int
strlen (const char *s)
{
//  register int __res;	// __res 是寄存器变量(ecx)。
	_asm{
		pushf
		mov edi,s
		mov ecx,0xffffffff
		xor al,al
		cld		// 清方向位。
		repne scasb		// al(0)与字符串中字符es:[edi]比较，若不相等就一直比较。
		not ecx	// ecx 取反。
		dec ecx		// ecx--，得字符串得长度值。
//		mov __res,ecx
		mov eax,ecx
		popf
	}
//  return __res;			// 返回字符串长度值。
}
/*extern _inline int
strlen (const char *s)
{
  register int __res __asm__ ("cx");	// __res 是寄存器变量(ecx)。
  __asm__ ("cld\n\t"		// 清方向位。
	   "repne\n\t"		// al(0)与字符串中字符es:[edi]比较，
	   "scasb\n\t"		// 若不相等就一直比较。
	   "notl %0\n\t"	// ecx 取反。
	   "decl %0"		// ecx--，得字符串得长度值。
: "=c" (__res): "D" (s), "a" (0), "" (0xffffffff):"di");
  return __res;			// 返回字符串长度值。
}*/

extern char *___strtok;		// 用于临时存放指向下面被分析字符串1(s)的指针。

//// 利用字符串2 中的字符将字符串1 分割成标记(tokern)序列。
// 将串1 看作是包含零个或多个单词(token)的序列，并由分割符字符串2 中的一个或多个字符分开。
// 第一次调用strtok()时，将返回指向字符串1 中第1 个token 首字符的指针，并在返回token 时将
// 一null 字符写到分割符处。后续使用null 作为字符串1 的调用，将用这种方法继续扫描字符串1，
// 直到没有token 为止。在不同的调用过程中，分割符串2 可以不同。
// 参数：s - 待处理的字符串1，ct - 包含各个分割符的字符串2。
// 汇编输出：%0 - ebx(__res)，%1 - esi(__strtok)；
// 汇编输入：%2 - ebx(__strtok)，%3 - esi(字符串1 指针s)，%4 - （字符串2 指针ct）。
// 返回：返回字符串s 中第1 个token，如果没有找到token，则返回一个null 指针。
// 后续使用字符串s 指针为null 的调用，将在原字符串s 中搜索下一个token。
extern _inline char *
strtok (char *s, register const char *ct)
{
//  register char *__res;
	_asm {
		pushf
		mov esi,s
		test esi,esi	// 首先测试esi(字符串1 指针s)是否是NULL。
		jne l1		// 如果不是，则表明是首次调用本函数，跳转标号1。
		mov ebx,strtok
		test ebx,ebx	// 如果是NULL，则表示此次是后续调用，测ebx(__strtok)。
		je l8		// 如果ebx 指针是NULL，则不能处理，跳转结束。
		mov esi,ebx	// 将ebx 指针复制到esi。
	l1: xor ebx,ebx	// 清ebx 指针。
		mov edi,ct	// 下面求字符串2 的长度。edi 指向字符串2。
		mov ecx,0xffffffff 	// 置ecx = 0xffffffff。
		xor eax,eax	// 清零eax。
		cld
		repne scasb// 将al(0)与es:[edi]比较，并且edi++。直到找到字符串2 的结束null 字符，或计数ecx==0。
		not ecx	// 将ecx 取反，
		dec ecx	// ecx--，得到字符串2 的长度值。
		je l7	// 分割符字符串空 // 若串2 长度为0，则转标号7。
		mov edx,ecx	// 将串2 长度暂存入edx。
	l2: lodsb	// 取串1 的字符ds:[esi]->al，并且esi++。
		test al,al	// 该字符为0 值吗(串1 结束)？
		je l7		// 如果是，则跳转标号7。
		mov edi,ct	// edi 再次指向串2 首。
		mov ecx,edx	// 取串2 的长度值置入计数器ecx。
		repne scasb// 将al 中串1 的字符与串2 中所有字符比较，判断该字符是否为分割符。
		je l2		// 若能在串2 中找到相同字符（分割符），则跳转标号2。
		dec esi	// 若不是分割符，则串1 指针esi 指向此时的该字符。
		cmp [esi],0	// 该字符是NULL 字符吗？
		je l7		// 若是，则跳转标号7 处。
		mov ebx,esi	// 将该字符的指针esi 存放在ebx。
	l3: lodsb	// 取串1 下一个字符ds:[esi]->al，并且esi++。
		test al,al	// 该字符是NULL 字符吗？
		je l5		// 若是，表示串1 结束，跳转到标号5。
		mov edi,ct	// edi 再次指向串2 首。
		mov ecx,edx	// 串2 长度值置入计数器ecx。
		repne scasb // 将al 中串1 的字符与串2 中每个字符比较，测试al 字符是否是分割符。
		jne l3		// 若不是分割符则跳转标号3，检测串1 中下一个字符。
		dec esi	// 若是分割符，则esi--，指向该分割符字符。
		cmp [esi],0	// 该分割符是NULL 字符吗？
		je l5		// 若是，则跳转到标号5。
		mov [esi],0	// 若不是，则将该分割符用NULL 字符替换掉。
		inc esi	// esi 指向串1 中下一个字符，也即剩余串首。
		jmp l6		// 跳转标号6 处。
	l5: xor esi,esi	// esi 清零。
	l6: cmp [ebx],0	// ebx 指针指向NULL 字符吗？
		jne l7	// 若不是，则跳转标号7。
		xor ebx,ebx	// 若是，则让ebx=NULL。
	l7: test ebx,ebx	// ebx 指针为NULL 吗？
		jne l8	// 若不是则跳转8，结束汇编代码。
		mov esi,ebx	// 将esi 置为NULL。
//	l8: mov __res,esi
	l8: mov eax,esi
		popf
	}
//  return __res;			// 返回指向新token 的指针。
}
/*extern _inline char *
strtok (char *s, const char *ct)
{
  register char *__res __asm__ ("si");
  __asm__ ("testl %1,%1\n\t"	// 首先测试esi(字符串1 指针s)是否是NULL。
	   "jne 1f\n\t"		// 如果不是，则表明是首次调用本函数，跳转标号1。
	   "testl %0,%0\n\t"	// 如果是NULL，则表示此次是后续调用，测ebx(__strtok)。
	   "je 8f\n\t"		// 如果ebx 指针是NULL，则不能处理，跳转结束。
	   "movl %0,%1\n"	// 将ebx 指针复制到esi。
	   "1:\txorl %0,%0\n\t"	// 清ebx 指针。
	   "movl $-1,%%ecx\n\t"	// 置ecx = 0xffffffff。
	   "xorl %%eax,%%eax\n\t"	// 清零eax。
	   "cld\n\t"		// 清方向位。
	   "movl %4,%%edi\n\t"	// 下面求字符串2 的长度。edi 指向字符串2。
	   "repne\n\t"		// 将al(0)与es:[edi]比较，并且edi++。
	   "scasb\n\t"		// 直到找到字符串2 的结束null 字符，或计数ecx==0。
	   "notl %%ecx\n\t"	// 将ecx 取反，
	   "decl %%ecx\n\t"	// ecx--，得到字符串2 的长度值。
	   "je 7f\n\t"		// empty delimeter-string 
// 分割符字符串空 // 若串2 长度为0，则转标号7。
	   "movl %%ecx,%%edx\n"	// 将串2 长度暂存入edx。
	   "2:\tlodsb\n\t"	// 取串1 的字符ds:[esi]??al，并且esi++。
	   "testb %%al,%%al\n\t"	// 该字符为0 值吗(串1 结束)？
	   "je 7f\n\t"		// 如果是，则跳转标号7。
	   "movl %4,%%edi\n\t"	// edi 再次指向串2 首。
	   "movl %%edx,%%ecx\n\t"	// 取串2 的长度值置入计数器ecx。
	   "repne\n\t"		// 将al 中串1 的字符与串2 中所有字符比较，
	   "scasb\n\t"		// 判断该字符是否为分割符。
	   "je 2b\n\t"		// 若能在串2 中找到相同字符（分割符），则跳转标号2。
	   "decl %1\n\t"	// 若不是分割符，则串1 指针esi 指向此时的该字符。
	   "cmpb $0,(%1)\n\t"	// 该字符是NULL 字符吗？
	   "je 7f\n\t"		// 若是，则跳转标号7 处。
	   "movl %1,%0\n"	// 将该字符的指针esi 存放在ebx。
	   "3:\tlodsb\n\t"	// 取串1 下一个字符ds:[esi]??al，并且esi++。
	   "testb %%al,%%al\n\t"	// 该字符是NULL 字符吗？
	   "je 5f\n\t"		// 若是，表示串1 结束，跳转到标号5。
	   "movl %4,%%edi\n\t"	// edi 再次指向串2 首。
	   "movl %%edx,%%ecx\n\t"	// 串2 长度值置入计数器ecx。
	   "repne\n\t"		// 将al 中串1 的字符与串2 中每个字符比较，
	   "scasb\n\t"		// 测试al 字符是否是分割符。
	   "jne 3b\n\t"		// 若不是分割符则跳转标号3，检测串1 中下一个字符。
	   "decl %1\n\t"	// 若是分割符，则esi--，指向该分割符字符。
	   "cmpb $0,(%1)\n\t"	// 该分割符是NULL 字符吗？
	   "je 5f\n\t"		// 若是，则跳转到标号5。
	   "movb $0,(%1)\n\t"	// 若不是，则将该分割符用NULL 字符替换掉。
	   "incl %1\n\t"	// esi 指向串1 中下一个字符，也即剩余串首。
	   "jmp 6f\n"		// 跳转标号6 处。
	   "5:\txorl %1,%1\n"	// esi 清零。
	   "6:\tcmpb $0,(%0)\n\t"	// ebx 指针指向NULL 字符吗？
	   "jne 7f\n\t"		// 若不是，则跳转标号7。
	   "xorl %0,%0\n"	// 若是，则让ebx=NULL。
	   "7:\ttestl %0,%0\n\t"	// ebx 指针为NULL 吗？
	   "jne 8f\n\t"		// 若不是则跳转8，结束汇编代码。
	   "movl %0,%1\n"	// 将esi 置为NULL。
"8:": "=b" (__res), "=S" (___strtok): "" (___strtok), "1" (s), "g" (ct):"ax", "cx", "dx",
	   "di");
  return __res;			// 返回指向新token 的指针。
}*/

//// 内存块复制。从源地址src 处开始复制n 个字节到目的地址dest 处。
// 参数：dest - 复制的目的地址，src - 复制的源地址，n - 复制字节数。
// %0 - ecx(n)，%1 - esi(src)，%2 - edi(dest)。
extern _inline void *
memcpy (void *dest, const void *src, int n)
{
	_asm{
		pushf
		mov esi,src
		mov edi,dest
		mov ecx,n
		cld		// 清方向位。
		rep movsb	// 重复执行复制ecx 个字节，从ds:[esi]到es:[edi]，esi++，edi++。
		popf
	}
	return dest;			// 返回目的地址。
}
/*extern _inline void *
memcpy (void *dest, const void *src, int n)
{
  __asm__ ("cld\n\t"		// 清方向位。
	   "rep\n\t"		// 重复执行复制ecx 个字节，
	   "movsb"		// 从ds:[esi]到es:[edi]，esi++，edi++。
::"c" (n), "S" (src), "D" (dest):"cx", "si", "di");
  return dest;			// 返回目的地址。
}*/

//// 内存块移动。同内存块复制，但考虑移动的方向。
// 参数：dest - 复制的目的地址，src - 复制的源地址，n - 复制字节数。
// 若dest<src 则：%0 - ecx(n)，%1 - esi(src)，%2 - edi(dest)。
// 否则：%0 - ecx(n)，%1 - esi(src+n-1)，%2 - edi(dest+n-1)。
// 这样操作是为了防止在复制时错误地重叠覆盖。
extern _inline void *
memmove (void *dest, const void *src, int n)
{
	if (dest < src) 
	{_asm {
		pushf
		mov esi,src
		mov edi,dest
		mov ecx,n
		cld		// 清方向位。
		rep movsb// 从ds:[esi]到es:[edi]，并且esi++，edi++，重复执行复制ecx 字节。
		popf
	}}else{_asm {
		pushf
//		mov esi,src + n - 1
		mov esi,src
		add esi,n - 1
//		mov edi,dest + n - 1
		mov edi,dest
		add edi,n - 1
		mov ecx,n
		std		// 置方向位，从末端开始复制。
		rep movsb// 从ds:[esi]到es:[edi]，并且esi--，edi--，复制ecx 个字节。
		popf
	}}
  return dest;
}
/*extern _inline void *
memmove (void *dest, const void *src, int n)
{
  if (dest < src)
    __asm__ ("cld\n\t"		// 清方向位。
	     "rep\n\t"		// 从ds:[esi]到es:[edi]，并且esi++，edi++，
	     "movsb"		// 重复执行复制ecx 字节。
  ::"c" (n), "S" (src), "D" (dest):"cx", "si", "di");
  else
    __asm__ ("std\n\t"		// 置方向位，从末端开始复制。
	     "rep\n\t"		// 从ds:[esi]到es:[edi]，并且esi--，edi--，
	     "movsb"		// 复制ecx 个字节。
  ::"c" (n), "S" (src + n - 1), "D" (dest + n - 1):"cx", "si",
	     "di");
  return dest;
}*/

//// 比较n 个字节的两块内存（两个字符串），即使遇上NULL 字节也不停止比较。
// 参数：csrc - 内存块1 地址，ct - 内存块2 地址，count - 比较的字节数。
// %0 - eax(__res)，%1 - eax(0)，%2 - edi(内存块1)，%3 - esi(内存块2)，%4 - ecx(count)。
// 返回：若块1>块2 返回1；块1<块2，返回-1；块1==块2，则返回0。
extern _inline int
memcmp (const void *csrc, const void *ct, int count)
{
//  register int __res; //__asm__ ("ax")	 __res 是寄存器变量。
	_asm {
		pushf
		mov edi,csrc
		mov esi,ct
		xor eax,eax // eax = 0
		mov ecx,count
		cld		// 清方向位。
		repe cmpsb// 比较ds:[esi]与es:[edi]的内容，并且esi++，edi++。如果相等则重复，
		je l1		// 如果都相同，则跳转到标号1，返回0(eax)值
		mov eax,1	// 否则eax 置1，
		jl l1		// 若内存块2 内容的值<内存块1，则跳转标号1。
		neg eax	// 否则eax = -eax。
//	l1: mov __res,eax
	l1:	popf
	}
//  return __res;			// 返回比较结果。
}
/*extern _inline int
memcmp (const void *csrc, const void *ct, int count)
{
  register int __res __asm__ ("ax");	// __res 是寄存器变量。
  __asm__ ("cld\n\t"		// 清方向位。
	   "repe\n\t"		// 如果相等则重复，
	   "cmpsb\n\t"		// 比较ds:[esi]与es:[edi]的内容，并且esi++，edi++。
	   "je 1f\n\t"		// 如果都相同，则跳转到标号1，返回0(eax)值
	   "movl $1,%%eax\n\t"	// 否则eax 置1，
	   "jl 1f\n\t"		// 若内存块2 内容的值<内存块1，则跳转标号1。
	   "negl %%eax\n"	// 否则eax = -eax。
"1:": "=a" (__res): "" (0), "D" (csrc), "S" (ct), "c" (count):"si", "di",
	   "cx");
  return __res;			// 返回比较结果。
}*/

//// 在n 字节大小的内存块(字符串)中寻找指定字符。
// 参数：csrc - 指定内存块地址，c - 指定的字符，count - 内存块长度。
// %0 - edi(__res)，%1 - eax(字符c)，%2 - edi(内存块地址csrc)，%3 - ecx(字节数count)。
// 返回第一个匹配字符的指针，如果没有找到，则返回NULL 字符。
extern _inline void *
memchr (const void *csrc, char c, int count)
{
//  register void *__res;	// __res 是寄存器变量。
  if (!count)			// 如果内存块长度==0，则返回NULL，没有找到。
    return NULL;
	_asm {
		pushf
		mov edi,csrc
		mov ecx,count
		mov al,c
		cld		// 清方向位。
		repne scasb// al 中字符与es:[edi]字符作比较，并且edi++，如果不相等则重复执行下面语句，
		je l1		// 如果相等则向前跳转到标号1 处。
		mov edi,1	// 否则edi 中置1。
	l1: dec edi	// 让edi 指向找到的字符（或是NULL）。
//		mov __res,edi
		mov eax,edi
		popf
	}
//  return __res;			// 返回字符指针。
}
/*extern _inline void *
memchr (const void *csrc, char c, int count)
{
  register void *__res __asm__ ("di");	// __res 是寄存器变量。
  if (!count)			// 如果内存块长度==0，则返回NULL，没有找到。
    return NULL;
  __asm__ ("cld\n\t"		// 清方向位。
	   "repne\n\t"		// 如果不相等则重复执行下面语句，
	   "scasb\n\t"		// al 中字符与es:[edi]字符作比较，并且edi++，
	   "je 1f\n\t"		// 如果相等则向前跳转到标号1 处。
	   "movl $1,%0\n"	// 否则edi 中置1。
	   "1:\tdecl %0"	// 让edi 指向找到的字符（或是NULL）。
: "=D" (__res): "a" (c), "D" (csrc), "c" (count):"cx");
  return __res;			// 返回字符指针。
}*/

//// 用字符填写指定长度内存块。
// 用字符c 填写s 指向的内存区域，共填count 字节。
// %0 - eax(字符c)，%1 - edi(内存地址)，%2 - ecx(字节数count)。
extern _inline void *
memset (void *s, char c, int count)
{
	_asm {
		pushf
		mov edi,s
		mov ecx,count
		mov al,c
		cld		// 清方向位。
		rep stosb// 将al 中字符存入es:[edi]中，并且edi++。重复ecx 指定的次数，执行
		popf
	}
  return s;
}
/*extern _inline void *
memset (void *s, char c, int count)
{
  __asm__ ("cld\n\t"		// 清方向位。
	   "rep\n\t"		// 重复ecx 指定的次数，执行
	   "stosb"		// 将al 中字符存入es:[edi]中，并且edi++。
::"a" (c), "D" (s), "c" (count):"cx", "di");
  return s;
}*/

#endif
