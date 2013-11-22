/* passed
* linux/kernel/keyboard.S
*
* (C) 1991 Linus Torvalds
*/
.586p
.model flat
/*
* Thanks to Alfred Leung for US keyboard patches
* Wolfgang Thiel for German keyboard patches
* Marc Corsini for the French keyboard
*/
/*
* 感谢Alfred Leung 添加了US 键盘补丁程序；
* Wolfgang Thiel 添加了德语键盘补丁程序；
* Marc Corsini 添加了法文键盘补丁程序。
*/

#include <linux/config.h> // 内核配置头文件。定义键盘语言和硬盘类型（HD_TYPE）可选项。

extern _do_tty_interrupt:proc, _show_stat:proc
extern _table_list:dword
public _keyboard_interrupt //int 21h 在console.c内的conn_init函数内设置

/*
* these are for the keyboard read functions
*/
/*
* 以下这些是用于键盘读操作。
*/
// bsize 是键盘缓冲区的长度（字节数）。
bsize = 1024 /* must be a power of two ! And MUST be the same
			as in tty_io.c !!!! */
			/* 数值必须是2 的次方！并且与tty_io.c 中的值匹配!!!! */
			// 以下这些是缓冲队列结构中的偏移量 */
head = 4 // 缓冲区中头指针字段偏移。
tail = 8 // 缓冲区中尾指针字段偏移。
proc_list = 12 // 等待该缓冲队列的进程字段偏移。
buf = 16 // 缓冲区字段偏移。

.code

mode db 0 /* caps, alt, ctrl and shift mode */
// mode 是键盘特殊键的按下状态标志。
// 表示大小写转换键(caps)、交换键(alt)、控制键(ctrl)和换档键(shift)的状态。
// 位7 caps 键按下；
// 位6 caps 键的状态(应该与leds 中的对应标志位一样)；
// 位5 右alt 键按下；
// 位4 左alt 键按下；
// 位3 右ctrl 键按下；
// 位2 左ctrl 键按下；
// 位1 右shift 键按下；
// 位0 左shift 键按下。

leds db 2 /* num-lock, caps, scroll-lock mode (nom-lock on) */
// 数字锁定键(num-lock)、大小写转换键(caps-lock)和滚动锁定键(scroll-lock)的LED 发光管状态。
// 位7-3 全0 不用；
// 位2 caps-lock；
// 位1 num-lock(初始置1，也即设置数字锁定键(num-lock)发光管为亮)；
// 位0 scroll-lock。

e0 db 0
// 当扫描码是0xe0 或0xe1 时，置该标志。表示其后还跟随着1 个或2 个字符扫描码，参见列表后说明。
// 位1 =1 收到0xe1 标志；
// 位0 =1 收到0xe0 标志。
/*
* con_int is the real interrupt routine that reads the
* keyboard scan-code and converts it into the appropriate
* ascii character(s).
*/
/*
* con_int 是实际的中断处理子程序，用于读键盘扫描码并将其转换
* 成相应的ascii 字符。
*/
//// 键盘中断int 21h处理程序入口点。
_keyboard_interrupt:
	push eax
	push ebx
	push ecx
	push edx
	push ds
	push es
	mov eax,10h // 将ds、es 段寄存器置为内核数据段。
	mov ds,ax
	mov es,ax
	xor al,al /* %eax is scan code */ /* eax 中是扫描码 */
	in al,60h // 读取扫描码->al。
	cmp al,0e0h // 该扫描码是0xe0 吗？如果是则跳转到设置e0 标志代码处。
	je set_e0
	cmp al,0e1h // 扫描码是0xe1 吗？如果是则跳转到设置e1 标志代码处。
	je set_e1
	call key_table[eax*4] // 调用键处理程序ker_table + eax * 4（参见下面502 行）。
	mov e0,0 // 复位e0 标志。
// 下面这段代码(55-65 行)是针对使用8255A 的PC 标准键盘电路进行硬件复位处理。端口0x61 是
// 8255A 输出口B 的地址，该输出端口的第7 位(PB7)用于禁止和允许对键盘数据的处理。
// 这段程序用于对收到的扫描码做出应答。方法是首先禁止键盘，然后立刻重新允许键盘工作。
e0_e1: 
	in al,61h // 取PPI 端口B 状态，其位7 用于允许/禁止(0/1)键盘。
	jmp l1 // 延迟一会。
