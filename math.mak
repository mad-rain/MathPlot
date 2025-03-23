# Microsoft Developer Studio Generated NMAKE File, Based on math.dsp
!IF "$(CFG)" == ""
CFG=math - Win32 Debug
!MESSAGE No configuration specified. Defaulting to math - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "math - Win32 Release" && "$(CFG)" != "math - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "math.mak" CFG="math - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "math - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "math - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "math - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\math.exe"


CLEAN :
	-@erase "$(INTDIR)\extremum.obj"
	-@erase "$(INTDIR)\func.obj"
	-@erase "$(INTDIR)\key_pnt.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\math.res"
	-@erase "$(INTDIR)\plot.obj"
	-@erase "$(INTDIR)\Plot_fnt.obj"
	-@erase "$(INTDIR)\Types.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\math.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=xicl6.exe
CPP_PROJ=/nologo /ML /W4 /GX /Ox /Ot /Og /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\math.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /O3 /O3 /Qpc80 /Qlong_double /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\math.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\math.bsc" 
BSC32_SBRS= \
	
LINK32=xilink6.exe
LINK32_FLAGS=opengl32.lib comctl32.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\math.pdb" /machine:I386 /out:"$(OUTDIR)\math.exe" 
LINK32_OBJS= \
	"$(INTDIR)\extremum.obj" \
	"$(INTDIR)\func.obj" \
	"$(INTDIR)\key_pnt.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\plot.obj" \
	"$(INTDIR)\Plot_fnt.obj" \
	"$(INTDIR)\Types.obj" \
	"$(INTDIR)\math.res"

"$(OUTDIR)\math.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "math - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\math.exe"


CLEAN :
	-@erase "$(INTDIR)\extremum.obj"
	-@erase "$(INTDIR)\func.obj"
	-@erase "$(INTDIR)\key_pnt.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\math.res"
	-@erase "$(INTDIR)\plot.obj"
	-@erase "$(INTDIR)\Plot_fnt.obj"
	-@erase "$(INTDIR)\Types.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\math.exe"
	-@erase "$(OUTDIR)\math.ilk"
	-@erase "$(OUTDIR)\math.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=xicl6.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\math.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\math.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\math.bsc" 
BSC32_SBRS= \
	
LINK32=xilink6.exe
LINK32_FLAGS=opengl32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\math.pdb" /debug /machine:I386 /out:"$(OUTDIR)\math.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\extremum.obj" \
	"$(INTDIR)\func.obj" \
	"$(INTDIR)\key_pnt.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\plot.obj" \
	"$(INTDIR)\Plot_fnt.obj" \
	"$(INTDIR)\Types.obj" \
	"$(INTDIR)\math.res"

"$(OUTDIR)\math.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("math.dep")
!INCLUDE "math.dep"
!ELSE 
!MESSAGE Warning: cannot find "math.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "math - Win32 Release" || "$(CFG)" == "math - Win32 Debug"
SOURCE=.\extremum.cpp

"$(INTDIR)\extremum.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\func.cpp

"$(INTDIR)\func.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\key_pnt.cpp

"$(INTDIR)\key_pnt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plot.cpp

"$(INTDIR)\plot.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Plot_fnt.cpp

"$(INTDIR)\Plot_fnt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Types.cpp

"$(INTDIR)\Types.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\math.rc

"$(INTDIR)\math.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

