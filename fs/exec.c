/* passed
 *  linux/fs/exec.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

/*
 * #!开始的程序检测部分是由tytso 实现的。
 */

/*
 * 需求时加载是于1991.12.1 实现的- 只需将执行文件头部分读进内存而无须
 * 将整个执行文件都加载进内存。执行文件的i 节点被放在当前进程的可执行字段中
 * ("current-.h>executable")，而页异常会进行执行文件的实际加载操作以及清理工作。
 *
 * 我可以再一次自豪地说，linux 经得起修改：只用了不到2 小时的工作时间就完全
 * 实现了需求加载处理。
 */

#include <errno.h>// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。
#include <string.h>// 字符串头文件。主要定义了一些有关字符串操作的嵌入函数。
#include <sys/stat.h>// 文件状态头文件。含有文件或文件系统状态结构stat{}和常量。
#include <a.out.h>// a.out 头文件。定义了a.out 执行文件格式和一些宏。

#include <linux/fs.h>// 文件系统头文件。定义文件表结构（file,buffer_head,m_inode 等）。
#include <linux/sched.h>// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
						// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>// 内核头文件。含有一些内核常用函数的原形定义。
#include <linux/mm.h>// 内存管理头文件。含有页面大小定义和一些页面释放函数原型。
#include <asm/segment.h>// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

extern int sys_exit(int exit_code);// 程序退出系统调用。
extern int sys_close(int fd);// 文件关闭系统调用。

/*
 * MAX_ARG_PAGES 定义了新程序分配给参数和环境变量使用的内存最大页数。
 * 32 页内存应该足够了，这使得环境和参数(env+arg)空间的总合达到128kB!
 */
#define MAX_ARG_PAGES 32

/*
 * create_tables()函数在新用户内存中解析环境变量和参数字符串，由此
 * 创建指针表，并将它们的地址放到"堆栈"上，然后返回新栈的指针值。
 */
//// 在新用户堆栈中创建环境和参数变量指针表。
// 参数：p - 以数据段为起点的参数和环境信息偏移指针；argc - 参数个数；envc -环境变量数。
// 返回：堆栈指针。
static unsigned long * create_tables(char * p,int argc,int envc)
{
	unsigned long *argv,*envp;
	unsigned long * sp;

// 堆栈指针是以4 字节（1 节）为边界寻址的，因此这里让sp 为4 的整数倍。
	sp = (unsigned long *) (0xfffffffc & (unsigned long) p);
// sp 向下移动，空出环境参数占用的空间个数，并让环境参数指针envp 指向该处。
	sp -= envc+1;
	envp = sp;
// sp 向下移动，空出命令行参数指针占用的空间个数，并让argv 指针指向该处。
// 下面指针加1，sp 将递增指针宽度字节值。
	sp -= argc+1;
	argv = sp;
// 将环境参数指针envp 和命令行参数指针以及命令行参数个数压入堆栈。
	put_fs_long((unsigned long)envp,--sp);
	put_fs_long((unsigned long)argv,--sp);
	put_fs_long((unsigned long)argc,--sp);
// 将命令行各参数指针放入前面空出来的相应地方，最后放置一个NULL 指针。
	while (argc-->0) {
		put_fs_long((unsigned long) p,argv++);
		while (get_fs_byte(p++)) /* nothing */ ;// p 指针前移4 字节。
	}
	put_fs_long(0,argv);
// 将环境变量各指针放入前面空出来的相应地方，最后放置一个NULL 指针。
	while (envc-->0) {
		put_fs_long((unsigned long) p,envp++);
		while (get_fs_byte(p++)) /* nothing */ ;
	}
	put_fs_long(0,envp);
	return sp;		// 返回构造的当前新堆栈指针。
}

/*
 * count()函数计算命令行参数/环境变量的个数。
 */
//// 计算参数个数。
// 参数：argv - 参数指针数组，最后一个指针项是NULL。
// 返回：参数个数。
static int count(char ** argv)
{
	int i=0;
	char ** tmp;

	if (tmp = argv)
		while (get_fs_long((unsigned long *) (tmp++)))
			i++;

	return i;
}

/*
 * 'copy_string()'函数从用户内存空间拷贝参数和环境字符串到内核空闲页面内存中。
 * 这些已具有直接放到新用户内存中的格式。
 *
 * 由TYT(Tytso)于1991.12.24 日修改，增加了from_kmem 参数，该参数指明了字符串或
 * 字符串数组是来自用户段还是内核段。
 * 
 * from_kmem     argv *        argv **
 *    0          用户空间      用户空间
 *    1          内核空间      用户空间
 *    2          内核空间      内核空间
 * 
 * 我们是通过巧妙处理fs 段寄存器来操作的。由于加载一个段寄存器代价太大，所以
 * 我们尽量避免调用set_fs()，除非实在必要。
 */
