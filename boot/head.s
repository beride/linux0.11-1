;/* passed
; *  linux/boot/head.s
; *
; *  (C) 1991  Linus Torvalds
; */
.586p
.model flat
;/*
; *   head.s 含有32 位启动代码。
; *
; * 注意!!! 32 位启动代码是从绝对地址0x00000000 开始的，这里也同样
; * 是页目录将存在的地方，因此这里的启动代码将被页目录覆盖掉。
; * 
; */
extrn _stack_start:far ptr,_main_rename:proc,_printk:proc
public _idt,_gdt,_pg_dir,_tmp_floppy_area
.code
_pg_dir:		;// 页目录将会存放在这里。
_startup_32:			;// 以下5行设置各个数据段寄存器。指向gdt数据段描述符项
	mov eax,10h
;// 再次注意!!! 这里已经处于32 位运行模式，因此这里的$0x10 并不是把地址0x10 装入各
;// 个段寄存器，它现在其实是全局段描述符表中的偏移值，或者更正确地说是一个描述符表
;// 项的选择符。有关选择符的说明请参见setup.s 中的说明。这里$0x10 的含义是请求特权
;// 级0(位0-1=0)、选择全局描述符表(位2=0)、选择表中第2 项(位3-15=2)。它正好指向表中
;// 的数据段描述符项。（描述符的具体数值参见前面setup.s ）。下面代码的含义是：
;// 置ds,es,fs,gs 中的选择符为setup.s 中构造的数据段（全局段描述符表的第2 项）=0x10，
;// 并将堆栈放置在数据段中的_stack_start 数组内，然后使用新的中断描述符表和全局段
;// 描述表.新的全局段描述表中初始内容与setup.s 中的完全一样。
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	lss esp,_stack_start	;// 表示_stack_start -> ss:esp，设置系统堆栈。
							;// stack_start 定义在kernel/sched.c，69 行。
	call setup_idt		;// 调用设置中断描述符表子程序。
	call setup_gdt		;// 调用设置全局描述符表子程序。
	mov eax,10h			;// reload all the segment registers
	mov ds,ax			;// after changing gdt. CS was already
	mov es,ax			;// reloaded in 'setup_gdt'
	mov fs,ax			;// 因为修改了gdt，所以需要重新装载所有的段寄存器。
	mov gs,ax			;// CS 代码段寄存器已经在setup_gdt 中重新加载过了。
	lss esp,_stack_start
;// 以下5行用于测试A20 地址线是否已经开启。采用的方法是向内存地址0x000000 处写入任意
;// 一个数值，然后看内存地址0x100000(1M)处是否也是这个数值。如果一直相同的话，就一直
;// 比较下去，也即死循环、死机。表示地址A20 线没有选通，结果内核就不能使用1M 以上内存。
	xor eax,eax
l1:	inc eax				;// check that A20 really IS enabled
	mov ds:[0],eax	;// loop forever if it isn't
	cmp ds:[100000h],eax
	je l1				;// '1b'表示向后(backward)跳转到标号1 去。
						;// 若是'5f'则表示向前(forward)跳转到标号5 去。
;/*
;* 注意! 在下面这段程序中，486 应该将位16 置位，以检查在超级用户模式下的写保护,
;* 此后"verify_area()"调用中就不需要了。486 的用户通常也会想将NE(;//5)置位，以便
;* 对数学协处理器的出错使用int 16。
;*/
;// 下面这段程序用于检查数学协处理器芯片是否存在。方法是修改控制寄存器CR0，在假设
;// 存在协处理器的情况下执行一个协处理器指令，如果出错的话则说明协处理器芯片不存
;// 在，需要设置CR0 中的协处理器仿真位EM（位2），并复位协处理器存在标志MP（位1）。
	mov eax,cr0				;// check math chip
	and eax,80000011h		;// Save PG,PE,ET
;/* "orl $0x10020,%eax" here for 486 might be good */
	or	 eax,2				;// set MP
	mov cr0,eax
	call check_x87
	jmp after_page_tables

