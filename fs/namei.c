/* passed
 *  linux/fs/namei.c
 *
 *  (C) 1991  Linus Torvalds
 */
#include <set_seg.h>

/*
 * tytso 作了一些纠正。
 */

#include <linux/sched.h>// 调度程序头文件，定义了任务结构task_struct、初始任务0 的数据，
							// 还有一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
#include <linux/kernel.h>// 内核头文件。含有一些内核常用函数的原形定义。
#include <asm/segment.h>// 段操作头文件。定义了有关段寄存器操作的嵌入式汇编函数。

#include <string.h>// 字符串头文件。主要定义了一些有关字符串操作的嵌入函数。
#include <fcntl.h>// 文件控制头文件。用于文件及其描述符的操作控制常数符号的定义。
#include <errno.h>// 错误号头文件。包含系统中各种出错号。(Linus 从minix 中引进的)。
#include <const.h>// 常数符号头文件。目前仅定义了i 节点中i_mode 字段的各标志位。
#include <sys/stat.h>// 文件状态头文件。含有文件或文件系统状态结构stat{}和常量。

// 访问模式宏。x 是include/fcntl.h 第7 行开始定义的文件访问标志。
// 根据x 值索引对应数值（数值表示rwx 权限: r, w, rw, wxrwxrwx）(数值是8 进制)。
#define ACC_MODE(x) ("\004\002\006\377"[(x)&O_ACCMODE])

/*
 * 如果想让文件名长度>NAME_LEN 的字符被截掉，就将下面定义注释掉。
 */
/* #define NO_TRUNCATE */

#define MAY_EXEC 1		// 可执行(可进入)。
#define MAY_WRITE 2		// 可写。
#define MAY_READ 4		// 可读。

/*
 *	permission()
 *
 * 该函数用于检测一个文件的读/写/执行权限。我不知道是否只需检查euid，还是
 * 需要检查euid 和uid 两者，不过这很容易修改。
 */
//// 检测文件访问许可权限。
// 参数：inode - 文件对应的i 节点；mask - 访问属性屏蔽码。
// 返回：访问许可返回1，否则返回0。
static int permission(struct m_inode * inode,int mask)
{
	int mode = inode->i_mode;// 文件访问属性

/* 特殊情况：即使是超级用户(root)也不能读/写一个已被删除的文件 */
// 如果i 节点有对应的设备，但该i 节点的连接数等于0，则返回。
	if (inode->i_dev && !inode->i_nlinks)
		return 0;
// 否则，如果进程的有效用户id(euid)与i 节点的用户id 相同，则取文件宿主的用户访问权限。
	else if (current->euid==inode->i_uid)
		mode >>= 6;
// 否则，如果进程的有效组id(egid)与i 节点的组id 相同，则取组用户的访问权限。
	else if (current->egid==inode->i_gid)
		mode >>= 3;
// 如果上面所取的的访问权限与屏蔽码相同，或者是超级用户，则返回1，否则返回0。
	if (((mode & mask & 0007) == mask) || suser()) // suser()在linux/kernel.h。
		return 1;
	return 0;
}

/*
 * ok，我们不能使用strncmp 字符串比较函数，因为名称不在我们的数据空间(不在内核空间)。
 * 因而我们只能使用match()。问题不大。match()同样也处理一些完整的测试。
 *
 * 注意！与strncmp 不同的是match()成功时返回1，失败时返回0。
 */
//// 指定长度字符串比较函数。
// 参数：len - 比较的字符串长度；name - 文件名指针；de - 目录项结构。
// 返回：相同返回1，不同返回0。
static int match(int len,const char * name,struct dir_entry * de)
{
	register int same; //__asm__("ax")
	char *de_name;

// 如果目录项指针空，或者目录项i 节点等于0，或者要比较的字符串长度超过文件名长度，则返回0。
	if (!de || !de->inode || len > NAME_LEN)
		return 0;
// 如果要比较的长度len 小于NAME_LEN，但是目录项中文件名长度超过len，则返回0。
	if (len < NAME_LEN && de->name[len])
		return 0;
// 下面嵌入汇编语句，在用户数据空间(fs)执行字符串的比较操作。
// %0 - eax(比较结果same)；%1 - eax(eax 初值0)；%2 - esi(名字指针)；%3 - edi(目录项名指针)；
// %4 - ecx(比较的字节长度值len)。
/*	__asm__("cld\n\t"				// 清方向位。
		"fs ; repe ; cmpsb\n\t"		// 用户空间执行循环比较[esi++]和[edi++]操作，
		"setz %%al"					// 若比较结果一样(z=0)则设置al=1(same=eax)。
		:"=a" (same)
		:"0" (0),"S" ((long) name),"D" ((long) de->name),"c" (len)
		:"cx","di","si");*/
	de_name = de->name;
	_asm{
		pushf
		xor eax,eax
		mov esi,name
		mov edi,de_name
		mov ecx,len
		cld		// 清方向位。
		// 用户空间执行循环比较[esi++]和[edi++]操作，
		repe cmps byte ptr fs:[edi],[esi]
		//上语句应该是错误的，但我不知道怎么改。还好系统可以运行:)
		setz al			// 若比较结果一样(z=0)则设置al=1(same=eax)。
		mov same,eax
		popf
	}
	return same;			// 返回比较结果。
}

/*
 *	find_entry()
 *
 * 在指定的目录中寻找一个与名字匹配的目录项。返回一个含有找到目录项的高速
 * 缓冲区以及目录项本身(作为一个参数- res_dir)。并不读目录项的i 节点- 如
 * 果需要的话需自己操作。
 *
 * '..'目录项，操作期间也会对几种特殊情况分别处理- 比如横越一个伪根目录以
 * 及安装点。
 */