l1: jmp l2
l2: or al,80h // al 位7 置位(禁止键盘工作)。
	jmp l3 // 再延迟一会。
l3: jmp l4
l4: out 61h,al // 使PPI PB7 位置位。
	jmp l5 // 延迟一会。
l5: jmp l6
l6: and al,7Fh // al 位7 复位。
	out 61h,al // 使PPI PB7 位复位（允许键盘工作）。
	mov al,20h // 向8259 中断芯片发送EOI(中断结束)信号。
	out 20h,al
	push 0 // 控制台tty 号=0，作为参数入栈。
	call _do_tty_interrupt // 将收到的数据复制成规范模式数据并存放在规范字符缓冲队列中。
	add esp,4 // 丢弃入栈的参数，弹出保留的寄存器，并中断返回。
	pop es
	pop ds
	pop edx
	pop ecx
	pop ebx
	pop eax
	iretd
set_e0: 
	mov e0,1 // 收到扫描前导码0xe0 时，设置e0 标志（位0）。
	jmp e0_e1
set_e1: 
	mov e0,2 // 收到扫描前导码0xe1 时，设置e1 标志（位1）。
	jmp e0_e1

/*
* 下面该子程序把ebx:eax 中的最多8 个字符添入缓冲队列中。(edx 是
* 所写入字符的顺序是al,ah,eal,eah,bl,bh...直到eax 等于0。
*/
put_queue:
	push ecx // 保存ecx，edx 内容。
	push edx // 取控制台tty 结构中读缓冲队列指针。
	mov edx,_table_list // read-queue for console
	mov ecx,head[edx] // 取缓冲队列中头指针->ecx。
l7: mov buf[edx+ecx],al // 将al 中的字符放入缓冲队列头指针位置处。
	inc ecx // 头指针前移1 字节。
	and ecx,bsize-1 // 以缓冲区大小调整头指针(若超出则返回缓冲区开始)。
	cmp ecx,tail[edx] // buffer full - discard everything
// 头指针==尾指针吗(缓冲队列满)？
	je l9 // 如果已满，则后面未放入的字符全抛弃。
	shrd eax,ebx,8 // 将ebx 中8 位比特位右移8 位到eax 中，但ebx 不变。
	je l8 // 还有字符吗？若没有(等于0)则跳转。
	shr ebx,8 // 将ebx 中比特位右移8 位，并跳转到标号l7 继续操作。
	jmp l7
l8: mov head[edx],ecx // 若已将所有字符都放入了队列，则保存头指针。
	mov ecx,proc_list[edx] // 该队列的等待进程指针？
	test ecx,ecx // 检测任务结构指针是否为空(有等待该队列的进程吗？)。
	je l9 // 无，则跳转；
	mov dword ptr [ecx],0 // 有，则置该进程为可运行就绪状态(唤醒该进程)。
l9: pop edx // 弹出保留的寄存器并返回。
	pop ecx
	ret

// 下面这段代码根据ctrl 或alt 的扫描码，分别设置模式标志中相应位。如果该扫描码之前收到过
// 0xe0 扫描码(e0 标志置位)，则说明按下的是键盘右边的ctrl 或alt 键，则对应设置ctrl 或alt
// 在模式标志mode 中的比特位。
ctrl:
	mov al,04h // 0x4 是模式标志mode 中左ctrl 键对应的比特位(位2)。
	jmp l10
alt:
	mov al,10h // 0x10 是模式标志mode 中左alt 键对应的比特位(位4)。
l10:
	cmp e0,0 // e0 标志置位了吗(按下的是右边的ctrl 或alt 键吗)？
	je l11 // 不是则转。
	add al,al // 是，则改成置相应右键的标志位(位3 或位5)。
l11:
	or mode,al // 设置模式标志mode 中对应的比特位。
	ret
// 这段代码处理ctrl 或alt 键松开的扫描码，对应复位模式标志mode 中的比特位。在处理时要根据
// e0 标志是否置位来判断是否是键盘右边的ctrl 或alt 键。
unctrl:
	mov al,04h // 模式标志mode 中左ctrl 键对应的比特位(位2)。
	jmp l12
