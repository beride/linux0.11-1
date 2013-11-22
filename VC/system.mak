# Microsoft Developer Studio Generated NMAKE File, Based on system.dsp
!IF "$(CFG)" == ""
CFG=system - Win32 Release
!MESSAGE No configuration specified. Defaulting to system - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "system - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "system.mak" CFG="system - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "system - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

MTL=midl.exe
OUTDIR=.\Release
INTDIR=.\Release

ALL : ".\Release\mm\page.o" ".\Release\mm\memory.o" ".\Release\lib\write.o" ".\Release\lib\wait.o" ".\Release\lib\string.o" ".\Release\lib\setsid.o" ".\Release\lib\open.o" ".\Release\lib\malloc.o" ".\Release\lib\execve.o" ".\Release\lib\errno.o" ".\Release\lib\dup.o" ".\Release\lib\ctype.o" ".\Release\lib\close.o" ".\Release\lib\_exit.o" ".\Release\kernel\vsprintf.o" ".\Release\kernel\traps.o" ".\Release\kernel\system_call.o" ".\Release\kernel\sys.o" ".\Release\kernel\signal.o" ".\Release\kernel\sched.o" ".\Release\kernel\printk.o" ".\Release\kernel\panic.o" ".\Release\kernel\mktime.o" ".\Release\kernel\math\math_emulate.o" ".\Release\kernel\fork.o" ".\Release\kernel\exit.o" ".\Release\kernel\chr_drv\tty_ioctl.o" ".\Release\kernel\chr_drv\tty_io.o" ".\Release\kernel\chr_drv\serial.o" ".\Release\kernel\chr_drv\rs_io.o" ".\Release\kernel\chr_drv\keyboard.o" ".\Release\kernel\chr_drv\console.o" ".\Release\kernel\blk_drv\ramdisk.o" ".\Release\kernel\blk_drv\ll_rw_blk.o" ".\Release\kernel\blk_drv\hd.o" ".\Release\kernel\blk_drv\floppy.o" ".\Release\kernel\asm.o" ".\Release\init\main.o" ".\Release\fs\truncate.o" ".\Release\fs\super.o" ".\Release\fs\stat.o" ".\Release\fs\read_write.o"\
 ".\Release\fs\pipe.o" ".\Release\fs\open.o" ".\Release\fs\namei.o" ".\Release\fs\ioctl.o" ".\Release\fs\inode.o" ".\Release\fs\file_table.o" ".\Release\fs\file_dev.o" ".\Release\fs\fcntl.o" ".\Release\fs\exec.o" ".\Release\fs\char_dev.o" ".\Release\fs\buffer.o" ".\Release\fs\block_dev.o" ".\Release\fs\bitmap.o" ".\Release\boot\head.o" 


