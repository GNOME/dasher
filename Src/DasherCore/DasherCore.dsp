# Microsoft Developer Studio Project File - Name="DasherCore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DasherCore - Win32 Win95 compatible Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DasherCore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DasherCore.mak" CFG="DasherCore - Win32 Win95 compatible Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DasherCore - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Win95 compatible Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Win95 compatible Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DasherCore - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Release/"
# PROP Intermediate_Dir "Target/Win32 Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Debug"
# PROP Intermediate_Dir "Target/Win32 Debug/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Unicode Debug"
# PROP Intermediate_Dir "Target/Win32 Unicode Debug/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug_static\libDasherCore.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Win95 compatible Release"
# PROP Intermediate_Dir "Target/Win32 Win95 compatible Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"DasherCoreDebugStatic\libDasherCore.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Win95 compatible Release/"
# PROP Intermediate_Dir "Target/Win32 Win95 compatible Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_STATIC" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"DasherCoreReleaseStatic\libDasherCore.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Unicode Release/"
# PROP Intermediate_Dir "Target/Win32 Unicode Release/Intermediate"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_STATIC" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_STATIC" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"DasherCoreReleaseStatic\libDasherCore.lib"
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "DasherCore - Win32 Release"
# Name "DasherCore - Win32 Debug"
# Name "DasherCore - Win32 Unicode Debug"
# Name "DasherCore - Win32 Win95 compatible Debug"
# Name "DasherCore - Win32 Win95 compatible Release"
# Name "DasherCore - Win32 Unicode Release"
# Begin Source File

SOURCE=.\Alphabet.cpp
# End Source File
# Begin Source File

SOURCE=.\Alphabet.h
# End Source File
# Begin Source File

SOURCE=.\AlphabetMap.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphabetMap.h
# End Source File
# Begin Source File

SOURCE=.\AlphIO.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphIO.h
# End Source File
# Begin Source File

SOURCE=.\Context.h
# End Source File
# Begin Source File

SOURCE=.\CustomAlphabet.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomAlphabet.h
# End Source File
# Begin Source File

SOURCE=.\DashEdit.h
# End Source File
# Begin Source File

SOURCE=.\DasherAppInterface.h
# End Source File
# Begin Source File

SOURCE=.\DasherInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherInterface.h
# End Source File
# Begin Source File

SOURCE=.\DasherModel.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherModel.h
# End Source File
# Begin Source File

SOURCE=.\DasherNode.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherNode.h
# End Source File
# Begin Source File

SOURCE=.\DasherScreen.h
# End Source File
# Begin Source File

SOURCE=.\DasherSettingsInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherSettingsInterface.h
# End Source File
# Begin Source File

SOURCE=.\DasherTypes.h
# End Source File
# Begin Source File

SOURCE=.\DasherView.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherView.h
# End Source File
# Begin Source File

SOURCE=.\DasherView.inl
# End Source File
# Begin Source File

SOURCE=.\DasherViewSquare.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherViewSquare.h
# End Source File
# Begin Source File

SOURCE=.\DasherViewSquare.inl
# End Source File
# Begin Source File

SOURCE=.\DasherWidgetInterface.h
# End Source File
# Begin Source File

SOURCE=.\EnglishAlphabet.h
# End Source File
# Begin Source File

SOURCE=.\LanguageModel.cpp
# End Source File
# Begin Source File

SOURCE=.\LanguageModel.h
# End Source File
# Begin Source File

SOURCE=.\ppmModel.cpp
# End Source File
# Begin Source File

SOURCE=.\ppmModel.h
# End Source File
# Begin Source File

SOURCE=.\SettingsStore.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsStore.h
# End Source File
# End Target
# End Project