unalt:
	mov al,10h // 0x10 是模式标志mode 中左alt 键对应的比特位(位4)。
l12:
	cmp e0,0 // e0 标志置位了吗(释放的是右边的ctrl 或alt 键吗)？
	je l13 // 不是，则转。
	add al,al // 是，则该成复位相应右键的标志位(位3 或位5)。
l13:
	not al // 复位模式标志mode 中对应的比特位。
	and mode,al
	ret

lshift:
	or mode,01h // 是左shift 键按下，设置mode 中对应的标志位(位0)。
	ret
unlshift:
	and mode,0feh // 是左shift 键松开，复位mode 中对应的标志位(位0)。
	ret
rshift:
	or mode,02h // 是右shift 键按下，设置mode 中对应的标志位(位1)。
	ret
unrshift:
	and mode,0fdh // 是右shift 键松开，复位mode 中对应的标志位(位1)。
	ret

caps:
	test mode,80h // 测试模式标志mode 中位7 是否已经置位(按下状态)。
	jne l14 // 如果已处于按下状态，则返回(ret)。
	xor leds,4 // 翻转leds 标志中caps-lock 比特位(位2)。
	xor mode,40h // 翻转mode 标志中caps 键按下的比特位(位6)。
	or mode,80h // 设置mode 标志中caps 键已按下标志位(位7)。
// 这段代码根据leds 标志，开启或关闭LED 指示器。
set_leds:
	call kb_wait // 等待键盘控制器输入缓冲空。
	mov al,0edh /* set leds command */ /* 设置LED 的命令 */
	out 60h,al // 发送键盘命令0xed 到0x60 端口。
	call kb_wait // 等待键盘控制器输入缓冲空。
	mov al,leds // 取leds 标志，作为参数。
	out 60h,al // 发送该参数。
	ret
uncaps:
	and mode,7fh // caps 键松开，则复位模式标志mode 中的对应位(位7)。
	ret
scroll:
	xor leds,1 // scroll 键按下，则翻转leds 标志中的对应位(位0)。
	jmp set_leds // 根据leds 标志重新开启或关闭LED 指示器。
num:
	xor leds,2 // num 键按下，则翻转leds 标志中的对应位(位1)。
	jmp set_leds // 根据leds 标志重新开启或关闭LED 指示器。

/*
* curosr-key/numeric keypad cursor keys are handled here.
* checking for numeric keypad etc.
*/
/*
* 这里处理方向键/数字小键盘方向键，检测数字小键盘等。
*/
cursor:
	sub al,47h // 扫描码是小数字键盘上的键(其扫描码>=0x47)发出的？
	jb l14 // 如果小于则不处理，返回。
	cmp al,12 // 如果扫描码 > 0x53(0x53 - 0x47= 12)，则
	ja l14 // 扫描码值超过83(0x53)，不处理，返回。
	jne cur2 /* check for ctrl-alt-del */ /* 检查是否ctrl-alt-del */
// 如果等于12，则说明del 键已被按下，则继续判断ctrl
// 和alt 是否也同时按下。
	test mode,0ch // 有ctrl 键按下吗？
	je cur2 // 无，则跳转。
	test mode,30h // 有alt 键按下吗？
	jne reboot // 有，则跳转到重启动处理。
cur2:
	cmp e0,01h /* e0 forces cursor movement */ /* e0 置位表示光标移动 */
// e0 标志置位了吗？
	je cur // 置位了，则跳转光标移动处理处cur。
	test leds,02h /* not num-lock forces cursor */ /* num-lock 键则不许 */
// 测试leds 中标志num-lock 键标志是否置位。
	je cur // 如果没有置位(num 的LED 不亮)，则也进行光标移动处理。
	test mode,03h /* shift forces cursor */ /* shift 键也使光标移动 */
// 测试模式标志mode 中shift 按下标志。
	jne cur // 如果有shift 键按下，则也进行光标移动处理。
	xor ebx,ebx // 否则查询扫数字表(199 行)，取对应键的数字ASCII 码。
	mov al,num_table[eax] // 以eax 作为索引值，取对应数字字符->al。
	jmp put_queue // 将该字符放入缓冲队列中。
l14:
	ret

