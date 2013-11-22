# Microsoft Developer Studio Generated NMAKE File, Based on Documents.dsp
!IF "$(CFG)" == ""
CFG=Documents - Win32 Release
!MESSAGE No configuration specified. Defaulting to Documents - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "Documents - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Documents.mak" CFG="Documents - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Documents - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\Release
INTDIR=.\Release

ALL : 


CLEAN :
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

MTL=midl.exe
MTL_PROJ=

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Documents.dep")
!INCLUDE "Documents.dep"
!ELSE 
!MESSAGE Warning: cannot find "Documents.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Documents - Win32 Release"

!ENDIF 