//// 复制指定个数的参数字符串到参数和环境空间。
// 参数：argc - 欲添加的参数个数；argv - 参数指针数组；page - 参数和环境空间页面指针数组。
//       p -在参数表空间中的偏移指针，始终指向已复制串的头部；from_kmem - 字符串来源标志。
// 在do_execve()函数中，p 初始化为指向参数表(128kB)空间的最后一个长字处，参数字符串
// 是以堆栈操作方式逆向往其中复制存放的，因此p 指针会始终指向参数字符串的头部。
// 返回：参数和环境空间当前头部指针。
static unsigned long copy_strings(int argc,char ** argv,unsigned long *page,
		unsigned long p, int from_kmem)
{
	char *tmp, *pag;
	int len, offset = 0;
	unsigned long old_fs, new_fs;

	if (!p)
		return 0;	/* 偏移指针验证 */
// 取ds 寄存器值到new_fs，并保存原fs 寄存器值到old_fs。
	new_fs = get_ds();
	old_fs = get_fs();
// 如果字符串和字符串数组来自内核空间，则设置fs 段寄存器指向内核数据段（ds）。
	if (from_kmem==2)
		set_fs(new_fs);
// 循环处理各个参数，从最后一个参数逆向开始复制，复制到指定偏移地址处。
	while (argc-- > 0) {
// 如果字符串在用户空间而字符串数组在内核空间，则设置fs 段寄存器指向内核数据段（ds）。
		if (from_kmem == 1)
			set_fs(new_fs);
// 从最后一个参数开始逆向操作，取fs 段中最后一参数指针到tmp，如果为空，则出错死机。
		if (!(tmp = (char *)get_fs_long(((unsigned long *)argv)+argc)))
			panic("argc is wrong");
// 如果字符串在用户空间而字符串数组在内核空间，则恢复fs 段寄存器原值。
		if (from_kmem == 1)
			set_fs(old_fs);
// 计算该参数字符串长度len，并使tmp 指向该参数字符串末端。
		len=0;		/* 我们知道串是以NULL 字节结尾的 */
		do {
			len++;
		} while (get_fs_byte(tmp++));
// 如果该字符串长度超过此时参数和环境空间中还剩余的空闲长度，则恢复fs 段寄存器并返回0。
		if (p-len < 0) {	/* 不会发生-因为有128kB 的空间 */
			set_fs(old_fs);
			return 0;
		}
// 复制fs 段中当前指定的参数字符串，是从该字符串尾逆向开始复制。
		while (len) {
			--p; --tmp; --len;
// 函数刚开始执行时，偏移变量offset 被初始化为0，因此若offset-1<0，说明是首次复制字符串，
// 则令其等于p 指针在页面内的偏移值，并申请空闲页面。
			if (--offset < 0) {
				offset = p % PAGE_SIZE;
// 如果字符串和字符串数组在内核空间，则恢复fs 段寄存器原值。
				if (from_kmem==2)
					set_fs(old_fs);
// 如果当前偏移值p 所在的串空间页面指针数组项page[p/PAGE_SIZE]==0，表示相应页面还不存在，
// 则需申请新的内存空闲页面，将该页面指针填入指针数组，并且也使pag 指向该新页面，若申请不
// 到空闲页面则返回0。
				if (!(pag = (char *) page[p/PAGE_SIZE]) &&
				    !(pag = (char *) page[p/PAGE_SIZE] = 
				      (char *) get_free_page())) 
					return 0;
// 如果字符串和字符串数组来自内核空间，则设置fs 段寄存器指向内核数据段（ds）。
				if (from_kmem==2)
					set_fs(new_fs);

			}
// 从fs 段中复制参数字符串中一字节到pag+offset 处。
			*(pag + offset) = get_fs_byte(tmp);
		}
	}
// 如果字符串和字符串数组在内核空间，则恢复fs 段寄存器原值。
	if (from_kmem==2)
		set_fs(old_fs);
// 最后，返回参数和环境空间中已复制参数信息的头部偏移值。
	return p;
}