// 这段代码处理光标的移动。
cur:
	mov al,cur_table[eax] // 取光标字符表中相应键的代表字符??al。
	cmp al,'9' // 若该字符<='9'，说明是上一页、下一页、插入或删除键，
	ja ok_cur // 则功能字符序列中要添入字符'~'。
	mov ah,'~'
ok_cur:
	shl eax,16 // 将ax 中内容移到eax 高字中。
	mov ax,5b1bh // 在ax 中放入'esc ['字符，与eax 高字中字符组成移动序列。
	xor ebx,ebx
	jmp put_queue // 将该字符放入缓冲队列中。

#if defined(KBD_FR)
num_table db "789 456 1230." // 数字小键盘上键对应的数字ASCII 码表。
#else
num_table db "789 456 1230,"
#endif
cur_table db  "HA5 DGC YB623" // 数字小键盘上方向键或插入删除键对应的移动表示字符表。

/*
* this routine handles function keys
*/
// 下面子程序处理功能键。
func:
	push eax
	push ecx
	push edx
	call _show_stat // 调用显示各任务状态函数(kernl/sched.c, 37)。
	pop edx
	pop ecx
	pop eax
	sub al,3Bh // 功能键'F1'的扫描码是0x3B，因此此时al 中是功能键索引号。
	jb end_func // 如果扫描码小于0x3b，则不处理，返回。
	cmp al,9 // 功能键是F1-F10？
	jbe ok_func // 是，则跳转。
	sub al,18 // 是功能键F11，F12 吗？
	cmp al,10 // 是功能键F11？
	jb end_func // 不是，则不处理，返回。
	cmp al,11 // 是功能键F12？
	ja end_func // 不是，则不处理，返回。
ok_func:
	cmp ecx,4 /* check that there is enough room */ /* 检查是否有足够空间*/
	jl end_func // 需要放入4 个字符序列，如果放不下，则返回。
	mov eax,func_table[eax*4] // 取功能键对应字符序列。
	xor ebx,ebx
	jmp put_queue // 放入缓冲队列中。
end_func:
	ret

/*
* 功能键发送的扫描码，F1 键为：'esc [ [ A'， F2 键为：'esc [ [ B'等。
*/
func_table:
 DD 415b5b1bh,425b5b1bh,435b5b1bh,445b5b1bh
 DD 455b5b1bh,465b5b1bh,475b5b1bh,485b5b1bh
 DD 495b5b1bh,4a5b5b1bh,4b5b5b1bh,4c5b5b1bh

// ps:只有KBD_US检测通过
// 扫描码-ASCII 字符映射表。
// 根据在config.h 中定义的键盘类型(FINNISH，US，GERMEN，FRANCH)，将相应键的扫描码映射
// 到ASCII 字符。
#if defined(KBD_FINNISH)
// 以下是芬兰语键盘的扫描码映射表。
key_map:
 db 0,27 // 扫描码0x00,0x01 对应的ASCII 码；
 db "1234567890+'" // 扫描码0x02,...0x0c,0x0d 对应的ASCII 码，以下类似。
 db 127,9
 db "qwertyuiop}"
 db 0,13,0
 db "asdfghjkl|{"
 db 0,0
 db "'zxcvbnm,.-"
 db 0,'*',0,32 /* 36-39 */ /* 扫描码0x36-0x39 对应的ASCII 码 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */ /* 扫描码0x4A-0x4E 对应的ASCII 码 */
 db 0,0,0,0,0,0,0 /* 4F-55 */ /* 扫描码0x4F-0x55 对应的ASCII 码 */
 db '<'
 db 10 dup(0)

// shift 键同时按下时的映射表。
shift_map:
 db 0,27
 db '!','"',"#$%&/()=?`"
 db 127,9
 db "QWERTYUIOP]^"
 db 13,0
 db "ASDFGHJKL\["
 db 0,0
 db "*ZXCVBNM:_"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '>'
 db 10 dup(0)

// alt 键同时按下时的映射表。
alt_map:
 db 0,0
 db 0,'@',0,'$',0,0,"{[]}",'\',0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db '~',13,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0,0,0 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db 0,0,0,0,0 /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '|'
 db 10 dup(0)

#elif defined(KBD_US)

// 以下是美式键盘的扫描码映射表。
key_map:
 db 0,27
 db "1234567890-="
 db 127,9
 db "qwertyuiop[]"
 db 13,0
 db "asdfghjkl;'"
 db '`',0
 db "\zxcvbnm,./"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '<'
 db 10 dup(0)


