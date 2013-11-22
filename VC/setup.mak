# Microsoft Developer Studio Generated NMAKE File, Based on setup.dsp
!IF "$(CFG)" == ""
CFG=setup - Win32 Release
!MESSAGE No configuration specified. Defaulting to setup - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "setup - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "setup.mak" CFG="setup - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "setup - Win32 Release" (based on "Win32 (x86) Generic Project")
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

ALL : ".\Release\boot\setup.obj" 


CLEAN :
	-@erase 
	-@erase ".\Release\boot\setup.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

IntDir=.\Release\boot
OutDir=.\Release
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : ".\Release\boot\setup.obj" 
   link.exe /VXD /NOLOGO /NOENTRY /NODEFAULTLIB /OUT:.\Release\setup .\Release\boot\setup.obj
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

MTL_PROJ=

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("setup.dep")
!INCLUDE "setup.dep"
!ELSE 
!MESSAGE Warning: cannot find "setup.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "setup - Win32 Release"
SOURCE=..\boot\setup.s
IntDir=.\Release\boot
WkspDir=.
InputPath=..\boot\setup.s
InputName=setup

"$(INTDIR)\setup.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	$(WkspDir)\masm\ml.exe /Cp /c /nologo /X /coff /Fo "$(IntDir)\$(InputName)" $(InputPath) 
	
<< 
	

!ENDIF 

