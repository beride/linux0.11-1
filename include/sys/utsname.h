#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#include <sys/types.h>		// 类型头文件。定义了基本的系统数据类型。

struct utsname
{
  char sysname[9];		// 本版本操作系统的名称。
  char nodename[9];		// 与实现相关的网络中节点名称。
  char release[9];		// 本实现的当前发行级别。
  char version[9];		// 本次发行的版本级别。
  char machine[9];		// 系统运行的硬件类型名称。
};

extern int uname (struct utsname *utsbuf);

#endif
