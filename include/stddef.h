#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;		// 两个指针相减结果的类型。
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;	// sizeof 返回的类型。
#endif

#undef NULL
#define NULL 0	// 空指针。

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)	// 成员在类型中的偏移位置。

#endif
