;/* passed
;* linux/kernel/rs_io.s
;*
;* (C) 1991 Linus Torvalds
;*/
.386p
.model flat
;/*
;* rs_io.s
;*
;* This module implements the rs232 io interrupts.
;*/
;/*
;* 该程序模块实现rs232 输入输出中断处理程序。
;*/

;// size 是读写队列缓冲区的字节长度。
bsize = 1024 ;/* must be power of two ! 必须是2 的次方并且需
			 ; and must match the value 与tty_io.c 中的值匹配!
			 ; in tty_io.c!!! */

;/* these are the offsets into the read/write buffer structures */
;/* 以下这些是读写缓冲结构中的偏移量 */
;// 对应定义在include/linux/tty.h 文件中tty_queue 结构中各变量的偏移量。
rs_addr = 0 ;// 串行端口号字段偏移（端口号是0x3f8 或0x2f8）。
head = 4 ;// 缓冲区中头指针字段偏移。
tail = 8 ;// 缓冲区中尾指针字段偏移。
proc_list = 12 ;// 等待该缓冲的进程字段偏移。
buf = 16 ;// 缓冲区字段偏移。

startup = 256 ;/* chars left in write queue when we restart it */
;/* 当写队列里还剩256 个字符空间(WAKEUP_CHARS)时，我们就可以写 */

;/*
;* These are the actual interrupt routines. They look where
;* the interrupt is coming from, and take appropriate action.
;*/
;/*
;* 这些是实际的中断程序。程序首先检查中断的来源，然后执行相应
;* 的处理。
;*/
extrn _table_list:dword, _do_tty_interrupt:proc
public _rs1_interrupt,_rs2_interrupt

.code
align 4
;// 串行端口1 中断处理程序入口点。
_rs1_interrupt:
	push _table_list+8 ;// tty 表中对应串口1 的读写缓冲指针的地址入栈(tty_io.c，99)。
	jmp rs_int
align 4
;// 串行端口2 中断处理程序入口点。
_rs2_interrupt:
	push _table_list+16 ;// tty 表中对应串口2 的读写缓冲队列指针的地址入栈。
rs_int:
	push edx
	push ecx
	push ebx
	push eax
	push es
	push ds ;/* as this is an interrupt, we cannot */
	push 10h ;/* know that bs is ok. Load it */
	pop ds ;/* 由于这是一个中断程序，我们不知道ds 是否正确，*/
	push 10h ;/* 所以加载它们(让ds、es 指向内核数据段 */
	pop es
	mov edx,24[esp] ;// 将缓冲队列指针地址存入edx 寄存器，
;// 也即35 或39 行上最先压入堆栈的地址。
	mov edx,[edx] ;// 取读队列指针(地址)->edx。
	mov edx,rs_addr[edx] ;// 取串口1 的端口号??edx。
	add edx,2 ;/* interrupt ident. reg */ /* edx 指向中断标识寄存器 */
rep_int: ;// 中断标识寄存器端口是0x3fa（0x2fa），参见上节列表后信息。
	xor eax,eax ;// eax 清零。
	in al,dx ;// 取中断标识字节，用以判断中断来源(有4 种中断情况)。
	test al,1 ;// 首先判断有无待处理的中断(位0=1 无中断；=0 有中断)。
	jne end1 ;// 若无待处理中断，则跳转至退出处理处end。
	cmp al,6 ;/* this shouldn't happen, but ... */ /* 这不会发生，但是…*/
	ja end1 ;// al 值>6? 是则跳转至end（没有这种状态）。
	mov ecx,24[esp] ;// 再取缓冲队列指针地址??ecx。
	push edx ;// 将端口号0x3fa(0x2fa)入栈。
	sub edx,2 ;// 0x3f8(0x2f8)。
	call jmp_table[eax*2] ;/* NOTE! not ;*4, bit0 is 0 already */ /* 不乘4，位0 已是0*/
;// 上面语句是指，当有待处理中断时，al 中位0=0，位2-1 是中断类型，因此相当于已经将中断类型
;// 乘了2，这里再乘2，得到跳转表对应各中断类型地址，并跳转到那里去作相应处理。
	pop edx ;// 弹出中断标识寄存器端口号0x3fa（或0x2fa）。
	jmp rep_int ;// 跳转，继续判断有无待处理中断并继续处理。
end1:
	mov al,20h ;// 向中断控制器发送结束中断指令EOI。
	out 20h,al ;/* EOI */
	pop ds
	pop es
	pop eax
	pop ebx
	pop ecx
	pop edx
	add esp,4 ;// jump over _table_list entry # 丢弃缓冲队列指针地址。
	iretd