CLEAN :
	-@erase 
	-@erase ".\Release\boot\head.o"
	-@erase ".\Release\fs\bitmap.o"
	-@erase ".\Release\fs\block_dev.o"
	-@erase ".\Release\fs\buffer.o"
	-@erase ".\Release\fs\char_dev.o"
	-@erase ".\Release\fs\exec.o"
	-@erase ".\Release\fs\fcntl.o"
	-@erase ".\Release\fs\file_dev.o"
	-@erase ".\Release\fs\file_table.o"
	-@erase ".\Release\fs\inode.o"
	-@erase ".\Release\fs\ioctl.o"
	-@erase ".\Release\fs\namei.o"
	-@erase ".\Release\fs\open.o"
	-@erase ".\Release\fs\pipe.o"
	-@erase ".\Release\fs\read_write.o"
	-@erase ".\Release\fs\stat.o"
	-@erase ".\Release\fs\super.o"
	-@erase ".\Release\fs\truncate.o"
	-@erase ".\Release\init\main.o"
	-@erase ".\Release\kernel\asm.o"
	-@erase ".\Release\kernel\blk_drv\floppy.o"
	-@erase ".\Release\kernel\blk_drv\hd.o"
	-@erase ".\Release\kernel\blk_drv\ll_rw_blk.o"
	-@erase ".\Release\kernel\blk_drv\ramdisk.o"
	-@erase ".\Release\kernel\chr_drv\console.o"
	-@erase ".\Release\kernel\chr_drv\keyboard.o"
	-@erase ".\Release\kernel\chr_drv\rs_io.o"
	-@erase ".\Release\kernel\chr_drv\serial.o"
	-@erase ".\Release\kernel\chr_drv\tty_io.o"
	-@erase ".\Release\kernel\chr_drv\tty_ioctl.o"
	-@erase ".\Release\kernel\exit.o"
	-@erase ".\Release\kernel\fork.o"
	-@erase ".\Release\kernel\math\math_emulate.o"
	-@erase ".\Release\kernel\mktime.o"
	-@erase ".\Release\kernel\panic.o"
	-@erase ".\Release\kernel\printk.o"
	-@erase ".\Release\kernel\sched.o"
	-@erase ".\Release\kernel\signal.o"
	-@erase ".\Release\kernel\sys.o"
	-@erase ".\Release\kernel\system_call.o"
	-@erase ".\Release\kernel\traps.o"
	-@erase ".\Release\kernel\vsprintf.o"
	-@erase ".\Release\lib\_exit.o"
	-@erase ".\Release\lib\close.o"
	-@erase ".\Release\lib\ctype.o"
	-@erase ".\Release\lib\dup.o"
	-@erase ".\Release\lib\errno.o"
	-@erase ".\Release\lib\execve.o"
	-@erase ".\Release\lib\malloc.o"
	-@erase ".\Release\lib\open.o"
	-@erase ".\Release\lib\setsid.o"
	-@erase ".\Release\lib\string.o"
	-@erase ".\Release\lib\wait.o"
	-@erase ".\Release\lib\write.o"
	-@erase ".\Release\mm\memory.o"
	-@erase ".\Release\mm\page.o"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : ".\Release\mm\page.o" ".\Release\mm\memory.o" ".\Release\lib\write.o" ".\Release\lib\wait.o" ".\Release\lib\string.o" ".\Release\lib\setsid.o" ".\Release\lib\open.o" ".\Release\lib\malloc.o" ".\Release\lib\execve.o" ".\Release\lib\errno.o" ".\Release\lib\dup.o" ".\Release\lib\ctype.o" ".\Release\lib\close.o" ".\Release\lib\_exit.o" ".\Release\kernel\vsprintf.o" ".\Release\kernel\traps.o" ".\Release\kernel\system_call.o" ".\Release\kernel\sys.o" ".\Release\kernel\signal.o" ".\Release\kernel\sched.o" ".\Release\kernel\printk.o" ".\Release\kernel\panic.o" ".\Release\kernel\mktime.o" ".\Release\kernel\math\math_emulate.o" ".\Release\kernel\fork.o" ".\Release\kernel\exit.o" ".\Release\kernel\chr_drv\tty_ioctl.o" ".\Release\kernel\chr_drv\tty_io.o" ".\Release\kernel\chr_drv\serial.o" ".\Release\kernel\chr_drv\rs_io.o" ".\Release\kernel\chr_drv\keyboard.o" ".\Release\kernel\chr_drv\console.o" ".\Release\kernel\blk_drv\ramdisk.o" ".\Release\kernel\blk_drv\ll_rw_blk.o" ".\Release\kernel\blk_drv\hd.o" ".\Release\kernel\blk_drv\floppy.o" ".\Release\kernel\asm.o" ".\Release\init\main.o" ".\Release\fs\truncate.o" ".\Release\fs\super.o" ".\Release\fs\stat.o"\
 ".\Release\fs\read_write.o" ".\Release\fs\pipe.o" ".\Release\fs\open.o" ".\Release\fs\namei.o" ".\Release\fs\ioctl.o" ".\Release\fs\inode.o" ".\Release\fs\file_table.o" ".\Release\fs\file_dev.o" ".\Release\fs\fcntl.o" ".\Release\fs\exec.o" ".\Release\fs\char_dev.o" ".\Release\fs\buffer.o" ".\Release\fs\block_dev.o" ".\Release\fs\bitmap.o" ".\Release\boot\head.o" 
   bin\ld -T .\bin\ld_script.txt -s -x --file-alignment=1 --section-alignment=1 -Ttext=0 --image-base=0 -nostdlib -Map=.\release\system.map -o .\release\system   ".\Release\boot\head.o" ".\Release\init\main.o"  ".\Release\kernel\exit.o"  ".\Release\kernel\fork.o"  ".\Release\kernel\math\math_emulate.o"  ".\Release\kernel\mktime.o"  ".\Release\kernel\panic.o"  ".\Release\kernel\printk.o" ".\Release\kernel\sched.o"   ".\Release\kernel\signal.o" ".\Release\kernel\sys.o"  ".\Release\kernel\system_call.o"  ".\Release\kernel\traps.o"  ".\Release\kernel\vsprintf.o" ".\Release\kernel\asm.o"   ".\Release\kernel\blk_drv\floppy.o" ".\Release\kernel\blk_drv\hd.o"   ".\Release\kernel\blk_drv\ll_rw_blk.o" ".\Release\kernel\blk_drv\ramdisk.o"   ".\Release\kernel\chr_drv\console.o" ".\Release\kernel\chr_drv\keyboard.o"   ".\Release\kernel\chr_drv\rs_io.o" ".\Release\kernel\chr_drv\serial.o"   ".\Release\kernel\chr_drv\tty_io.o" ".\Release\kernel\chr_drv\tty_ioctl.o"   ".\Release\mm\memory.o" ".\Release\mm\page.o" ".\Release\fs\bitmap.o"   ".\Release\fs\block_dev.o" ".\Release\fs\buffer.o" ".\Release\fs\char_dev.o"   ".\Release\fs\exec.o"  ".\Release\fs\fcntl.o"       ".\Release\fs\file_dev.o"\
  ".\Release\fs\file_table.o"   ".\Release\fs\inode.o" ".\Release\fs\ioctl.o"   ".\Release\fs\namei.o" ".\Release\fs\open.o" ".\Release\fs\pipe.o"   ".\Release\fs\read_write.o" ".\Release\fs\stat.o" ".\Release\fs\super.o"  ".\Release\fs\truncate.o" ".\Release\lib\_exit.o" ".\Release\lib\close.o"  ".\Release\lib\ctype.o" ".\Release\lib\dup.o" ".\Release\lib\errno.o"  ".\Release\lib\execve.o" ".\Release\lib\malloc.o" ".\Release\lib\open.o"  ".\Release\lib\setsid.o" ".\Release\lib\string.o" ".\Release\lib\wait.o"  ".\Release\lib\write.o"
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