;/*
;* 我们依赖于ET 标志的正确性来检测287/387 存在与否。
;*/
check_x87:
	fninit
	fstsw ax
	cmp al,0
	je l2				;/* no coprocessor: have to set bits */
	mov eax,cr0			;// 如果存在的则向前跳转到标号1 处，否则改写cr0。
	xor eax,6		;/* reset MP, set EM */
	mov cr0,eax
	ret
align 2	;// 这里".align 2"的含义是指存储边界对齐调整。
l2:			;// 即按4 字节方式对齐内存地址。
	 db 0DBh,0E4h		;/* 287 协处理器码。 */
	 ret

;/*
; * 下面这段是设置中断描述符表子程序setup_idt
; *
; * 将中断描述符表idt 设置成具有256 个项，并都指向ignore_int 中断门。然后加载
; * 中断描述符表寄存器(用lidt 指令)。真正实用的中断门以后再安装。当我们在其它
; * 地方认为一切都正常时再开启中断。该子程序将会被页表覆盖掉。
; */
setup_idt:
	lea edx,ignore_int		;// 将ignore_int 的有效地址（偏移值）值 edx 寄存器
	mov eax,00080000h		;// 将选择符0x0008 置入eax 的高16 位中。
	mov ax,dx				;/* selector = 0x0008 = cs */
							;// 偏移值的低16 位置入eax 的低16 位中。此时eax 含
							;// 有门描述符低4 字节的值。
	mov dx,8E00h		;/* interrupt gate - dpl=0, present */
							;// 此时edx 含有门描述符高4 字节的值。
	lea edi,_idt
	mov ecx,256
rp_sidt:
	mov [edi],eax		;// 将哑中断门描述符存入表中。
	mov [edi+4],edx
	add edi,8			;// edi 指向表中下一项。
	dec ecx
	jne rp_sidt
	lidt fword ptr idt_descr		;// 加载中断描述符表寄存器值。
	ret

;/*
; * 下面这段是设置全局描述符表项setup_gdt
; *
; * 这个子程序设置一个新的全局描述符表gdt，并加载。此时仅创建了两个表项，与前
; * 面的一样。该子程序只有两行，“非常的”复杂，所以当然需要这么长的注释了:)。
; */
setup_gdt:
	lgdt fword ptr gdt_descr		;// 加载全局描述符表寄存器(内容已设置好，见232-238 行)。
	ret

;/*
; * Linus 将内核的内存页表直接放在页目录之后，使用了4 个表来寻址16 Mb 的物理内存。
; * 如果你有多于16 Mb 的内存，就需要在这里进行扩充修改。
; */
;// 每个页表长为4 Kb 字节，而每个页表项需要4 个字节，因此一个页表共可以存放1000 个，
;// 表项如果一个表项寻址4 Kb 的地址空间，则一个页表就可以寻址4 Mb 的物理内存。页表项
;// 的格式为：项的前0-11 位存放一些标志，如是否在内存中(P 位0)、读写许可(R/W 位1)、
;// 普通用户还是超级用户使用(U/S 位2)、是否修改过(是否脏了)(D 位6)等；表项的位12-31 
;// 是页框地址，用于指出一页内存的物理起始地址。
org 1000h		;// 从偏移0x1000 处开始是第1 个页表（偏移0 开始处将存放页表目录）。
pg0:

org 2000h
pg1:

org 3000h
pg2:

org 4000h
pg3:

org 5000h		;// 定义下面的内存数据块从偏移0x5000 处开始。
;/*
; * 当DMA（直接存储器访问）不能访问缓冲块时，下面的tmp_floppy_area 内存块
; * 就可供软盘驱动程序使用。其地址需要对齐调整，这样就不会跨越64kB 边界。
; */
_tmp_floppy_area:
	db 1024 dup(0)		;// 共保留1024 项，每项1 字节，填充数值0 。