//// 查找指定目录和文件名的目录项。
// 参数：dir - 指定目录i 节点的指针；name - 文件名；namelen - 文件名长度；
// 返回：高速缓冲区指针；res_dir - 返回的目录项结构指针；
static struct buffer_head * find_entry(struct m_inode ** dir,
	const char * name, int namelen, struct dir_entry ** res_dir)
{
	int entries;
	int block,i;
	struct buffer_head * bh;
	struct dir_entry * de;
	struct super_block * sb;

// 如果定义了NO_TRUNCATE，则若文件名长度超过最大长度NAME_LEN，则返回。
#ifdef NO_TRUNCATE
	if (namelen > NAME_LEN)
		return NULL;
//如果没有定义NO_TRUNCATE，则若文件名长度超过最大长度NAME_LEN，则截短之。
#else
	if (namelen > NAME_LEN)
		namelen = NAME_LEN;
#endif
// 计算本目录中目录项项数entries。置空返回目录项结构指针。
	entries = (*dir)->i_size / (sizeof (struct dir_entry));
	*res_dir = NULL;
// 如果文件名长度等于0，则返回NULL，退出。
	if (!namelen)
		return NULL;
/* 检查目录项'..'，因为可能需要对其特别处理 */
	if (namelen==2 && get_fs_byte(name)=='.' && get_fs_byte(name+1)=='.') {
/* 伪根中的'..'如同一个假'.'(只需改变名字长度) */
// 如果当前进程的根节点指针即是指定的目录，则将文件名修改为'.'，
		if ((*dir) == current->root)
			namelen=1;
// 否则如果该目录的i 节点号等于ROOT_INO(1)的话，说明是文件系统根节点。则取文件系统的超级块。
		else if ((*dir)->i_num == ROOT_INO) {
/* 在一个安装点上的'..'将导致目录交换到安装到文件系统的目录i 节点。
   注意！由于设置了mounted 标志，因而我们能够取出该新目录 */
			sb=get_super((*dir)->i_dev);
// 如果被安装到的i 节点存在，则先释放原i 节点，然后对被安装到的i 节点进行处理。
// 让*dir 指向该被安装到的i 节点；该i 节点的引用数加1。
			if (sb->s_imount) {
				iput(*dir);
				(*dir)=sb->s_imount;
				(*dir)->i_count++;
			}
		}
	}
// 如果该i 节点所指向的第一个直接磁盘块号为0，则返回NULL，退出。
	if (!(block = (*dir)->i_zone[0]))
		return NULL;
// 从节点所在设备读取指定的目录项数据块，如果不成功，则返回NULL，退出。
	if (!(bh = bread((*dir)->i_dev,block)))
		return NULL;
// 在目录项数据块中搜索匹配指定文件名的目录项，首先让de 指向数据块，并在不超过目录中目录项数
// 的条件下，循环执行搜索。
	i = 0;
	de = (struct dir_entry *) bh->b_data;
	while (i < entries) {
// 如果当前目录项数据块已经搜索完，还没有找到匹配的目录项，则释放当前目录项数据块。
		if ((char *)de >= BLOCK_SIZE+bh->b_data) {
			brelse(bh);
			bh = NULL;
// 在读入下一目录项数据块。若这块为空，则只要还没有搜索完目录中的所有目录项，就跳过该块，
// 继续读下一目录项数据块。若该块不空，就让de 指向该目录项数据块，继续搜索。
			if (!(block = bmap(*dir,i/DIR_ENTRIES_PER_BLOCK)) ||
			    !(bh = bread((*dir)->i_dev,block))) {
				i += DIR_ENTRIES_PER_BLOCK;
				continue;
			}
			de = (struct dir_entry *) bh->b_data;
		}
// 如果找到匹配的目录项的话，则返回该目录项结构指针和该目录项数据块指针，退出。
		if (match(namelen,name,de)) {
			*res_dir = de;
			return bh;
		}
// 否则继续在目录项数据块中比较下一个目录项。
		de++;
		i++;
	}
// 若指定目录中的所有目录项都搜索完还没有找到相应的目录项，则释放目录项数据块，返回NULL。
	brelse(bh);
	return NULL;
}

/*
 *	add_entry()
 *
 * 使用与find_entry()同样的方法，往指定目录中添加一文件目录项。
 * 如果失败则返回NULL。
 *
 * 注意！！'de'(指定目录项结构指针)的i 节点部分被设置为0 - 这表示
 * 在调用该函数和往目录项中添加信息之间不能睡眠，因为若睡眠那么其它
 * 人(进程)可能会已经使用了该目录项。
 */