MTL_PROJ=

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("system.dep")
!INCLUDE "system.dep"
!ELSE 
!MESSAGE Warning: cannot find "system.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "system - Win32 Release"
SOURCE=..\boot\head.s
IntDir=.\Release\boot
InputPath=..\boot\head.s
InputName=head

".\Release\boot\head.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bin\ml.exe -c -coff -nologo -Cp /X /Fo "$(IntDir)\$(InputName).o" $(InputPath)
<< 
	
SOURCE=..\init\main.c
IntDir=.\Release\init
InputPath=..\init\main.c
InputName=main

".\Release\init\main.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\math\math_emulate.c
IntDir=.\Release\kernel\math
InputPath=..\kernel\math\math_emulate.c
InputName=math_emulate

".\Release\kernel\math\math_emulate.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\blk_drv\floppy.c
IntDir=.\Release\kernel\blk_drv
InputPath=..\kernel\blk_drv\floppy.c
InputName=floppy

".\Release\kernel\blk_drv\floppy.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\blk_drv\hd.c
IntDir=.\Release\kernel\blk_drv
InputPath=..\kernel\blk_drv\hd.c
InputName=hd

".\Release\kernel\blk_drv\hd.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\blk_drv\ll_rw_blk.c
IntDir=.\Release\kernel\blk_drv
InputPath=..\kernel\blk_drv\ll_rw_blk.c
InputName=ll_rw_blk

".\Release\kernel\blk_drv\ll_rw_blk.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\blk_drv\ramdisk.c
IntDir=.\Release\kernel\blk_drv
InputPath=..\kernel\blk_drv\ramdisk.c
InputName=ramdisk

".\Release\kernel\blk_drv\ramdisk.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\chr_drv\console.c
IntDir=.\Release\kernel\chr_drv
InputPath=..\kernel\chr_drv\console.c
InputName=console