;// 各中断类型处理程序地址跳转表，共有4 种中断来源：
;// modem 状态变化中断，写字符中断，读字符中断，线路状态有问题中断。
jmp_table:
	dd modem_status,write_char,read_char,line_status

align 4
modem_status:
	add edx,6 ;/* clear intr by reading modem status reg */
	in al,dx ;/* 通过读modem 状态寄存器进行复位(0x3fe) */
	ret

align 4
line_status:
	add edx,5 ;/* clear intr by reading line status reg. */
	in al,dx ;/* 通过读线路状态寄存器进行复位(0x3fd) */
	ret

align 4
read_char:
	in al,dx ;// 读取字符->al。
	mov edx,ecx ;// 当前串口缓冲队列指针地址??edx。
	sub edx,_table_list ;// 缓冲队列指针表首址 - 当前串口队列指针地址??edx，
	shr edx,3 ;// 差值/8。对于串口1 是1，对于串口2 是2。
	mov ecx,[ecx] ;// read-queue # 取读缓冲队列结构地址??ecx。
	mov ebx,head[ecx] ;// 取读队列中缓冲头指针??ebx。
	mov buf[ebx+ecx],al ;// 将字符放在缓冲区中头指针所指的位置。
	inc ebx ;// 将头指针前移一字节。
	and ebx,bsize-1 ;// 用缓冲区大小对头指针进行模操作。指针不能超过缓冲区大小。
	cmp ebx,tail[ecx] ;// 缓冲区头指针与尾指针比较。
	je l1 ;// 若相等，表示缓冲区满，跳转到标号1 处。
	mov head[ecx],ebx ;// 保存修改过的头指针。
l1: push edx ;// 将串口号压入堆栈(1- 串口1，2 - 串口2)，作为参数，
	call _do_tty_interrupt ;// 调用tty 中断处理C 函数（。
	add esp,4 ;// 丢弃入栈参数，并返回。
	ret

align 4
write_char:
	mov ecx,[4+ecx] ;// write-queue # 取写缓冲队列结构地址??ecx。
	mov ebx,head[ecx] ;// 取写队列头指针??ebx。
	sub ebx,tail[ecx] ;// 头指针 - 尾指针 = 队列中字符数。
	and ebx,bsize-1 ;// nr chars in queue # 对指针取模运算。
	je write_buffer_empty ;// 如果头指针 = 尾指针，说明写队列无字符，跳转处理。
	cmp ebx,startup ;// 队列中字符数超过256 个？
	ja l2 ;// 超过，则跳转处理。
	mov ebx,proc_list[ecx] ;// wake up sleeping process # 唤醒等待的进程。
;// 取等待该队列的进程的指针，并判断是否为空。
	test ebx,ebx ;// is there any? # 有等待的进程吗？
	je l2 ;// 是空的，则向前跳转到标号1 处。
	mov dword ptr [ebx],0 ;// 否则将进程置为可运行状态(唤醒进程)。。
l2: mov ebx,tail[ecx] ;// 取尾指针。
	mov al,buf[ebx+ecx] ;// 从缓冲中尾指针处取一字符??al。
	out dx,al ;// 向端口0x3f8(0x2f8)送出到保持寄存器中。
	inc ebx ;// 尾指针前移。
	and ebx,bsize-1 ;// 尾指针若到缓冲区末端，则折回。
	mov tail[ecx],ebx ;// 保存已修改过的尾指针。
	cmp ebx,head[ecx] ;// 尾指针与头指针比较，
	je write_buffer_empty ;// 若相等，表示队列已空，则跳转。
	ret
align 4
write_buffer_empty:
	mov ebx,proc_list[ecx] ;// wake up sleeping process # 唤醒等待的进程。
;// 取等待该队列的进程的指针，并判断是否为空。
	test ebx,ebx ;// is there any? # 有等待的进程吗？
	je l3 ;// 无，则向前跳转到标号1 处。
	mov dword ptr [ebx],0 ;// 否则将进程置为可运行状态(唤醒进程)。
l3: inc edx ;// 指向端口0x3f9(0x2f9)。
	in al,dx ;// 读取中断允许寄存器。
	jmp l4 ;// 稍作延迟。
l4: jmp l5
l5: and al,0dh ;/* disable transmit interrupt */
;/* 屏蔽发送保持寄存器空中断（位1） */
	out dx,al ;// 写入0x3f9(0x2f9)。
	ret
end