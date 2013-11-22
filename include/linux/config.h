#ifndef _CONFIG_H
#define _CONFIG_H

/*
* The root-device is no longer hard-coded. You can change the default
* root-device by changing the line ROOT_DEV = XXX in boot/bootsect.s
*/
/*
* 根文件系统设备已不再是硬编码的了。通过修改boot/bootsect.s 文件中行
* ROOT_DEV = XXX，你可以改变根设备的默认设置值。
*/

/*
* define your keyboard here -
* KBD_FINNISH for Finnish keyboards
* KBD_US for US-type
* KBD_GR for German keyboards
* KBD_FR for Frech keyboard
*/
/*
* 在这里定义你的键盘类型 -
* KBD_FINNISH 是芬兰键盘。
* KBD_US 是美式键盘。
* KBD_GR 是德式键盘。
* KBD_FR 是法式键盘。
*/
#define KBD_US 
/*#define KBD_GR */
/*#define KBD_FR */
/*#define KBD_FINNISH*/

/*
* Normally, Linux can get the drive parameters from the BIOS at
* startup, but if this for some unfathomable reason fails, you'd
* be left stranded. For this case, you can define HD_TYPE, which
* contains all necessary info on your harddisk.
*
* The HD_TYPE macro should look like this:
*
* #define HD_TYPE { head, sect, cyl, wpcom, lzone, ctl}
*
* In case of two harddisks, the info should be sepatated by
* commas:
*
* #define HD_TYPE { h,s,c,wpcom,lz,ctl },{ h,s,c,wpcom,lz,ctl }
*/
/*
* 通常，Linux 能够在启动时从BIOS 中获取驱动器德参数，但是若由于未知原因
* 而没有得到这些参数时，会使程序束手无策。对于这种情况，你可以定义HD_TYPE，
* 其中包括硬盘的所有信息。
*
* HD_TYPE 宏应该象下面这样的形式：
*
* #define HD_TYPE { head, sect, cyl, wpcom, lzone, ctl}
*
* 对于有两个硬盘的情况，参数信息需用逗号分开：
*
* #define HD_TYPE { h,s,c,wpcom,lz,ctl }, {h,s,c,wpcom,lz,ctl }
*/
/*
This is an example, two drives, first is type 2, second is type 3:

#define HD_TYPE { 4,17,615,300,615,8 }, { 6,17,615,300,615,0 }

NOTE: ctl is 0 for all drives with heads<=8, and ctl=8 for drives
with more than 8 heads.

If you want the BIOS to tell what kind of drive you have, just
leave HD_TYPE undefined. This is the normal thing to do.
*/
/*
* 下面是一个例子，两个硬盘，第1 个是类型2，第2 个是类型3：
*
11.23 fdreg.h 头文件
* #define HD_TYPE { 4,17,615,300,615,8 }, {6,17,615,300,615,0 }
*
* 注意：对应所有硬盘，若其磁头数<=8，则ctl 等于0，若磁头数多于8 个，
* 则ctl=8。
*
* 如果你想让BIOS 给出硬盘的类型，那么只需不定义HD_TYPE。这是默认操作。
*/

#endif