".\Release\kernel\chr_drv\console.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\chr_drv\keyboard.S
IntDir=.\Release\kernel\chr_drv
ProjDir=.
InputPath=..\kernel\chr_drv\keyboard.S
InputName=keyboard

".\Release\kernel\chr_drv\keyboard.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /X /I "..\include" /EP /P $(InputPath) 
	bin\ml.exe -c -coff -nologo -Cp /X /Fo "$(IntDir)\$(InputName).o" $(ProjDir)\$(InputName).i 
<< 
	
SOURCE=..\kernel\chr_drv\rs_io.s
IntDir=.\Release\kernel\chr_drv
InputPath=..\kernel\chr_drv\rs_io.s
InputName=rs_io

".\Release\kernel\chr_drv\rs_io.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bin\ml.exe -c -coff -nologo -Cp /X /Fo "$(IntDir)\$(InputName).o" $(InputPath)
<< 
	
SOURCE=..\kernel\chr_drv\serial.c
IntDir=.\Release\kernel\chr_drv
InputPath=..\kernel\chr_drv\serial.c
InputName=serial

".\Release\kernel\chr_drv\serial.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\chr_drv\tty_io.c
IntDir=.\Release\kernel\chr_drv
InputPath=..\kernel\chr_drv\tty_io.c
InputName=tty_io

".\Release\kernel\chr_drv\tty_io.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\chr_drv\tty_ioctl.c
IntDir=.\Release\kernel\chr_drv
InputPath=..\kernel\chr_drv\tty_ioctl.c
InputName=tty_ioctl

".\Release\kernel\chr_drv\tty_ioctl.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\asm.s
IntDir=.\Release\kernel
InputPath=..\kernel\asm.s
InputName=asm

".\Release\kernel\asm.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bin\ml.exe -c -coff -nologo -Cp /X /Fo "$(IntDir)\$(InputName).o" $(InputPath)
<< 
	
SOURCE=..\kernel\exit.c
IntDir=.\Release\kernel
InputPath=..\kernel\exit.c
InputName=exit

".\Release\kernel\exit.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\fork.c
IntDir=.\Release\kernel
InputPath=..\kernel\fork.c
InputName=fork

".\Release\kernel\fork.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\mktime.c
IntDir=.\Release\kernel
InputPath=..\kernel\mktime.c
InputName=mktime

".\Release\kernel\mktime.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\panic.c
IntDir=.\Release\kernel
InputPath=..\kernel\panic.c
InputName=panic

".\Release\kernel\panic.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\printk.c
IntDir=.\Release\kernel
InputPath=..\kernel\printk.c
InputName=printk

".\Release\kernel\printk.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\sched.c
IntDir=.\Release\kernel
InputPath=..\kernel\sched.c
InputName=sched

".\Release\kernel\sched.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\signal.c
IntDir=.\Release\kernel
InputPath=..\kernel\signal.c
InputName=signal

".\Release\kernel\signal.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\sys.c
IntDir=.\Release\kernel
InputPath=..\kernel\sys.c
InputName=sys

".\Release\kernel\sys.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\system_call.s
IntDir=.\Release\kernel
InputPath=..\kernel\system_call.s
InputName=system_call

".\Release\kernel\system_call.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bin\ml.exe -c -coff -nologo -Cp /X /Fo "$(IntDir)\$(InputName).o" $(InputPath)
<< 
	
SOURCE=..\kernel\traps.c
IntDir=.\Release\kernel
InputPath=..\kernel\traps.c
InputName=traps

".\Release\kernel\traps.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\kernel\vsprintf.c
IntDir=.\Release\kernel
InputPath=..\kernel\vsprintf.c
InputName=vsprintf

".\Release\kernel\vsprintf.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\mm\memory.c
IntDir=.\Release\mm
InputPath=..\mm\memory.c
InputName=memory

".\Release\mm\memory.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\mm\page.s
IntDir=.\Release\mm
InputPath=..\mm\page.s
InputName=page

".\Release\mm\page.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bin\ml.exe -c -coff -nologo -Cp /X /Fo "$(IntDir)\$(InputName).o" $(InputPath)
<< 
	
SOURCE=..\fs\bitmap.c
IntDir=.\Release\fs
InputPath=..\fs\bitmap.c
InputName=bitmap

