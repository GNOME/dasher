# Microsoft Developer Studio Project File - Name="Expat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Expat - Win32 Win95 compatible Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Expat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Expat.mak" CFG="Expat - Win32 Win95 compatible Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Expat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Win95 compatible Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Win95 compatible Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Expat - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Expat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Release/"
# PROP Intermediate_Dir "Target/Win32 Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Expat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Debug"
# PROP Intermediate_Dir "Target/Win32 Debug/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Expat - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Unicode Debug"
# PROP Intermediate_Dir "Target/Win32 Unicode Debug/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug_static\libexpat.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Expat - Win32 Win95 compatible Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Win95 compatible Release"
# PROP Intermediate_Dir "Target/Win32 Win95 compatible Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "COMPILED_FROM_DSP" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"ExpatDebugStatic\libexpat.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Expat - Win32 Win95 compatible Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Win95 compatible Release/"
# PROP Intermediate_Dir "Target/Win32 Win95 compatible Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"ExpatReleaseStatic\libexpat.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Expat - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Unicode Release/"
# PROP Intermediate_Dir "Target/Win32 Unicode Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "COMPILED_FROM_DSP" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"ExpatReleaseStatic\libexpat.lib"
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Expat - Win32 Release"
# Name "Expat - Win32 Debug"
# Name "Expat - Win32 Unicode Debug"
# Name "Expat - Win32 Win95 compatible Debug"
# Name "Expat - Win32 Win95 compatible Release"
# Name "Expat - Win32 Unicode Release"
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