;// 下面这几个入栈操作(pushl)用于为调用/init/main.c 程序和返回作准备。
;// 前面3 个入栈指令不知道作什么用的，也许是Linus 用于在调试时能看清机器码用的.。
;// 139 行的入栈操作是模拟调用main.c 程序时首先将返回地址入栈的操作，所以如果
;// main.c 程序真的退出时，就会返回到这里的标号L6 处继续执行下去，也即死循环。
;// 140 行将main.c 的地址压入堆栈，这样，在设置分页处理（setup_paging）结束后
;// 执行'ret'返回指令时就会将main.c 程序的地址弹出堆栈，并去执行main.c 程序去了。
after_page_tables:
	push 0			;// These are the parameters to main :-)
	push 0			;// 这些是调用main 程序的参数（指init/main.c）。
	push 0
	push L6			;// return address for main, if it decides to.
	push _main_rename		;// '_main'是编译程序对main 的内部表示方法。
	jmp setup_paging
L6:
	jmp L6			;// main should never return here, but
				;// just in case, we know what happens.

;/* 下面是默认的中断“向量句柄” :-) */
int_msg:
	db "Unknown interrupt\n\r"		;// 定义字符串“未知中断(回车换行)”。
align 2				;// 按4 字节方式对齐内存地址。
ignore_int:
	push eax
	push ecx
	push edx
	push ds			;// 这里请注意！！ds,es,fs,gs 等虽然是16 位的寄存器，但入栈后
	push es			;// 仍然会以32 位的形式入栈，也即需要占用4 个字节的堆栈空间。
	push fs
	mov eax,10h			;// 置段选择符（使ds,es,fs 指向gdt 表中的数据段）。
	mov ds,ax
	mov es,ax
	mov fs,ax
	push int_msg		;// 把调用printk 函数的参数指针（地址）入栈。
	call _printk		;// 该函数在/kernel/printk.c 中。
						;// '_printk'是printk 编译后模块中的内部表示法。
	pop eax
	pop fs
	pop es
	pop ds
	pop edx
	pop ecx
	pop eax
	iretd		;// 中断返回（把中断调用时压入栈的CPU 标志寄存器（32 位）值也弹出）。


;/*
; * Setup_paging
; *
; * 这个子程序通过设置控制寄存器cr0 的标志（PG 位31）来启动对内存的分页处理
; * 功能，并设置各个页表项的内容，以恒等映射前16 MB 的物理内存。分页器假定
; * 不会产生非法的地址映射（也即在只有4Mb 的机器上设置出大于4Mb 的内存地址）。
; *
; * 注意！尽管所有的物理地址都应该由这个子程序进行恒等映射，但只有内核页面管
; * 理函数能直接使用>1Mb 的地址。所有“一般”函数仅使用低于1Mb 的地址空间，或
; * 者是使用局部数据空间，地址空间将被映射到其它一些地方去-- mm(内存管理程序)
; * 会管理这些事的。
; *
; * 对于那些有多于16Mb 内存的家伙- 太幸运了，我还没有，为什么你会有:-)。代码就
; * 在这里，对它进行修改吧。（实际上，这并不太困难的。通常只需修改一些常数等。
; * 我把它设置为16Mb，因为我的机器再怎么扩充甚至不能超过这个界限（当然，我的机 
; * 器很便宜的:-)）。我已经通过设置某类标志来给出需要改动的地方（搜索“16Mb”），
; * 但我不能保证作这些改动就行了 :-( )
; */
align 2		;// 按4 字节方式对齐内存地址边界。
setup_paging:	;// 首先对5 页内存（1 页目录+ 4 页页表）清零
	mov ecx,1024*5		;/* 5 pages - pg_dir+4 page tables */
	xor eax,eax
	xor edi,edi			;/* pg_dir is at 0x000 */
							;// 页目录从0x000 地址开始。
	pushf		;// VC内汇编使用cld和std后，需要自己恢复DF的值
	cld
	rep stosd