shift_map:
 db 0,27
 db "!@#$%^&*()_+"
 db 127,9
 db "QWERTYUIOP{}"
 db 13,0
 db "ASDFGHJKL:",'"'
 db '~',0
 db "|ZXCVBNM<>?"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '>'
 db 10 dup(0)

alt_map:
 db 0,0
 db 0,'@',0,'$',0,0,"{[]}","\",0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db '~',13,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0,0,0 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db 0,0,0,0,0 /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '|'
 db 10 dup(0)

#elif defined(KBD_GR)

// 以下是德语键盘的扫描码映射表。
key_map:
 db 0,27
 db "1234567890\'"
 db 127,9
 db "qwertzuiop@+"
 db 13,0
 db "asdfghjkl[]^"
 db 0,'#'
 db "yxcvbnm,.-"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '<'
 db 10 dup(0)


shift_map:
 db 0,27
 db '!','"',"#$%&/()=?`"
 db 127,9
 db "QWERTZUIOP\*"
 db 13,0
 db "ASDFGHJKL{}~"
 db 0,''
 db "YXCVBNM:_"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '>'
 db 10 dup(0)

alt_map:
 db 0,0
 db 0,'@',0,'$',0,0,"{[]}",'\',0
 db 0,0
 db '@',0,0,0,0,0,0,0,0,0,0
 db '~',13,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0,0,0 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db 0,0,0,0,0 /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '|'
 db 10 dup(0)


#elif defined(KBD_FR)

// 以下是法语键盘的扫描码映射表。
key_map:
 db 0,27
 db "&{",'"',"'(-}_/@)="
 db 127,9
 db "azertyuiop^$"
 db 13,0
 db "qsdfghjklm|"
 db '`',0,42 /* coin sup gauche, don't know, [*|mu] */
 db "wxcvbn,:!"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '<'
 db 10 dup(0)

shift_map:
 db 0,27
 db "1234567890]+"
 db 127,9
 db "AZERTYUIOP<>"
 db 13,0
 db "QSDFGHJKLM%"
 db '~',0,'#'
 db "WXCVBN?./\"
 db 0,'*',0,32 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db '-',0,0,0,'+' /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '>'
 db 10 dup(0)

alt_map:
 db 0,0
 db 0,"~#{[|`\^@]}"
 db 0,0
 db '@',0,0,0,0,0,0,0,0,0,0
 db '~',13,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0,0,0 /* 36-39 */
 db 16 dup(0) /* 3A-49 */ /* 扫描码0x3A-0x49 对应的ASCII 码 */
 db 0,0,0,0,0 /* 4A-4E */
 db 0,0,0,0,0,0,0 /* 4F-55 */
 db '|'
 db 10 dup(0)

#else
#error "KBD-type not defined"
#endif
/*
* do_self handles "normal" keys, ie keys that don't change meaning
* and which have just one character returns.
*/
/*
* do_self 用于处理“普通”键，也即含义没有变化并且只有一个字符返回的键。
*/
do_self:
// 454-460 行用于根据模式标志mode 选择alt_map、shift_map 或key_map 映射表之一。
	lea ebx,alt_map // alt 键同时按下时的映射表基址alt_map??ebx。
	test mode,20 /* alt-gr */ /* 右alt 键同时按下了? */
	jne l15 // 是，则向前跳转到标号1 处。
	lea ebx,shift_map // shift 键同时按下时的映射表基址shift_map??ebx。
	test mode,03h // 有shift 键同时按下了吗？
	jne l15 // 有，则向前跳转到标号1 处。
	lea ebx,key_map // 否则使用普通映射表key_map。
// 取映射表中对应扫描码的ASCII 字符，若没有对应字符，则返回(转none)。
l15:
	mov al,[ebx+eax] // 将扫描码作为索引值，取对应的ASCII 码??al。
	or al,al // 检测看是否有对应的ASCII 码。
	je none // 若没有(对应的ASCII 码=0)，则返回。
