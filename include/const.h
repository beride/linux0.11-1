#ifndef _CONST_H
#define _CONST_H

#define BUFFER_END 0x200000	// 定义缓冲使用内存的末端(代码中没有使用该常量)。

// i 节点数据结构中i_mode 字段的各标志位。
#define I_TYPE 0170000		// 指明i 节点类型。
#define I_DIRECTORY 0040000	// 是目录文件。
#define I_REGULAR 0100000	// 常规文件，不是目录文件或特殊文件。
#define I_BLOCK_SPECIAL 0060000	// 块设备特殊文件。
#define I_CHAR_SPECIAL 0020000	// 字符设备特殊文件。
#define I_NAMED_PIPE 0010000	// 命名管道。
#define I_SET_UID_BIT 0004000	// 在执行时设置有效用户id 类型。
#define I_SET_GID_BIT 0002000	// 在执行时设置有效组id 类型。

#endif