//// 根据指定的目录和文件名添加目录项。
// 参数：dir - 指定目录的i 节点；name - 文件名；namelen - 文件名长度；
// 返回：高速缓冲区指针；res_dir - 返回的目录项结构指针；
static struct buffer_head * add_entry(struct m_inode * dir,
	const char * name, int namelen, struct dir_entry ** res_dir)
{
	int block,i;
	struct buffer_head * bh;
	struct dir_entry * de;

	*res_dir = NULL;
// 如果定义了NO_TRUNCATE，则若文件名长度超过最大长度NAME_LEN，则返回。
#ifdef NO_TRUNCATE
	if (namelen > NAME_LEN)
		return NULL;
//如果没有定义NO_TRUNCATE，则若文件名长度超过最大长度NAME_LEN，则截短之。
#else
	if (namelen > NAME_LEN)
		namelen = NAME_LEN;
#endif
// 如果文件名长度等于0，则返回NULL，退出。
	if (!namelen)
		return NULL;
// 如果该目录i 节点所指向的第一个直接磁盘块号为0，则返回NULL 退出。
	if (!(block = dir->i_zone[0]))
		return NULL;
// 如果读取该磁盘块失败，则返回NULL 并退出。
	if (!(bh = bread(dir->i_dev,block)))
		return NULL;
// 在目录项数据块中循环查找最后未使用的目录项。首先让目录项结构指针de 指向高速缓冲的数据块
// 开始处，也即第一个目录项。
	i = 0;
	de = (struct dir_entry *) bh->b_data;
	while (1) {
// 如果当前判别的目录项已经超出当前数据块，则释放该数据块，重新申请一块磁盘块block。如果
// 申请失败，则返回NULL，退出。
		if ((char *)de >= BLOCK_SIZE+bh->b_data) {
			brelse(bh);
			bh = NULL;
			block = create_block(dir,i/DIR_ENTRIES_PER_BLOCK);
			if (!block)
				return NULL;
// 如果读取磁盘块返回的指针为空，则跳过该块继续。
			if (!(bh = bread(dir->i_dev,block))) {
				i += DIR_ENTRIES_PER_BLOCK;
				continue;
			}
// 否则，让目录项结构指针de 志向该块的高速缓冲数据块开始处。
			de = (struct dir_entry *) bh->b_data;
		}
// 如果当前所操作的目录项序号i*目录结构大小已经超过了该目录所指出的大小i_size，则说明该第i
// 个目录项还未使用，我们可以使用它。于是对该目录项进行设置(置该目录项的i 节点指针为空)。并
// 更新该目录的长度值(加上一个目录项的长度，设置目录的i 节点已修改标志，再更新该目录的改变时
// 间为当前时间。
		if (i*sizeof(struct dir_entry) >= dir->i_size) {
			de->inode=0;
			dir->i_size = (i+1)*sizeof(struct dir_entry);
			dir->i_dirt = 1;
			dir->i_ctime = CURRENT_TIME;
		}
// 若该目录项的i 节点为空，则表示找到一个还未使用的目录项。于是更新目录的修改时间为当前时间。
// 并从用户数据区复制文件名到该目录项的文件名字段，置相应的高速缓冲块已修改标志。返回该目录
// 项的指针以及该高速缓冲区的指针，退出。
		if (!de->inode) {
			dir->i_mtime = CURRENT_TIME;
			for (i=0; i < NAME_LEN ; i++)
				de->name[i]=(i<namelen)?get_fs_byte(name+i):0;
			bh->b_dirt = 1;
			*res_dir = de;
			return bh;
		}
// 如果该目录项已经被使用，则继续检测下一个目录项。
		de++;
		i++;
	}
// 执行不到这里。也许Linus 在写这段代码时是先复制了上面find_entry()的代码，而后修改的:)
	brelse(bh);
	return NULL;
}

/*
 *	get_dir()
 *
 * 该函数根据给出的路径名进行搜索，直到达到最顶端的目录。
 * 如果失败则返回NULL。
 */
//// 搜寻指定路径名的目录。
// 参数：pathname - 路径名。
// 返回：目录的i 节点指针。失败时返回NULL。
static struct m_inode * get_dir(const char * pathname)
{
	char c;
	const char * thisname;
	struct m_inode * inode;
	struct buffer_head * bh;
	int namelen,inr,idev;
	struct dir_entry * de;

// 如果进程没有设定根i 节点，或者该进程根i 节点的引用为0，则系统出错，死机。
	if (!current->root || !current->root->i_count)
		panic("No root inode");
// 如果进程的当前工作目录指针为空，或者该当前目录i 节点的引用计数为0，也是系统有问题，死机。
	if (!current->pwd || !current->pwd->i_count)
		panic("No cwd inode");
// 如果用户指定的路径名的第1 个字符是'/'，则说明路径名是绝对路径名。则从根i 节点开始操作。
	if ((c=get_fs_byte(pathname))=='/') {
		inode = current->root;
		pathname++;
// 否则若第一个字符是其它字符，则表示给定的是相对路径名。应从进程的当前工作目录开始操作。
// 则取进程当前工作目录的i 节点。
	} else if (c)
		inode = current->pwd;
// 否则表示路径名为空，出错。返回NULL，退出。
	else
		return NULL;	/* 空的路径名是错误的 */
// 将取得的i 节点引用计数增1。
	inode->i_count++;
	while (1) {
// 若该i 节点不是目录节点，或者没有可进入的访问许可，则释放该i 节点，返回NULL，退出。
		thisname = pathname;
		if (!S_ISDIR(inode->i_mode) || !permission(inode,MAY_EXEC)) {
			iput(inode);
			return NULL;
		}
// 从路径名开始起搜索检测字符，直到字符已是结尾符(NULL)或者是'/'，此时namelen 正好是当前处理
// 目录名的长度。如果最后也是一个目录名，但其后没有加'/'，则不会返回该最后目录的i 节点！
// 比如：/var/log/httpd，将只返回log/目录的i 节点。
		for(namelen=0;(c=get_fs_byte(pathname++))&&(c!='/');namelen++)
			/* nothing */ ;
// 若字符是结尾符NULL，则表明已经到达指定目录，则返回该i 节点指针，退出。
		if (!c)
			return inode;
// 调用查找指定目录和文件名的目录项函数，在当前处理目录中寻找子目录项。如果没有找到，
// 则释放该i 节点，并返回NULL，退出。
		if (!(bh = find_entry(&inode,thisname,namelen,&de))) {
			iput(inode);
			return NULL;
		}
// 取该子目录项的i 节点号inr 和设备号idev，释放包含该目录项的高速缓冲块和该i 节点。
		inr = de->inode;
		idev = inode->i_dev;
		brelse(bh);
		iput(inode);
// 取节点号inr 的i 节点信息，若失败，则返回NULL，退出。否则继续以该子目录的i 节点进行操作。
		if (!(inode = iget(idev,inr)))
			return NULL;
	}
}

/*
 *	dir_namei()
 *
 * dir_namei()函数返回指定目录名的i 节点指针，以及在最顶层目录的名称。
 */
