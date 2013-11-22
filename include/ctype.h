#ifndef _CTYPE_H
#define _CTYPE_H

#define _U 0x01 /* upper */	// 该比特位用于大写字符[A-Z]。
#define _L 0x02 /* lower */	// 该比特位用于小写字符[a-z]。
#define _D 0x04 /* digit */	// 该比特位用于数字[0-9]。
#define _C 0x08 /* cntrl */	// 该比特位用于控制字符。
#define _P 0x10 /* punct */	// 该比特位用于标点字符。
#define _S 0x20 /* white space (space/lf/tab) */	// 用于空白字符，如空格、\t、\n 等。
#define _X 0x40 /* hex digit */	// 该比特位用于十六进制数字。
#define _SP 0x80 /* hard space (0x20) */	// 该比特位用于空格字符(0x20)。

extern unsigned char _ctype[];	// 字符特性数组(表)，定义了各个字符对应上面的属性。
extern char _ctmp;		// 一个临时字符变量(在fs/ctype.c 中定义)。

// 下面是一些确定字符类型的宏。
#define isalnum(c) ((_ctype+1)[c]&(_U|_L|_D))	// 是字符或数字[A-Z]、[a-z]或[0-9]。
#define isalpha(c) ((_ctype+1)[c]&(_U|_L))	// 是字符。
#define iscntrl(c) ((_ctype+1)[c]&(_C))	// 是控制字符。
#define isdigit(c) ((_ctype+1)[c]&(_D))	// 是数字。
#define isgraph(c) ((_ctype+1)[c]&(_P|_U|_L|_D))	// 是图形字符。
#define islower(c) ((_ctype+1)[c]&(_L))	// 是小写字符。
#define isprint(c) ((_ctype+1)[c]&(_P|_U|_L|_D|_SP))	// 是可打印字符。
#define ispunct(c) ((_ctype+1)[c]&(_P))	// 是标点符号。
#define isspace(c) ((_ctype+1)[c]&(_S))	// 是空白字符如空格,\f,\n,\r,\t,\v。
#define isupper(c) ((_ctype+1)[c]&(_U))	// 是大写字符。
#define isxdigit(c) ((_ctype+1)[c]&(_D|_X))	// 是十六进制数字。

#define isascii(c) (((unsigned) c)<=0x7f)	// 是ASCII 字符。
#define toascii(c) (((unsigned) c)&0x7f)	// 转换成ASCII 字符。

#define tolower(c) (_ctmp=c,isupper(_ctmp)?_ctmp-( 'A'- 'a'):_ctmp)	// 转换成对应小写字符。
#define toupper(c) (_ctmp=c,islower(_ctmp)?_ctmp-( 'a'- 'A'):_ctmp)	// 转换成对应大写字符。

#endif
