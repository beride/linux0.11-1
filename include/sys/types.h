#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	// 用于对象的大小（长度）。
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long time_t;		// 用于时间（以秒计）。
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;
#endif

#ifndef NULL
#define NULL 0
#endif

typedef int pid_t;		// 用于进程号和进程组号。
typedef unsigned short uid_t;	// 用于用户号（用户标识号）。
typedef unsigned char gid_t;	// 用于组号。
typedef unsigned short dev_t;	// 用于设备号。
typedef unsigned short ino_t;	// 用于文件序列号。
typedef unsigned short mode_t;	// 用于某些文件属性。
typedef unsigned short umode_t;	//
typedef unsigned char nlink_t;	// 用于连接计数。
typedef int daddr_t;
typedef long off_t;		// 用于文件长度（大小）。
typedef unsigned char u_char;	// 无符号字符类型。
typedef unsigned short ushort;	// 无符号短整数类型。

typedef struct
{
  int quot, rem;
}
div_t;				// 用于DIV 操作。
typedef struct
{
  long quot, rem;
}
ldiv_t;				// 用于长DIV 操作。

struct ustat
{
  daddr_t f_tfree;
  ino_t f_tinode;
  char f_fname[6];
  char f_fpack[6];
};

#endif