//// 修改局部描述符表中的描述符基址和段限长，并将参数和环境空间页面放置在数据段末端。
// 参数：text_size - 执行文件头部中a_text 字段给出的代码段长度值；
//       page - 参数和环境空间页面指针数组。
// 返回：数据段限长值(64MB)。
static unsigned long change_ldt(unsigned long text_size,unsigned long * page)
{
	unsigned long code_limit,data_limit,code_base,data_base;
	int i;

// 根据执行文件头部a_text 值，计算以页面长度为边界的代码段限长。并设置数据段长度为64MB。
	code_limit = text_size+PAGE_SIZE -1;
	code_limit &= 0xFFFFF000;
	data_limit = 0x4000000;
// 取当前进程中局部描述符表代码段描述符中代码段基址，代码段基址与数据段基址相同。
	code_base = get_base(current->ldt[1]);
	data_base = code_base;
// 重新设置局部表中代码段和数据段描述符的基址和段限长。
	set_base(current->ldt[1],code_base);
	set_limit(current->ldt[1],code_limit);
	set_base(current->ldt[2],data_base);
	set_limit(current->ldt[2],data_limit);
/* 要确信fs 段寄存器已指向新的数据段 */
// fs 段寄存器中放入局部表数据段描述符的选择符(0x17)。
//	__asm__("pushl $0x17\n\tpop %%fs"::);
	_asm {
		push 0x17
		pop fs
	}
// 将参数和环境空间已存放数据的页面（共可有MAX_ARG_PAGES 页，128kB）放到数据段线性地址的
// 末端。是调用函数put_page()进行操作的（mm/memory.c, 197）。
	data_base += data_limit;
	for (i=MAX_ARG_PAGES-1 ; i>=0 ; i--) {
		data_base -= PAGE_SIZE;
		if (page[i])						// 如果该页面存在，
			put_page(page[i],data_base);	// 就放置该页面。
	}
	return data_limit;		// 最后返回数据段限长(64MB)。
}

/*
 * 'do_execve()'函数执行一个新程序。
 */