// 参数：pathname - 目录路径名；namelen - 路径名长度。
// 返回：指定目录名最顶层目录的i 节点指针和最顶层目录名及其长度。
static struct m_inode * dir_namei(const char * pathname,
	int * namelen, const char ** name)
{
	char c;
	const char * basename;
	struct m_inode * dir;

// 取指定路径名最顶层目录的i 节点，若出错则返回NULL，退出。
	if (!(dir = get_dir(pathname)))
		return NULL;
// 对路径名pathname 进行搜索检测，查处最后一个'/'后面的名字字符串，计算其长度，并返回最顶
// 层目录的i 节点指针。
	basename = pathname;
	while (c=get_fs_byte(pathname++))
		if (c=='/')
			basename=pathname;
	*namelen = pathname-basename-1;
	*name = basename;
	return dir;
}

/*
 *	namei()
 *
 * 该函数被许多简单的命令用于取得指定路径名称的i 节点。open、link 等则使用它们
 * 自己的相应函数，但对于象修改模式'chmod'等这样的命令，该函数已足够用了。
 */
//// 取指定路径名的i 节点。
// 参数：pathname - 路径名。
// 返回：对应的i 节点。
struct m_inode * namei(const char * pathname)
{
	const char * basename;
	int inr,dev,namelen;
	struct m_inode * dir;
	struct buffer_head * bh;
	struct dir_entry * de;

// 首先查找指定路径的最顶层目录的目录名及其i 节点，若不存在，则返回NULL，退出。
	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return NULL;
// 如果返回的最顶层名字的长度是0，则表示该路径名以一个目录名为最后一项。
	if (!namelen)			/* 对应于'/usr/'等情况 */
		return dir;
// 在返回的顶层目录中寻找指定文件名的目录项的i 节点。因为如果最后也是一个目录名，但其后没
// 有加'/'，则不会返回该最后目录的i 节点！比如：/var/log/httpd，将只返回log/目录的i 节点。
// 因此dir_namei()将不以'/'结束的最后一个名字当作一个文件名来看待。因此这里需要单独对这种
// 情况使用寻找目录项i 节点函数find_entry()进行处理。
	bh = find_entry(&dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		return NULL;
	}
// 取该目录项的i 节点号和目录的设备号，并释放包含该目录项的高速缓冲区以及目录i 节点。
	inr = de->inode;
	dev = dir->i_dev;
	brelse(bh);
	iput(dir);
// 取对应节号的i 节点，修改其被访问时间为当前时间，并置已修改标志。最后返回该i 节点指针。
	dir=iget(dev,inr);
	if (dir) {
		dir->i_atime=CURRENT_TIME;
		dir->i_dirt=1;
	}
	return dir;
}

/*
 *	open_namei()
 *
 * open()所使用的namei 函数- 这其实几乎是完整的打开文件程序。
 */
//// 文件打开namei 函数。
// 参数：pathname - 文件路径名；flag - 文件打开标志；mode - 文件访问许可属性；
// 返回：成功返回0，否则返回出错码；res_inode - 返回的对应文件路径名的的i 节点指针。
int open_namei(const char * pathname, int flag, int mode,
	struct m_inode ** res_inode)
{
	const char * basename;
	int inr,dev,namelen;
	struct m_inode * dir, *inode;
	struct buffer_head * bh;
	struct dir_entry * de;

// 如果文件访问许可模式标志是只读(0)，但文件截0 标志O_TRUNC 却置位了，则改为只写标志。
	if ((flag & O_TRUNC) && !(flag & O_ACCMODE))
		flag |= O_WRONLY;
// 使用进程的文件访问许可屏蔽码，屏蔽掉给定模式中的相应位，并添上普通文件标志。
	mode &= 0777 & ~current->umask;
	mode |= I_REGULAR;
// 根据路径名寻找到对应的i 节点，以及最顶端文件名及其长度。
	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return -ENOENT;
// 如果最顶端文件名长度为0(例如'/usr/'这种路径名的情况)，那么若打开操作不是创建、截0，
// 则表示打开一个目录名，直接返回该目录的i 节点，并退出。
	if (!namelen) {			/* special case: '/usr/' etc */
		if (!(flag & (O_ACCMODE|O_CREAT|O_TRUNC))) {
			*res_inode=dir;
			return 0;
		}
// 否则释放该i 节点，返回出错码。
		iput(dir);
		return -EISDIR;
	}
// 在dir 节点对应的目录中取文件名对应的目录项结构de 和该目录项所在的高速缓冲区。
	bh = find_entry(&dir,basename,namelen,&de);
// 如果该高速缓冲指针为NULL，则表示没有找到对应文件名的目录项，因此只可能是创建文件操作。
	if (!bh) {
// 如果不是创建文件，则释放该目录的i 节点，返回出错号退出。
		if (!(flag & O_CREAT)) {
			iput(dir);
			return -ENOENT;
		}
// 如果用户在该目录没有写的权力，则释放该目录的i 节点，返回出错号退出。
		if (!permission(dir,MAY_WRITE)) {
			iput(dir);
			return -EACCES;
		}
// 在目录节点对应的设备上申请一个新i 节点，若失败，则释放目录的i 节点，并返回没有空间出错码。
		inode = new_inode(dir->i_dev);
		if (!inode) {
			iput(dir);
			return -ENOSPC;
		}
// 否则使用该新i 节点，对其进行初始设置：置节点的用户id；对应节点访问模式；置已修改标志。
		inode->i_uid = current->euid;
		inode->i_mode = mode;
		inode->i_dirt = 1;
// 然后在指定目录dir 中添加一新目录项。
		bh = add_entry(dir,basename,namelen,&de);
// 如果返回的应该含有新目录项的高速缓冲区指针为NULL，则表示添加目录项操作失败。于是将该
// 新i 节点的引用连接计数减1；并释放该i 节点与目录的i 节点，返回出错码，退出。
		if (!bh) {
			inode->i_nlinks--;
			iput(inode);
			iput(dir);
			return -ENOSPC;
		}
// 初始设置该新目录项：置i 节点号为新申请到的i 节点的号码；并置高速缓冲区已修改标志。然后
// 释放该高速缓冲区，释放目录的i 节点。返回新目录项的i 节点指针，退出。
		de->inode = inode->i_num;
		bh->b_dirt = 1;
		brelse(bh);
		iput(dir);
		*res_inode = inode;
		return 0;
	}
// 若上面在目录中取文件名对应的目录项结构操作成功(也即bh 不为NULL)，取出该目录项的i 节点号
// 和其所在的设备号，并释放该高速缓冲区以及目录的i 节点。
	inr = de->inode;
	dev = dir->i_dev;
	brelse(bh);
	iput(dir);
// 如果独占使用标志O_EXCL 置位，则返回文件已存在出错码，退出。
	if (flag & O_EXCL)
		return -EEXIST;
// 如果取该目录项对应i 节点的操作失败，则返回访问出错码，退出。
	if (!(inode=iget(dev,inr)))
		return -EACCES;
// 若该i 节点是一个目录的节点并且访问模式是只读或只写，或者没有访问的许可权限，则释放该i
// 节点，返回访问权限出错码，退出。
	if ((S_ISDIR(inode->i_mode) && (flag & O_ACCMODE)) ||
	    !permission(inode,ACC_MODE(flag))) {
		iput(inode);
		return -EPERM;
	}
// 更新该i 节点的访问时间字段为当前时间。
	inode->i_atime = CURRENT_TIME;
// 如果设立了截0 标志，则将该i 节点的文件长度截为0。
	if (flag & O_TRUNC)
		truncate(inode);
// 最后返回该目录项i 节点的指针，并返回0（成功）。
	*res_inode = inode;
	return 0;
}