// 若ctrl 键已按下或caps 键锁定，并且字符在'a'-'}'(0x61-0x7D)范围内，则将其转成大写字符
// (0x41-0x5D)。
	test mode,4ch /* ctrl or caps */ /* 控制键已按下或caps 亮？*/
	je l16 // 没有，则向前跳转标号2 处。
	cmp al,'a' // 将al 中的字符与'a'比较。
	jb l16 // 若al 值<'a'，则转标号2 处。
	cmp al,'}' // 将al 中的字符与'}'比较。
	ja l16 // 若al 值>'}'，则转标号2 处。
	sub al,32 // 将al 转换为大写字符(减0x20)。
// 若ctrl 键已按下，并且字符在'`'--'_'(0x40-0x5F)之间(是大写字符)，则将其转换为控制字符
// (0x00-0x1F)。
l16: 
	test mode,0ch /* ctrl */ /* ctrl 键同时按下了吗？*/
	je l17 // 若没有则转标号3。
	cmp al,64 // 将al 与'@'(64)字符比较(即判断字符所属范围)。
	jb l17 // 若值<'@'，则转标号3。
	cmp al,64+32 // 将al 与'`'(96)字符比较(即判断字符所属范围)。
	jae l17 // 若值>='`'，则转标号3。
	sub al,64 // 否则al 值减0x40，
// 即将字符转换为0x00-0x1f 之间的控制字符。
// 若左alt 键同时按下，则将字符的位7 置位。
l17: 
	test mode,10h /* left alt */ /* 左alt 键同时按下？*/
	je l18 // 没有，则转标号4。
	or al,80h // 字符的位7 置位。
// 将al 中的字符放入读缓冲队列中。
l18: 
	and eax,0ffh // 清eax 的高字和ah。
	xor ebx,ebx // 清ebx。
	call put_queue // 将字符放入缓冲队列中。
none: 
	ret

/*
* minus has a routine of it's own, as a 'E0h' before
* the scan code for minus means that the numeric keypad
* slash was pushed.
*/
/*
* 减号有它自己的处理子程序，因为在减号扫描码之前的0xe0
* 意味着按下了数字小键盘上的斜杠键。
*/
minus: 
	cmp e0,1 // e0 标志置位了吗？
	jne do_self // 没有，则调用do_self 对减号符进行普通处理。
	mov eax,'/' // 否则用'/'替换减号'-'??al。
	xor ebx,ebx
	jmp put_queue // 并将字符放入缓冲队列中。