".\Release\fs\bitmap.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\block_dev.c
IntDir=.\Release\fs
InputPath=..\fs\block_dev.c
InputName=block_dev

".\Release\fs\block_dev.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\buffer.c
IntDir=.\Release\fs
InputPath=..\fs\buffer.c
InputName=buffer

".\Release\fs\buffer.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\char_dev.c
IntDir=.\Release\fs
InputPath=..\fs\char_dev.c
InputName=char_dev

".\Release\fs\char_dev.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\exec.c
IntDir=.\Release\fs
InputPath=..\fs\exec.c
InputName=exec

".\Release\fs\exec.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\fcntl.c
IntDir=.\Release\fs
InputPath=..\fs\fcntl.c
InputName=fcntl

".\Release\fs\fcntl.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\file_dev.c
IntDir=.\Release\fs
InputPath=..\fs\file_dev.c
InputName=file_dev

".\Release\fs\file_dev.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\file_table.c
IntDir=.\Release\fs
InputPath=..\fs\file_table.c
InputName=file_table

".\Release\fs\file_table.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\inode.c
IntDir=.\Release\fs
InputPath=..\fs\inode.c
InputName=inode

".\Release\fs\inode.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\ioctl.c
IntDir=.\Release\fs
InputPath=..\fs\ioctl.c
InputName=ioctl

".\Release\fs\ioctl.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\namei.c
IntDir=.\Release\fs
InputPath=..\fs\namei.c
InputName=namei

".\Release\fs\namei.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\open.c
IntDir=.\Release\fs
InputPath=..\fs\open.c
InputName=open

".\Release\fs\open.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\pipe.c
IntDir=.\Release\fs
InputPath=..\fs\pipe.c
InputName=pipe

".\Release\fs\pipe.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\read_write.c
IntDir=.\Release\fs
InputPath=..\fs\read_write.c
InputName=read_write

".\Release\fs\read_write.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\stat.c
IntDir=.\Release\fs
InputPath=..\fs\stat.c
InputName=stat

".\Release\fs\stat.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\super.c
IntDir=.\Release\fs
InputPath=..\fs\super.c
InputName=super

".\Release\fs\super.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\fs\truncate.c
IntDir=.\Release\fs
InputPath=..\fs\truncate.c
InputName=truncate

".\Release\fs\truncate.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\_exit.c
IntDir=.\Release\lib
InputPath=..\lib\_exit.c
InputName=_exit

".\Release\lib\_exit.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\close.c
IntDir=.\Release\lib
InputPath=..\lib\close.c
InputName=close

".\Release\lib\close.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\ctype.c
IntDir=.\Release\lib
InputPath=..\lib\ctype.c
InputName=ctype

".\Release\lib\ctype.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\dup.c
IntDir=.\Release\lib
InputPath=..\lib\dup.c
InputName=dup

".\Release\lib\dup.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\errno.c
IntDir=.\Release\lib
InputPath=..\lib\errno.c
InputName=errno

".\Release\lib\errno.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\execve.c
IntDir=.\Release\lib
InputPath=..\lib\execve.c
InputName=execve

".\Release\lib\execve.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\malloc.c
IntDir=.\Release\lib
InputPath=..\lib\malloc.c
InputName=malloc

".\Release\lib\malloc.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\open.c
IntDir=.\Release\lib
InputPath=..\lib\open.c
InputName=open

".\Release\lib\open.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\setsid.c
IntDir=.\Release\lib
InputPath=..\lib\setsid.c
InputName=setsid

".\Release\lib\setsid.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\string.c
IntDir=.\Release\lib
InputPath=..\lib\string.c
InputName=string

".\Release\lib\string.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\wait.c
IntDir=.\Release\lib
InputPath=..\lib\wait.c
InputName=wait

".\Release\lib\wait.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	
SOURCE=..\lib\write.c
IntDir=.\Release\lib
InputPath=..\lib\write.c
InputName=write

".\Release\lib\write.o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	cl /nologo  /W3 /vd0 /GX- /Ob1 /Ot /Oa /Og /Op /X /I "..\include" /u /FAs /Fa"$(IntDir)/" /Fp"Release/system.pch" /YX /Fo"$(IntDir)/$(InputName).o" /Fd"$(IntDir)/" /FD /c $(InputPath)
<< 
	

!ENDIF 