//// 系统调用函数- 创建一个特殊文件或普通文件节点(node)。
// 创建名称为filename，由mode 和dev 指定的文件系统节点(普通文件、设备特殊文件或命名管道)。
// 参数：filename - 路径名；mode - 指定使用许可以及所创建节点的类型；dev - 设备号。
// 返回：成功则返回0，否则返回出错码。
int sys_mknod(const char * filename, int mode, int dev)
{
	const char * basename;
	int namelen;
	struct m_inode * dir, * inode;
	struct buffer_head * bh;
	struct dir_entry * de;
	
// 如果不是超级用户，则返回访问许可出错码。
	if (!suser())
		return -EPERM;
// 如果找不到对应路径名目录的i 节点，则返回出错码。
	if (!(dir = dir_namei(filename,&namelen,&basename)))
		return -ENOENT;
// 如果最顶端的文件名长度为0，则说明给出的路径名最后没有指定文件名，释放该目录i 节点，返回
// 出错码，退出。
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
// 如果在该目录中没有写的权限，则释放该目录的i 节点，返回访问许可出错码，退出。
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
// 如果对应路径名上最后的文件名的目录项已经存在，则释放包含该目录项的高速缓冲区，释放目录
// 的i 节点，返回文件已经存在出错码，退出。
	bh = find_entry(&dir,basename,namelen,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
// 申请一个新的i 节点，如果不成功，则释放目录的i 节点，返回无空间出错码，退出。
	inode = new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
// 设置该i 节点的属性模式。如果要创建的是块设备文件或者是字符设备文件，则令i 节点的直接块
// 指针0 等于设备号。
	inode->i_mode = mode;
	if (S_ISBLK(mode) || S_ISCHR(mode))
		inode->i_zone[0] = dev;
// 设置该i 节点的修改时间、访问时间为当前时间。
	inode->i_mtime = inode->i_atime = CURRENT_TIME;
	inode->i_dirt = 1;
// 在目录中新添加一个目录项，如果失败(包含该目录项的高速缓冲区指针为NULL)，则释放目录的
// i 节点；所申请的i 节点引用连接计数复位，并释放该i 节点。返回出错码，退出。
	bh = add_entry(dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		inode->i_nlinks=0;
		iput(inode);
		return -ENOSPC;
	}
// 令该目录项的i 节点字段等于新i 节点号，置高速缓冲区已修改标志，释放目录和新的i 节点，
// 释放高速缓冲区，最后返回0(成功)。
	de->inode = inode->i_num;
	bh->b_dirt = 1;
	iput(dir);
	iput(inode);
	brelse(bh);
	return 0;
}

//// 系统调用函数- 创建目录。
// 参数：pathname - 路径名；mode - 目录使用的权限属性。
// 返回：成功则返回0，否则返回出错码。
int sys_mkdir(const char * pathname, int mode)
{
	const char * basename;
	int namelen;
	struct m_inode * dir, * inode;
	struct buffer_head * bh, *dir_block;
	struct dir_entry * de;

// 如果不是超级用户，则返回访问许可出错码。
	if (!suser())
		return -EPERM;
// 如果找不到对应路径名目录的i 节点，则返回出错码。
	if (!(dir = dir_namei(pathname,&namelen,&basename)))
		return -ENOENT;
// 如果最顶端的文件名长度为0，则说明给出的路径名最后没有指定文件名，释放该目录i 节点，返回
// 出错码，退出。
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
// 如果在该目录中没有写的权限，则释放该目录的i 节点，返回访问许可出错码，退出。
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
// 如果对应路径名上最后的文件名的目录项已经存在，则释放包含该目录项的高速缓冲区，释放目录
// 的i 节点，返回文件已经存在出错码，退出。
	bh = find_entry(&dir,basename,namelen,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		return -EEXIST;
	}
// 申请一个新的i 节点，如果不成功，则释放目录的i 节点，返回无空间出错码，退出。
	inode = new_inode(dir->i_dev);
	if (!inode) {
		iput(dir);
		return -ENOSPC;
	}
// 置该新i 节点对应的文件长度为32(一个目录项的大小)，置节点已修改标志，以及节点的修改时间
// 和访问时间。
	inode->i_size = 32;
	inode->i_dirt = 1;
	inode->i_mtime = inode->i_atime = CURRENT_TIME;
// 为该i 节点申请一磁盘块，并令节点第一个直接块指针等于该块号。如果申请失败，则释放对应目录
// 的i 节点；复位新申请的i 节点连接计数；释放该新的i 节点，返回没有空间出错码，退出。
	if (!(inode->i_zone[0]=new_block(inode->i_dev))) {
		iput(dir);
		inode->i_nlinks--;
		iput(inode);
		return -ENOSPC;
	}
// 置该新的i 节点已修改标志。
	inode->i_dirt = 1;
// 读新申请的磁盘块。若出错，则释放对应目录的i 节点；释放申请的磁盘块；复位新申请的i 节点
// 连接计数；释放该新的i 节点，返回没有空间出错码，退出。
	if (!(dir_block=bread(inode->i_dev,inode->i_zone[0]))) {
		iput(dir);
		free_block(inode->i_dev,inode->i_zone[0]);
		inode->i_nlinks--;
		iput(inode);
		return -ERROR;
	}
// 令de 指向目录项数据块，置该目录项的i 节点号字段等于新申请的i 节点号，名字字段等于"."。
	de = (struct dir_entry *) dir_block->b_data;
	de->inode=inode->i_num;
	strcpy(de->name,".");
// 然后de 指向下一个目录项结构，该结构用于存放上级目录的节点号和名字".."。
	de++;
	de->inode = dir->i_num;
	strcpy(de->name,"..");
	inode->i_nlinks = 2;
// 然后设置该高速缓冲区已修改标志，并释放该缓冲区。
	dir_block->b_dirt = 1;
	brelse(dir_block);
// 初始化设置新i 节点的模式字段，并置该i 节点已修改标志。
	inode->i_mode = I_DIRECTORY | (mode & 0777 & ~current->umask);
	inode->i_dirt = 1;
// 在目录中新添加一个目录项，如果失败(包含该目录项的高速缓冲区指针为NULL)，则释放目录的
// i 节点；所申请的i 节点引用连接计数复位，并释放该i 节点。返回出错码，退出。
	bh = add_entry(dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		free_block(inode->i_dev,inode->i_zone[0]);
		inode->i_nlinks=0;
		iput(inode);
		return -ENOSPC;
	}
// 令该目录项的i 节点字段等于新i 节点号，置高速缓冲区已修改标志，释放目录和新的i 节点，释放
// 高速缓冲区，最后返回0(成功)。
	de->inode = inode->i_num;
	bh->b_dirt = 1;
	dir->i_nlinks++;
	dir->i_dirt = 1;
	iput(dir);
	iput(inode);
	brelse(bh);
	return 0;
}

/*
 * 用于检查指定的目录是否为空的子程序(用于rmdir 系统调用函数)。
 */
//// 检查指定目录是否是空的。
// 参数：inode - 指定目录的i 节点指针。
// 返回：0 - 是空的；1 - 不空。
static int empty_dir(struct m_inode * inode)
{
	int nr,block;
	int len;
	struct buffer_head * bh;
	struct dir_entry * de;

// 计算指定目录中现有目录项的个数(应该起码有2 个，即"."和".."两个文件目录项)。
	len = inode->i_size / sizeof (struct dir_entry);
// 如果目录项个数少于2 个或者该目录i 节点的第1 个直接块没有指向任何磁盘块号，或者相应磁盘
// 块读不出，则显示警告信息“设备dev 上目录错”，返回0(失败)。
	if (len<2 || !inode->i_zone[0] ||
	    !(bh=bread(inode->i_dev,inode->i_zone[0]))) {
	    	printk("warning - bad directory on dev %04x\n",inode->i_dev);
		return 0;
	}
// 让de 指向含有读出磁盘块数据的高速缓冲区中第1 项目录项。
	de = (struct dir_entry *) bh->b_data;
// 如果第1 个目录项的i 节点号字段值不等于该目录的i 节点号，或者第2 个目录项的i 节点号字段
// 为零，或者两个目录项的名字字段不分别等于"."和".."，则显示出错警告信息“设备dev 上目录错”
// 并返回0。
	if (de[0].inode != inode->i_num || !de[1].inode || 
	    strcmp(".",de[0].name) || strcmp("..",de[1].name)) {
	    	printk("warning - bad directory on dev %04x\n",inode->i_dev);
		return 0;
	}
// 令nr 等于目录项序号；de 指向第三个目录项。
	nr = 2;
	de += 2;
// 循环检测该目录中所有的目录项(len-2 个)，看有没有目录项的i 节点号字段不为0(被使用)。
	while (nr<len) {
// 如果该块磁盘块中的目录项已经检测完，则释放该磁盘块的高速缓冲区，读取下一块含有目录项的
// 磁盘块。若相应块没有使用(或已经不用，如文件已经删除等)，则继续读下一块，若读不出，则出
// 错，返回0。否则让de 指向读出块的首个目录项。
		if ((void *) de >= (void *) (bh->b_data+BLOCK_SIZE)) {
			brelse(bh);
			block=bmap(inode,nr/DIR_ENTRIES_PER_BLOCK);
			if (!block) {
				nr += DIR_ENTRIES_PER_BLOCK;
				continue;
			}
			if (!(bh=bread(inode->i_dev,block)))
				return 0;
			de = (struct dir_entry *) bh->b_data;
		}
// 如果该目录项的i 节点号字段不等于0，则表示该目录项目前正被使用，则释放该高速缓冲区，
// 返回0，退出。
		if (de->inode) {
			brelse(bh);
			return 0;
		}
// 否则，若还没有查询完该目录中的所有目录项，则继续检测。
		de++;
		nr++;
	}
// 到这里说明该目录中没有找到已用的目录项(当然除了头两个以外)，则返回缓冲区，返回1。
	brelse(bh);
	return 1;
}

//// 系统调用函数- 删除指定名称的目录。
// 参数： name - 目录名(路径名)。
// 返回：返回0 表示成功，否则返回出错号。
int sys_rmdir(const char * name)
{
	const char * basename;
	int namelen;
	struct m_inode * dir, * inode;
	struct buffer_head * bh;
	struct dir_entry * de;

// 如果不是超级用户，则返回访问许可出错码。
	if (!suser())
		return -EPERM;
// 如果找不到对应路径名目录的i 节点，则返回出错码。
	if (!(dir = dir_namei(name,&namelen,&basename)))
		return -ENOENT;
// 如果最顶端的文件名长度为0，则说明给出的路径名最后没有指定文件名，释放该目录i 节点，返回
// 出错码，退出。
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
// 如果在该目录中没有写的权限，则释放该目录的i 节点，返回访问许可出错码，退出。
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
// 如果对应路径名上最后的文件名的目录项不存在，则释放包含该目录项的高速缓冲区，释放目录
// 的i 节点，返回文件已经存在出错码，退出。否则dir 是包含要被删除目录名的目录i 节点，de
// 是要被删除目录的目录项结构。
	bh = find_entry(&dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		return -ENOENT;
	}
// 取该目录项指明的i 节点。若出错则释放目录的i 节点，并释放含有目录项的高速缓冲区，返回
// 出错号。
	if (!(inode = iget(dir->i_dev, de->inode))) {
		iput(dir);
		brelse(bh);
		return -EPERM;
	}
// 若该目录设置了受限删除标志并且进程的有效用户id 不等于该i 节点的用户id，则表示没有权限删
// 除该目录，于是释放包含要删除目录名的目录i 节点和该要删除目录的i 节点，释放高速缓冲区，
// 返回出错码。
	if ((dir->i_mode & S_ISVTX) && current->euid &&
	    inode->i_uid != current->euid) {
		iput(dir);
		iput(inode);
		brelse(bh);
		return -EPERM;
	}
// 如果要被删除的目录项的i 节点的设备号不等于包含该目录项的目录的设备号，或者该被删除目录的
// 引用连接计数大于1(表示有符号连接等)，则不能删除该目录，于是释放包含要删除目录名的目录
// i 节点和该要删除目录的i 节点，释放高速缓冲区，返回出错码。
	if (inode->i_dev != dir->i_dev || inode->i_count>1) {
		iput(dir);
		iput(inode);
		brelse(bh);
		return -EPERM;
	}
// 如果要被删除目录的目录项i 节点的节点号等于包含该需删除目录的i 节点号，则表示试图删除"."
// 目录。于是释放包含要删除目录名的目录i 节点和该要删除目录的i 节点，释放高速缓冲区，返回
// 出错码。
	if (inode == dir) {	/* 我们不可以删除"."，但可以删除"../dir"*/
		iput(inode);
		iput(dir);
		brelse(bh);
		return -EPERM;
	}
// 若要被删除的目录的i 节点的属性表明这不是一个目录，则释放包含要删除目录名的目录i 节点和
// 该要删除目录的i 节点，释放高速缓冲区，返回出错码。
	if (!S_ISDIR(inode->i_mode)) {
		iput(inode);
		iput(dir);
		brelse(bh);
		return -ENOTDIR;
	}
// 若该需被删除的目录不空，则释放包含要删除目录名的目录i 节点和该要删除目录的i 节点，释放
// 高速缓冲区，返回出错码。
	if (!empty_dir(inode)) {
		iput(inode);
		iput(dir);
		brelse(bh);
		return -ENOTEMPTY;
	}
// 若该需被删除目录的i 节点的连接数不等于2，则显示警告信息。
	if (inode->i_nlinks != 2)
		printk("empty directory has nlink!=2 (%d)",inode->i_nlinks);
// 置该需被删除目录的目录项的i 节点号字段为0，表示该目录项不再使用，并置含有该目录项的高速
// 缓冲区已修改标志，并释放该缓冲区。
	de->inode = 0;
	bh->b_dirt = 1;
	brelse(bh);
// 置被删除目录的i 节点的连接数为0，并置i 节点已修改标志。
	inode->i_nlinks=0;
	inode->i_dirt=1;
// 将包含被删除目录名的目录的i 节点引用计数减1，修改其改变时间和修改时间为当前时间，并置
// 该节点已修改标志。
	dir->i_nlinks--;
	dir->i_ctime = dir->i_mtime = CURRENT_TIME;
	dir->i_dirt=1;
// 最后释放包含要删除目录名的目录i 节点和该要删除目录的i 节点，返回0(成功)。
	iput(dir);
	iput(inode);
	return 0;
}

//// 系统调用函数- 删除文件名以及可能也删除其相关的文件。
// 从文件系统删除一个名字。如果是一个文件的最后一个连接，并且没有进程正打开该文件，则该文件
// 也将被删除，并释放所占用的设备空间。
// 参数：name - 文件名。
// 返回：成功则返回0，否则返回出错号。
int sys_unlink(const char * name)
{
	const char * basename;
	int namelen;
	struct m_inode * dir, * inode;
	struct buffer_head * bh;
	struct dir_entry * de;

// 如果找不到对应路径名目录的i 节点，则返回出错码。
	if (!(dir = dir_namei(name,&namelen,&basename)))
		return -ENOENT;
// 如果最顶端的文件名长度为0，则说明给出的路径名最后没有指定文件名，释放该目录i 节点，
// 返回出错码，退出。
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
// 如果在该目录中没有写的权限，则释放该目录的i 节点，返回访问许可出错码，退出。
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		return -EPERM;
	}
// 如果对应路径名上最后的文件名的目录项不存在，则释放包含该目录项的高速缓冲区，释放目录
// 的i 节点，返回文件已经存在出错码，退出。否则dir 是包含要被删除目录名的目录i 节点，de
// 是要被删除目录的目录项结构。
	bh = find_entry(&dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		return -ENOENT;
	}
// 取该目录项指明的i 节点。若出错则释放目录的i 节点，并释放含有目录项的高速缓冲区，
// 返回出错号。
	if (!(inode = iget(dir->i_dev, de->inode))) {
		iput(dir);
		brelse(bh);
		return -ENOENT;
	}
// 如果该目录设置了受限删除标志并且用户不是超级用户，并且进程的有效用户id 不等于被删除文件
// 名i 节点的用户id，并且进程的有效用户id 也不等于目录i 节点的用户id，则没有权限删除该文件
// 名。则释放该目录i 节点和该文件名目录项的i 节点，释放包含该目录项的缓冲区，返回出错号。
	if ((dir->i_mode & S_ISVTX) && !suser() &&
	    current->euid != inode->i_uid &&
	    current->euid != dir->i_uid) {
		iput(dir);
		iput(inode);
		brelse(bh);
		return -EPERM;
	}
// 如果该指定文件名是一个目录，则也不能删除，释放该目录i 节点和该文件名目录项的i 节点，
// 释放包含该目录项的缓冲区，返回出错号。
	if (S_ISDIR(inode->i_mode)) {
		iput(inode);
		iput(dir);
		brelse(bh);
		return -EPERM;
	}
// 如果该i 节点的连接数已经为0，则显示警告信息，修正其为1。
	if (!inode->i_nlinks) {
		printk("Deleting nonexistent file (%04x:%d), %d\n",
			inode->i_dev,inode->i_num,inode->i_nlinks);
		inode->i_nlinks=1;
	}
// 将该文件名的目录项中的i 节点号字段置为0，表示释放该目录项，并设置包含该目录项的缓冲区
// 已修改标志，释放该高速缓冲区。
	de->inode = 0;
	bh->b_dirt = 1;
	brelse(bh);
// 该i 节点的连接数减1，置已修改标志，更新改变时间为当前时间。最后释放该i 节点和目录的i 节
// 点，返回0(成功)。
	inode->i_nlinks--;
	inode->i_dirt = 1;
	inode->i_ctime = CURRENT_TIME;
	iput(inode);
	iput(dir);
	return 0;
}