/*
* This table decides which routine to call when a scan-code has been
* gotten. Most routines just call do_self, or none, depending if
* they are make or break.
*/
/* 下面是一张子程序地址跳转表。当取得扫描码后就根据此表调用相应的扫描码处理子程序。
* 大多数调用的子程序是do_self，或者是none，这起决于是按键(make)还是释放键(break)。
*/
key_table:
 DD none,   do_self,do_self,do_self /* 00-03 s0 esc 1 2 */
 DD do_self,do_self,do_self,do_self /* 04-07 3 4 5 6 */
 DD do_self,do_self,do_self,do_self /* 08-0B 7 8 9 0 */
 DD do_self,do_self,do_self,do_self /* 0C-0F + ' bs tab */
 DD do_self,do_self,do_self,do_self /* 10-13 q w e r */
 DD do_self,do_self,do_self,do_self /* 14-17 t y u i */
 DD do_self,do_self,do_self,do_self /* 18-1B o p } ^ */
 DD do_self,ctrl,   do_self,do_self /* 1C-1F enter ctrl a s */
 DD do_self,do_self,do_self,do_self /* 20-23 d f g h */
 DD do_self,do_self,do_self,do_self /* 24-27 j k l | */
 DD do_self,do_self,lshift, do_self /* 28-2B { para lshift , */
 DD do_self,do_self,do_self,do_self /* 2C-2F z x c v */
 DD do_self,do_self,do_self,do_self /* 30-33 b n m , */
 DD do_self,minus,  rshift, do_self /* 34-37 . - rshift * */
 DD alt,    do_self,caps,   func /* 38-3B alt sp caps f1 */
 DD func,   func,   func,   func /* 3C-3F f2 f3 f4 f5 */
 DD func,   func,   func,   func /* 40-43 f6 f7 f8 f9 */
 DD func,   num,    scroll, cursor /* 44-47 f10 num scr home */
 DD cursor, cursor, do_self,cursor /* 48-4B up pgup - left */
 DD cursor, cursor, do_self,cursor /* 4C-4F n5 right + end */
 DD cursor, cursor, cursor, cursor /* 50-53 dn pgdn ins del */
 DD none,   none,   do_self,func /* 54-57 sysreq ? < f11 */
 DD func,   none,   none,   none /* 58-5B f12 ? ? ? */
 DD none,   none,   none,   none /* 5C-5F ? ? ? ? */
 DD none,   none,   none,   none /* 60-63 ? ? ? ? */
 DD none,   none,   none,   none /* 64-67 ? ? ? ? */
 DD none,   none,   none,   none /* 68-6B ? ? ? ? */
 DD none,   none,   none,   none /* 6C-6F ? ? ? ? */
 DD none,   none,   none,   none /* 70-73 ? ? ? ? */
 DD none,   none,   none,   none /* 74-77 ? ? ? ? */
 DD none,   none,   none,   none /* 78-7B ? ? ? ? */
 DD none,   none,   none,   none /* 7C-7F ? ? ? ? */
 DD none,   none,   none,   none /* 80-83 ? br br br */
 DD none,   none,   none,   none /* 84-87 br br br br */
 DD none,   none,   none,   none /* 88-8B br br br br */
 DD none,   none,   none,   none /* 8C-8F br br br br */
 DD none,   none,   none,   none /* 90-93 br br br br */
 DD none,   none,   none,   none /* 94-97 br br br br */
 DD none,   none,   none,   none /* 98-9B br br br br */
 DD none,   unctrl, none,   none /* 9C-9F br unctrl br br */
 DD none,   none,   none,   none /* A0-A3 br br br br */
 DD none,   none,   none,   none /* A4-A7 br br br br */
 DD none,   none,   unlshift,none /* A8-AB br br unlshift br */
 DD none,   none,   none,   none /* AC-AF br br br br */
 DD none,   none,   none,   none /* B0-B3 br br br br */
 DD none,   none,   unrshift,none /* B4-B7 br br unrshift br */
 DD unalt,  none,   uncaps, none /* B8-BB unalt br uncaps br */
 DD none,none,none,none /* BC-BF br br br br */
 DD none,none,none,none /* C0-C3 br br br br */
 DD none,none,none,none /* C4-C7 br br br br */
 DD none,none,none,none /* C8-CB br br br br */
 DD none,none,none,none /* CC-CF br br br br */
 DD none,none,none,none /* D0-D3 br br br br */
 DD none,none,none,none /* D4-D7 br br br br */
 DD none,none,none,none /* D8-DB br ? ? ? */
 DD none,none,none,none /* DC-DF ? ? ? ? */
 DD none,none,none,none /* E0-E3 e0 e1 ? ? */
 DD none,none,none,none /* E4-E7 ? ? ? ? */
 DD none,none,none,none /* E8-EB ? ? ? ? */
 DD none,none,none,none /* EC-EF ? ? ? ? */
 DD none,none,none,none /* F0-F3 ? ? ? ? */
 DD none,none,none,none /* F4-F7 ? ? ? ? */
 DD none,none,none,none /* F8-FB ? ? ? ? */
 DD none,none,none,none /* FC-FF ? ? ? ? */

/*
* kb_wait waits for the keyboard controller buffer to empty.
* there is no timeout - if the buffer doesn't empty, we hang.
*/
/*
* 子程序kb_wait 用于等待键盘控制器缓冲空。不存在超时处理 - 如果
* 缓冲永远不空的话，程序就会永远等待(死掉)。
*/
kb_wait:
	push eax
l19: 
	in al,64h // 读键盘控制器状态。
	test al,02h // 测试输入缓冲器是否为空(等于0)。
	jne l19 // 若不空，则跳转循环等待。
	pop eax
	ret
/*
* This routine reboots the machine by asking the keyboard
* controller to pulse the reset-line low.
*/
/*
* 该子程序通过设置键盘控制器，向复位线输出负脉冲，使系统复位重启(reboot)。
*/
reboot:
	call kb_wait // 首先等待键盘控制器输入缓冲器空。
	mov word ptr ds:[472h],1234h /* don't do memory check */
	mov al,0fch /* pulse reset and A20 low */
	out 64h,al // 向系统复位和A20 线输出负脉冲。
die: 
	jmp die // 死机。

end