;// 下面4 句设置页目录中的项，我们共有4 个页表所以只需设置4 项。
;// 页目录项的结构与页表中项的结构一样，4 个字节为1 项。参见上面的说明。
;// "$pg0+7"表示：0x00001007，是页目录表中的第1 项。
;// 则第1 个页表所在的地址= 0x00001007 & 0xfffff000 = 0x1000；第1 个页表
;// 的属性标志= 0x00001007 & 0x00000fff = 0x07，表示该页存在、用户可读写。
	mov eax,_pg_dir
	mov [eax],pg0+7		;/* set present bit/user r/w */
	mov [eax+4],pg1+7		;/*  --------- " " --------- */
	mov [eax+8],pg2+7		;/*  --------- " " --------- */
	mov [eax+12],pg3+7		;/*  --------- " " --------- */
;// 下面6 行填写4 个页表中所有项的内容，共有：4(页表)*1024(项/页表)=4096 项(0 - 0xfff)，
;// 也即能映射物理内存4096*4Kb = 16Mb。
;// 每项的内容是：当前项所映射的物理内存地址+ 该页的标志（这里均为7）。
;// 使用的方法是从最后一个页表的最后一项开始按倒退顺序填写。一个页表的最后一项
;// 在页表中的位置是1023*4 = 4092。因此最后一页的最后一项的位置就是$pg3+4092。
	mov edi,pg3+4092		;// edi -> 最后一页的最后一项。
	mov eax,00fff007h		;/*  16Mb - 4096 + 7 (r/w user,p) */
							;// 最后1 项对应物理内存页面的地址是0xfff000，
							;// 加上属性标志7，即为0xfff007.
	std					;// 方向位置位，edi 值递减(4 字节)。
L3:	stosd				;/* fill pages backwards - more efficient :-) */
	sub eax,00001000h	;// 每填写好一项，物理地址值减0x1000。
	jge L3				;// 如果小于0 则说明全添写好了。
	popf
;// 设置页目录基址寄存器cr3 的值，指向页目录表。
	xor eax,eax		;/* 页目录表(pg_dir)在0x0000 处。 */
	mov cr3,eax		;/* cr3 - page directory start */
;// 设置启动使用分页处理（cr0 的PG 标志，位31）
	mov eax,cr0
	or  eax,80000000h	;// 添上PG 标志。
	mov cr0,eax			;/* set paging (PG) bit */
	ret						;/* this also flushes prefetch-queue */
;// 在改变分页处理标志后要求使用转移指令刷新预取指令队列，这里用的是返回指令ret。
;// 该返回指令的另一个作用是将堆栈中的main 程序的地址弹出，并开始运行/init/main.c 
;// 程序。本程序到此真正结束了。

align 2			;// 按4 字节方式对齐内存地址边界。
	dw 0
;//下面两行是lidt 指令的6 字节操作数：长度，基址。
idt_descr:
	dw 256*8-1
	dd _idt			;// idt contains 256 entries
align 2
	dw 0
;// 下面两行是lgdt 指令的6 字节操作数：长度，基址。
gdt_descr:
	dw 256*8-1		;// so does gdt (not that that's any
	dd _gdt			;// magic number, but it works for me :^)

align 4			;// 按8 字节方式对齐内存地址边界。
_idt:
	DQ 256 dup(0)	;// idt is uninitialized // 256 项，每项8 字节，填0。

;// 全局表。前4 项分别是空项（不用）、代码段描述符、数据段描述符、系统段描述符，
;// 其中系统段描述符linux 没有派用处。后面还预留了252 项的空间，用于放置所创建
;// 任务的局部描述符(LDT)和对应的任务状态段TSS 的描述符。
;// (0-nul, 1-cs, 2-ds, 3-sys, 4-TSS0, 5-LDT0, 6-TSS1, 7-LDT1, 8-TSS2 etc...)
_gdt:
	DQ 0000000000000000h	;/* NULL descriptor */
	DQ 00c09a0000000fffh	;/* 16Mb */  // 代码段最大长度16M。
	DQ 00c0920000000fffh	;/* 16Mb */	// 数据段最大长度16M。
	DQ 0000000000000000h	;/* TEMPORARY - don't use */
	DQ 252 dup(0)				;/* space for LDT's and TSS's etc */
end
