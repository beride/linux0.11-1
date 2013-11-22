# Microsoft Developer Studio Generated NMAKE File, Based on bootsect.dsp
!IF "$(CFG)" == ""
CFG=bootsect - Win32 Release
!MESSAGE No configuration specified. Defaulting to bootsect - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "bootsect - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bootsect.mak" CFG="bootsect - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bootsect - Win32 Release" (based on "Win32 (x86) Generic Project")
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
INTDIR=.\Release\boot

ALL : ".\Release\boot\bootsect.obj" 


CLEAN :
	-@erase 
	-@erase ".\Release\boot\bootsect.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

IntDir=.\Release\boot
OutDir=.\Release
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : ".\Release\boot\bootsect.obj" 
   link.exe /VXD /NOLOGO /NOENTRY /NODEFAULTLIB /OUT:.\Release\bootsect .\Release\boot\bootsect.obj
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

MTL_PROJ=

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("bootsect.dep")
!INCLUDE "bootsect.dep"
!ELSE 
!MESSAGE Warning: cannot find "bootsect.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "bootsect - Win32 Release"
SOURCE=..\boot\bootsect.s
IntDir=.\Release\boot
WkspDir=.
InputPath=..\boot\bootsect.s
InputName=bootsect

"$(INTDIR)\bootsect.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(WkspDir)\masm\ml.exe /Cp /c /nologo /X /coff /Fo "$(IntDir)\$(InputName)" $(InputPath)
<< 
	

!ENDIF 

