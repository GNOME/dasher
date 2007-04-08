# Microsoft Developer Studio Project File - Name="Expat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Expat - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Expat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Expat.mak" CFG="Expat - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Expat - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Win95 compatible Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Win95 compatible Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Accessible Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Expat - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Target/Win32 Unicode Debug"
# PROP BASE Intermediate_Dir ".\Target/Win32 Unicode Debug/Intermediate"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Target/Win32 Unicode Debug"
# PROP Intermediate_Dir ".\Target/Win32 Unicode Debug/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Unicode Debug/Intermediate/Expat.pch" /Fo".\Target/Win32 Unicode Debug/Intermediate/" /Fd".\Target/Win32 Unicode Debug/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Unicode Debug/Intermediate/Expat.pch" /Fo".\Target/Win32 Unicode Debug/Intermediate/" /Fd".\Target/Win32 Unicode Debug/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Unicode Debug\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Unicode Debug\Expat.lib" 

!ELSEIF  "$(CFG)" == "Expat - Win32 Win95 compatible Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Target/Win32 Win95 compatible Release"
# PROP BASE Intermediate_Dir ".\Target/Win32 Win95 compatible Release/Intermediate"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Target/Win32 Win95 compatible Release"
# PROP Intermediate_Dir ".\Target/Win32 Win95 compatible Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Win95 compatible Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Win95 compatible Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\Expat.lib" 

!ELSEIF  "$(CFG)" == "Expat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Target/Win32 Debug"
# PROP BASE Intermediate_Dir ".\Target/Win32 Debug/Intermediate"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Target/Win32 Debug"
# PROP Intermediate_Dir ".\Target/Win32 Debug/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Debug/Intermediate/Expat.pch" /Fo".\Target/Win32 Debug/Intermediate/" /Fd".\Target/Win32 Debug/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Debug/Intermediate/Expat.pch" /Fo".\Target/Win32 Debug/Intermediate/" /Fd".\Target/Win32 Debug/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Debug\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Debug\Expat.lib" 

!ELSEIF  "$(CFG)" == "Expat - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Target/Win32 Unicode Release"
# PROP BASE Intermediate_Dir ".\Target/Win32 Unicode Release/Intermediate"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Target/Win32 Unicode Release"
# PROP Intermediate_Dir ".\Target/Win32 Unicode Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "COMPILED_FROM_DSP" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Unicode Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Unicode Release/Intermediate/" /Fd".\Target/Win32 Unicode Release/Intermediate/" /c /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "COMPILED_FROM_DSP" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Unicode Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Unicode Release/Intermediate/" /Fd".\Target/Win32 Unicode Release/Intermediate/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Unicode Release\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Unicode Release\Expat.lib" 

!ELSEIF  "$(CFG)" == "Expat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Target/Win32 Release"
# PROP BASE Intermediate_Dir ".\Target/Win32 Release/Intermediate"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Target/Win32 Release"
# PROP Intermediate_Dir ".\Target/Win32 Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "COMPILED_FROM_DSP" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Release/Intermediate/" /Fd".\Target/Win32 Release/Intermediate/" /c /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "COMPILED_FROM_DSP" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Release/Intermediate/" /Fd".\Target/Win32 Release/Intermediate/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Release\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Release\Expat.lib" 

!ELSEIF  "$(CFG)" == "Expat - Win32 Win95 compatible Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Target/Win32 Win95 compatible Release"
# PROP BASE Intermediate_Dir ".\Target/Win32 Win95 compatible Release/Intermediate"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Target/Win32 Win95 compatible Release"
# PROP Intermediate_Dir ".\Target/Win32 Win95 compatible Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "COMPILED_FROM_DSP" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Win95 compatible Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /c /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "COMPILED_FROM_DSP" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Win95 compatible Release/Intermediate/Expat.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\Expat.lib" 

!ELSEIF  "$(CFG)" == "Expat - Win32 Accessible Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Accessible Unicode Debug"
# PROP BASE Intermediate_Dir "Accessible Unicode Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Accessible Unicode Debug"
# PROP Intermediate_Dir "Accessible Unicode Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Unicode Debug/Intermediate/Expat.pch" /Fo".\Target/Win32 Unicode Debug/Intermediate/" /Fd".\Target/Win32 Unicode Debug/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Unicode Debug/Intermediate/Expat.pch" /Fo".\Target/Win32 Unicode Debug/Intermediate/" /Fd".\Target/Win32 Unicode Debug/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Unicode Debug\Expat.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Unicode Debug\Expat.lib" 

!ENDIF

# Begin Target

# Name "Expat - Win32 Unicode Debug"
# Name "Expat - Win32 Win95 compatible Debug"
# Name "Expat - Win32 Debug"
# Name "Expat - Win32 Unicode Release"
# Name "Expat - Win32 Release"
# Name "Expat - Win32 Win95 compatible Release"
# Name "Expat - Win32 Accessible Unicode Debug"
# Begin Source File

SOURCE=.\lib\ascii.h
# End Source File
# Begin Source File

SOURCE=.\lib\asciitab.h
# End Source File
# Begin Source File

SOURCE=.\lib\expat.h
# End Source File
# Begin Source File

SOURCE=.\lib\iasciitab.h
# End Source File
# Begin Source File

SOURCE=.\lib\latin1tab.h
# End Source File
# Begin Source File

SOURCE=.\lib\nametab.h
# End Source File
# Begin Source File

SOURCE=.\lib\utf8tab.h
# End Source File
# Begin Source File

SOURCE=.\lib\xmlparse.c
# End Source File
# Begin Source File

SOURCE=.\lib\xmlrole.c
# End Source File
# Begin Source File

SOURCE=.\lib\xmlrole.h
# End Source File
# Begin Source File

SOURCE=.\lib\xmltok.c
# End Source File
# Begin Source File

SOURCE=.\lib\xmltok.h
# End Source File
# Begin Source File

SOURCE=.\lib\xmltok_impl.h
# End Source File
# End Target
# End Project