//// 系统调用函数- 为文件建立一个文件名。
// 为一个已经存在的文件创建一个新连接(也称为硬连接- hard link)。
// 参数：oldname - 原路径名；newname - 新的路径名。
// 返回：若成功则返回0，否则返回出错号。
int sys_link(const char * oldname, const char * newname)
{
	struct dir_entry * de;
	struct m_inode * oldinode, * dir;
	struct buffer_head * bh;
	const char * basename;
	int namelen;

// 取原文件路径名对应的i 节点oldinode。如果为0，则表示出错，返回出错号。
	oldinode=namei(oldname);
	if (!oldinode)
		return -ENOENT;
// 如果原路径名对应的是一个目录名，则释放该i 节点，返回出错号。
	if (S_ISDIR(oldinode->i_mode)) {
		iput(oldinode);
		return -EPERM;
	}
// 查找新路径名的最顶层目录的i 节点，并返回最后的文件名及其长度。如果目录的i 节点没有找到，
// 则释放原路径名的i 节点，返回出错号。
	dir = dir_namei(newname,&namelen,&basename);
	if (!dir) {
		iput(oldinode);
		return -EACCES;
	}
// 如果新路径名中不包括文件名，则释放原路径名i 节点和新路径名目录的i 节点，返回出错号。
	if (!namelen) {
		iput(oldinode);
		iput(dir);
		return -EPERM;
	}
// 如果新路径名目录的设备号与原路径名的设备号不一样，则也不能建立连接，于是释放新路径名
// 目录的i 节点和原路径名的i 节点，返回出错号。
	if (dir->i_dev != oldinode->i_dev) {
		iput(dir);
		iput(oldinode);
		return -EXDEV;
	}
// 如果用户没有在新目录中写的权限，则也不能建立连接，于是释放新路径名目录的i 节点
// 和原路径名的i 节点，返回出错号。
	if (!permission(dir,MAY_WRITE)) {
		iput(dir);
		iput(oldinode);
		return -EACCES;
	}
// 查询该新路径名是否已经存在，如果存在，则也不能建立连接，于是释放包含该已存在目录项的
// 高速缓冲区，释放新路径名目录的i 节点和原路径名的i 节点，返回出错号。
	bh = find_entry(&dir,basename,namelen,&de);
	if (bh) {
		brelse(bh);
		iput(dir);
		iput(oldinode);
		return -EEXIST;
	}
// 在新目录中添加一个目录项。若失败则释放该目录的i 节点和原路径名的i 节点，返回出错号。
	bh = add_entry(dir,basename,namelen,&de);
	if (!bh) {
		iput(dir);
		iput(oldinode);
		return -ENOSPC;
	}
// 否则初始设置该目录项的i 节点号等于原路径名的i 节点号，并置包含该新添目录项的高速缓冲区
// 已修改标志，释放该缓冲区，释放目录的i 节点。
	de->inode = oldinode->i_num;
	bh->b_dirt = 1;
	brelse(bh);
	iput(dir);
// 将原节点的应用计数加1，修改其改变时间为当前时间，并设置i 节点已修改标志，最后释放原
// 路径名的i 节点，并返回0(成功)。
	oldinode->i_nlinks++;
	oldinode->i_ctime = CURRENT_TIME;
	oldinode->i_dirt = 1;
	iput(oldinode);
	return 0;
}
