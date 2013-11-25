;/* passed
; *  该文件包括页异常中断处理程序（中断14），主要分两种情况处理。
; * 一是由于缺页引起的页异常中断，通过调用do_no_page(error_code, address)来处理；
; * 二是由页写保护引起的页异常，此时调用页写保护处理函数do_wp_page(error_code, address)
; * 进行处理。其中的出错码(error_code)是由CPU 自动产生并压入堆栈的，出现异常时访问的
; * 线性地址是从控制寄存器CR2 中取得的。CR2 是专门用来存放页出错时的线性地址。
; */
.586p
.model flat
;/*
; * page.s 程序包含底层页异常处理代码。实际的工作在memory.c 中完成。
; */
extrn _do_no_page:proc,_do_wp_page:proc
public _page_fault

.code
_page_fault:
	xchg ss:[esp],eax	;// 取出错码到eax。
	push ecx
	push edx
	push ds
	push es
	push fs
	mov edx,10h		;// 置内核数据段选择符。
	mov ds,dx
	mov es,dx
	mov fs,dx
	mov edx,cr2			;// 取引起页面异常的线性地址
	push edx			;// 将该线性地址和出错码压入堆栈，作为调用函数的参数。
	push eax
	test eax,1			;// 测试标志P，如果不是缺页引起的异常则跳转。
	jne l1
	call _do_no_page	;// 调用缺页处理函数（mm/memory.c,365 行）。
	jmp l2			
l1:	call _do_wp_page	;// 调用写保护处理函数（mm/memory.c,247 行）。
l2:	add esp,8		;// 丢弃压入栈的两个参数。
	pop fs
	pop es
	pop ds
	pop edx
	pop ecx
	pop eax
	iretd
end
;/*
; * 当处理器在转换线性地址到物理地址的过程中检测到以下两种条件时，
; * 就会发生页异常中断，中断14。
; *   o 当CPU 发现对应页目录项或页表项的存在位（Present）标志为0。
; *   o 当前进程没有访问指定页面的权限。
; * 对于页异常处理中断，CPU 提供了两项信息用来诊断页异常和从中恢复运行。
; * (1) 放在堆栈上的出错码。该出错码指出了异常是由于页不存在引起的还是违反了访问权限引起的；
; * 		在发生异常时CPU 的当前特权层；以及是读操作还是写操作。出错码的格式是一个32 位的长
; * 		字。但只用了最后的3 个比特位。分别说明导致异常发生时的原因：
; * 		位2(U/S) - 0 表示在超级用户模式下执行，1 表示在用户模式下执行；
; * 		位1(W/R) - 0 表示读操作，1 表示写操作；
; * 		位0(P) - 0 表示页不存在，1 表示页级保护。
; * (2) CR2(控制寄存器2)。CPU 将造成异常的用于访问的线性地址存放在CR2 中。异常处理程序可以
; * 		使用这个地址来定位相应的页目录和页表项。如果在页异常处理程序执行期间允许发生另一
; * 		个页异常，那么处理程序应该将CR2 压入堆栈中。
; */
