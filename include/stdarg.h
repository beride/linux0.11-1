#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;		// 定义va_list 是一个字符指针类型。

/* Amount of space required in an argument list for an arg of type TYPE.
TYPE may alternatively be an expression whose type is used. */
/* 下面给出了类型为TYPE 的arg 参数列表所要求的空间容量。
TYPE 也可以是使用该类型的一个表达式 */

// 下面这句定义了取整后的TYPE 类型的字节长度值。是int 长度(4)的倍数。
#define __va_rounded_size(TYPE) \
(((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

// 下面这个函数（用宏实现）使AP 指向传给函数的可变参数表的第一个参数。
// 在第一次调用va_arg 或va_end 之前，必须首先调用该函数。
// 17 行上的__builtin_saveregs()是在gcc 的库程序libgcc2.c 中定义的，用于保存寄存器。
// 它的说明可参见gcc 手册章节“Target Description Macros”中的
// “Implementing the Varargs Macros”小节。
#ifndef __sparc__
#define va_start(AP, LASTARG) \
(AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#else
#define va_start(AP, LASTARG) \
(__builtin_saveregs (), \
AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#endif

// 下面该宏用于被调用函数完成一次正常返回。va_end 可以修改AP 使其在重新调用
// va_start 之前不能被使用。va_end 必须在va_arg 读完所有的参数后再被调用。
void va_end (va_list);		/* Defined in gnulib *//* 在gnulib 中定义 */
#define va_end(AP)

// 下面该宏用于扩展表达式使其与下一个被传递参数具有相同的类型和值。
// 对于缺省值，va_arg 可以用字符、无符号字符和浮点类型。
// 在第一次使用va_arg 时，它返回表中的第一个参数，后续的每次调用都将返回表中的
// 下一个参数。这是通过先访问AP，然后把它增加以指向下一项来实现的。
// va_arg 使用TYPE 来完成访问和定位下一项，每调用一次va_arg，它就修改AP 以指示
// 表中的下一参数。
#define va_arg(AP, TYPE) \
(AP += __va_rounded_size (TYPE), \
*((TYPE *) (AP - __va_rounded_size (TYPE))))

#endif /* _STDARG_H */