//// execve()系统中断调用函数。加载并执行子进程（其它程序）。
// 该函数系统中断调用(int 0x80)功能号__NR_execve 调用的函数。
// 参数：eip - 指向堆栈中调用系统中断的程序代码指针eip 处，参见kernel/system_call.s 程序
// 开始部分的说明；tmp - 系统中断调用本函数时的返回地址，无用；
//                 filename - 被执行程序文件名；argv - 命令行参数指针数组；
//                 envp - 环境变量指针数组。
// 返回：如果调用成功，则不返回；否则设置出错号，并返回-1。
int do_execve(unsigned long * eip,long tmp,char * filename,
	char ** argv, char ** envp)
{
	struct m_inode * inode;				// 内存中I 节点指针结构变量。
	struct buffer_head * bh;			// 高速缓存块头指针。
	struct exec ex;						// 执行文件头部数据结构变量。
	unsigned long page[MAX_ARG_PAGES];	// 参数和环境字符串空间的页面指针数组。
	int i,argc,envc;
	int e_uid, e_gid;					// 有效用户id 和有效组id。
	int retval;							// 返回值。
	int sh_bang = 0;					// 控制是否需要执行脚本处理代码。
// 参数和环境字符串空间中的偏移指针，初始化为指向该空间的最后一个长字处。
	unsigned long p=PAGE_SIZE*MAX_ARG_PAGES-4;

// eip[1]中是原代码段寄存器cs，其中的选择符不可以是内核段选择符，也即内核不能调用本函数。
	if ((0xffff & eip[1]) != 0x000f)
		panic("execve called from supervisor mode");
// 初始化参数和环境串空间的页面指针数组（表）。
	for (i=0 ; i<MAX_ARG_PAGES ; i++)	/* clear page-table */
		page[i]=0;
// 取可执行文件的对应i 节点号。
	if (!(inode=namei(filename)))		/* get executables inode */
		return -ENOENT;
// 计算参数个数和环境变量个数。
	argc = count(argv);
	envc = count(envp);
	
// 执行文件必须是常规文件。若不是常规文件则置出错返回码，跳转到exec_error2(第347 行)。
restart_interp:
	if (!S_ISREG(inode->i_mode)) {	/* must be regular file */
		retval = -EACCES;
		goto exec_error2;
	}
// 检查被执行文件的执行权限。根据其属性(对应i 节点的uid 和gid)，看本进程是否有权执行它。
	i = inode->i_mode;
	e_uid = (i & S_ISUID) ? inode->i_uid : current->euid;
	e_gid = (i & S_ISGID) ? inode->i_gid : current->egid;
	if (current->euid == inode->i_uid)
		i >>= 6;
	else if (current->egid == inode->i_gid)
		i >>= 3;
	if (!(i & 1) &&
	    !((inode->i_mode & 0111) && suser())) {
		retval = -ENOEXEC;
		goto exec_error2;
	}
// 读取执行文件的第一块数据到高速缓冲区，若出错则置出错码，跳转到exec_error2 处去处理。
	if (!(bh = bread(inode->i_dev,inode->i_zone[0]))) {
		retval = -EACCES;
		goto exec_error2;
	}
// 下面对执行文件的头结构数据进行处理，首先让ex 指向执行头部分的数据结构。
	ex = *((struct exec *) bh->b_data);	/* 读取执行头部分 */
// 如果执行文件开始的两个字节为'#!'，并且sh_bang 标志没有置位，则处理脚本文件的执行。
	if ((bh->b_data[0] == '#') && (bh->b_data[1] == '!') && (!sh_bang)) {
		/*
		 * 这部分处理对'#!'的解释，有些复杂，但希望能工作。-TYT
		 */

		char buf[1023], *cp, *interp, *i_name, *i_arg;
		unsigned long old_fs;

// 复制执行程序头一行字符'#!'后面的字符串到buf 中，其中含有脚本处理程序名。
		strncpy(buf, bh->b_data+2, 1022);
// 释放高速缓冲块和该执行文件i 节点。
		brelse(bh);
		iput(inode);
// 取第一行内容，并删除开始的空格、制表符。
		buf[1022] = '\0';
		if (cp = strchr(buf, '\n')) {
			*cp = '\0';
			for (cp = buf; (*cp == ' ') || (*cp == '\t'); cp++);
		}
// 若该行没有其它内容，则出错。置出错码，跳转到exec_error1 处。
		if (!cp || *cp == '\0') {
			retval = -ENOEXEC; /* No interpreter name found */
			goto exec_error1;
		}
// 否则就得到了开头是脚本解释执行程序名称的一行内容。
		interp = i_name = cp;
// 下面分析该行。首先取第一个字符串，其应该是脚本解释程序名，iname 指向该名称。
		i_arg = 0;
		for ( ; *cp && (*cp != ' ') && (*cp != '\t'); cp++) {
 			if (*cp == '/')
				i_name = cp+1;
		}
// 若文件名后还有字符，则应该是参数串，令i_arg 指向该串。
		if (*cp) {
			*cp++ = '\0';
			i_arg = cp;
		}
		/*
		 * OK，我们已经解析出解释程序的文件名以及(可选的)参数。
		 */
// 若sh_bang 标志没有设置，则设置它，并复制指定个数的环境变量串和参数串到参数和环境空间中。
		if (sh_bang++ == 0) {
			p = copy_strings(envc, envp, page, p, 0);
			p = copy_strings(--argc, argv+1, page, p, 0);
		}
		/*
		 * 拼接(1) argv[0]中放解释程序的名称
		 * (2) (可选的)解释程序的参数
		 * (3) 脚本程序的名称
		 *
		 * 这是以逆序进行处理的，是由于用户环境和参数的存放方式造成的。
		 */
// 复制脚本程序文件名到参数和环境空间中。
		p = copy_strings(1, &filename, page, p, 1);
// 复制解释程序的参数到参数和环境空间中。
		argc++;
		if (i_arg) {
			p = copy_strings(1, &i_arg, page, p, 2);
			argc++;
		}
// 复制解释程序文件名到参数和环境空间中。若出错，则置出错码，跳转到exec_error1。
		p = copy_strings(1, &i_name, page, p, 2);
		argc++;
		if (!p) {
			retval = -ENOMEM;
			goto exec_error1;
		}
		/*
		 * OK，现在使用解释程序的i 节点重启进程。
		 */
// 保留原fs 段寄存器（原指向用户数据段），现置其指向内核数据段。
		old_fs = get_fs();
		set_fs(get_ds());
// 取解释程序的i 节点，并跳转到restart_interp 处重新处理。
		if (!(inode=namei(interp))) { /* get executables inode */
			set_fs(old_fs);
			retval = -ENOENT;
			goto exec_error1;
		}
		set_fs(old_fs);
		goto restart_interp;
	}
// 释放该缓冲区。
	brelse(bh);
// 下面对执行头信息进行处理。
// 对于下列情况，将不执行程序：如果执行文件不是需求页可执行文件(ZMAGIC)、或者代码重定位部分
// 长度a_trsize 不等于0、或者数据重定位信息长度不等于0、或者代码段+数据段+堆段长度超过50MB、
// 或者i 节点表明的该执行文件长度小于代码段+数据段+符号表长度+执行头部分长度的总和。
	if (N_MAGIC(ex) != ZMAGIC || ex.a_trsize || ex.a_drsize ||
		ex.a_text+ex.a_data+ex.a_bss>0x3000000 ||
		inode->i_size < ex.a_text+ex.a_data+ex.a_syms+N_TXTOFF(ex)) {
		retval = -ENOEXEC;
		goto exec_error2;
	}
// 如果执行文件执行头部分长度不等于一个内存块大小（1024 字节），也不能执行。转exec_error2。
	if (N_TXTOFF(ex) != BLOCK_SIZE) {
		printk("%s: N_TXTOFF != BLOCK_SIZE. See a.out.h.", filename);
		retval = -ENOEXEC;
		goto exec_error2;
	}
// 如果sh_bang 标志没有设置，则复制指定个数的环境变量字符串和参数到参数和环境空间中。
// 若sh_bang 标志已经设置，则表明是将运行脚本程序，此时环境变量页面已经复制，无须再复制。
	if (!sh_bang) {
		p = copy_strings(envc,envp,page,p,0);
		p = copy_strings(argc,argv,page,p,0);
// 如果p=0，则表示环境变量与参数空间页面已经被占满，容纳不下了。转至出错处理处。
		if (!p) {
			retval = -ENOMEM;
			goto exec_error2;
		}
	}
/* OK，下面开始就没有返回的地方了 */
// 如果原程序也是一个执行程序，则释放其i 节点，并让进程executable 字段指向新程序i 节点。
	if (current->executable)
		iput(current->executable);
	current->executable = inode;
// 清复位所有信号处理句柄。但对于SIG_IGN 句柄不能复位，因此在322 与323 行之间需添加一条
// if 语句：if (current->sa[I].sa_handler != SIG_IGN)。这是源代码中的一个bug。
	for (i=0 ; i<32 ; i++)
		current->sigaction[i].sa_handler = NULL;
// 根据执行时关闭(close_on_exec)文件句柄位图标志，关闭指定的打开文件，并复位该标志。
	for (i=0 ; i<NR_OPEN ; i++)
		if ((current->close_on_exec>>i)&1)
			sys_close(i);
	current->close_on_exec = 0;
// 根据指定的基地址和限长，释放原来程序代码段和数据段所对应的内存页表指定的内存块及页表本身。
	free_page_tables(get_base(current->ldt[1]),get_limit(0x0f));
	free_page_tables(get_base(current->ldt[2]),get_limit(0x17));
// 如果“上次任务使用了协处理器”指向的是当前进程，则将其置空，并复位使用了协处理器的标志。
	if (last_task_used_math == current)
		last_task_used_math = NULL;
	current->used_math = 0;
// 根据a_text 修改局部表中描述符基址和段限长，并将参数和环境空间页面放置在数据段末端。
// 执行下面语句之后，p 此时是以数据段起始处为原点的偏移值，仍指向参数和环境空间数据开始处，
// 也即转换成为堆栈的指针。
	p += change_ldt(ex.a_text,page)-MAX_ARG_PAGES*PAGE_SIZE;
// create_tables()在新用户堆栈中创建环境和参数变量指针表，并返回该堆栈指针。
	p = (unsigned long) create_tables((char *)p,argc,envc);
// 修改当前进程各字段为新执行程序的信息。令进程代码段尾值字段end_code = a_text；令进程数据
// 段尾字段end_data = a_data + a_text；令进程堆结尾字段brk = a_text + a_data + a_bss。
	current->brk = ex.a_bss +
		(current->end_data = ex.a_data +
		(current->end_code = ex.a_text));
// 设置进程堆栈开始字段为堆栈指针所在的页面，并重新设置进程的用户id 和组id。
	current->start_stack = p & 0xfffff000;
	current->euid = e_uid;
	current->egid = e_gid;
// 初始化一页bss 段数据，全为零。
	i = ex.a_text+ex.a_data;
	while (i&0xfff)
		put_fs_byte(0,(char *) (i++));
// 将原调用系统中断的程序在堆栈上的代码指针替换为指向新执行程序的入口点，并将堆栈指针替换
// 为新执行程序的堆栈指针。返回指令将弹出这些堆栈数据并使得CPU 去执行新的执行程序，因此不会
// 返回到原调用系统中断的程序中去了。
	eip[0] = ex.a_entry;		/* eip，魔法起作用了 :-) */
	eip[3] = p;			/* esp，堆栈指针 */
	return 0;
exec_error2:
	iput(inode);
exec_error1:
	for (i=0 ; i<MAX_ARG_PAGES ; i++)
		free_page(page[i]);
	return(retval);
}
