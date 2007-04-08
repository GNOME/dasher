# Microsoft Developer Studio Project File - Name="DasherCore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DasherCore - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DasherCore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DasherCore.mak" CFG="DasherCore - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DasherCore - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Win95 compatible Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Win95 compatible Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DasherCore - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DasherCore - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Debug/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Debug/Intermediate/" /Fd".\Target/Win32 Debug/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Debug/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Debug/Intermediate/" /Fd".\Target/Win32 Debug/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Debug\DasherCore.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Debug\DasherCore.lib" 

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Release"

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
# ADD BASE CPP /nologo /MT /Zi /W3 /Og /Ob2 /Oi /Ot /GA /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "_STATIC" /D "_UNICODE" /D "_UNICODE" /GF /Gy /Fp".\Target/Win32 Unicode Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Unicode Release/Intermediate/" /Fd".\Target/Win32 Unicode Release/Intermediate/" /c /GX 
# ADD CPP /nologo /MT /Zi /W3 /Og /Ob2 /Oi /Ot /GA /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "_STATIC" /D "_UNICODE" /D "_UNICODE" /GF /Gy /Fp".\Target/Win32 Unicode Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Unicode Release/Intermediate/" /Fd".\Target/Win32 Unicode Release/Intermediate/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Unicode Release\DasherCore.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Unicode Release\DasherCore.lib" 

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Debug"

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
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Win95 compatible Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_LIB" /D "_MBCS" /Fp".\Target/Win32 Win95 compatible Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\DasherCore.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\DasherCore.lib" 

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /O2 /Og /Ob2 /Oi /Ot /GA /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "_STATIC" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Release/Intermediate/" /Fd".\Target/Win32 Release/Intermediate/" /c /GX 
# ADD CPP /nologo /MT /W3 /O2 /Og /Ob2 /Oi /Ot /GA /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "_STATIC" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Release/Intermediate/" /Fd".\Target/Win32 Release/Intermediate/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Release\DasherCore.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Release\DasherCore.lib" 

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Release"

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
# ADD BASE CPP /nologo /MT /W3 /Og /Ob2 /Oi /Ot /GA /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "_STATIC" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Win95 compatible Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /c /GX 
# ADD CPP /nologo /MT /W3 /Og /Ob2 /Oi /Ot /GA /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_LIB" /D "_STATIC" /D "_MBCS" /GF /Gy /Fp".\Target/Win32 Win95 compatible Release/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Win95 compatible Release/Intermediate/" /Fd".\Target/Win32 Win95 compatible Release/Intermediate/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\DasherCore.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Win95 compatible Release\DasherCore.lib" 

!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Debug"

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
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_LIB" /D "_UNICODE" /D "_UNICODE" /Fp".\Target/Win32 Unicode Debug/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Unicode Debug/Intermediate/" /Fd".\Target/Win32 Unicode Debug/Intermediate/" /FR /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_STATIC" /D "_LIB" /D "_UNICODE" /D "_UNICODE" /Fp".\Target/Win32 Unicode Debug/Intermediate/DasherCore.pch" /Fo".\Target/Win32 Unicode Debug/Intermediate/" /Fd".\Target/Win32 Unicode Debug/Intermediate/" /FR /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Target\Win32 Unicode Debug\DasherCore.lib" 
# ADD LIB32 /nologo /out:".\Target\Win32 Unicode Debug\DasherCore.lib" 

!ENDIF

# Begin Target

# Name "DasherCore - Win32 Debug"
# Name "DasherCore - Win32 Unicode Release"
# Name "DasherCore - Win32 Win95 compatible Debug"
# Name "DasherCore - Win32 Release"
# Name "DasherCore - Win32 Win95 compatible Release"
# Name "DasherCore - Win32 Unicode Debug"
# Begin Group "LanguageModelling"

# PROP Default_Filter ""
# Begin Source File

SOURCE=LanguageModelling\BigramLanguageModel.cpp
# End Source File
# Begin Source File

SOURCE=LanguageModelling\BigramLanguageModel.h
# End Source File
# Begin Source File

SOURCE=LanguageModelling\DictLanguageModel.cpp
# End Source File
# Begin Source File

SOURCE=LanguageModelling\DictLanguageModel.h
# End Source File
# Begin Source File

SOURCE=LanguageModelling\LanguageModel.cpp

!IF  "$(CFG)" == "DasherCore - Win32 Debug"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GZ /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Release"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Debug"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GZ /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Release"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Release"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Debug"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=LanguageModelling\LanguageModel.h
# End Source File
# Begin Source File

SOURCE=LanguageModelling\PPMLanguageModel.cpp

!IF  "$(CFG)" == "DasherCore - Win32 Debug"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GZ /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Release"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Debug"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GZ /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Release"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Win95 compatible Release"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GX 
!ELSEIF  "$(CFG)" == "DasherCore - Win32 Unicode Debug"

# PROP Intermediate_Dir "$(IntDir)/$(InputName)1.obj"
# ADD CPP /nologo /Fo"$(IntDir)/$(InputName)1.obj" /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=LanguageModelling\PPMLanguageModel.h
# End Source File
# Begin Source File

SOURCE=LanguageModelling\WordLanguageModel.cpp
# End Source File
# Begin Source File

SOURCE=LanguageModelling\WordLanguageModel.h
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=Win32\DasherInterface.cpp
# End Source File
# Begin Source File

SOURCE=Win32\DasherInterface.h
# End Source File
# Begin Source File

SOURCE=Win32\WinCommon.h
# End Source File
# Begin Source File

SOURCE=Win32\WinHelper.cpp
# End Source File
# Begin Source File

SOURCE=Win32\WinHelper.h
# End Source File
# Begin Source File

SOURCE=Win32\WinLocalisation.cpp
# End Source File
# Begin Source File

SOURCE=Win32\WinLocalisation.h
# End Source File
# Begin Source File

SOURCE=Win32\WinOptions.cpp
# End Source File
# Begin Source File

SOURCE=Win32\WinOptions.h
# End Source File
# Begin Source File

SOURCE=Win32\WinUTF8.cpp
# End Source File
# Begin Source File

SOURCE=Win32\WinUTF8.h
# End Source File
# Begin Source File

SOURCE=Win32\WinWrap.cpp
# End Source File
# Begin Source File

SOURCE=Win32\WinWrap.h
# End Source File
# End Group
# Begin Source File

SOURCE=Alphabet\AlphIO.cpp
# End Source File
# Begin Source File

SOURCE=Alphabet\AlphIO.h
# End Source File
# Begin Source File

SOURCE=Alphabet\Alphabet.cpp
# End Source File
# Begin Source File

SOURCE=Alphabet\Alphabet.h
# End Source File
# Begin Source File

SOURCE=Alphabet\AlphabetMap.cpp
# End Source File
# Begin Source File

SOURCE=Alphabet\AlphabetMap.h
# End Source File
# Begin Source File

SOURCE=ColourIO.cpp
# End Source File
# Begin Source File

SOURCE=ColourIO.h
# End Source File
# Begin Source File

SOURCE=CustomColours.cpp
# End Source File
# Begin Source File

SOURCE=CustomColours.h
# End Source File
# Begin Source File

SOURCE=.\DashEdit.h
# End Source File
# Begin Source File

SOURCE=.\DasherAppInterface.h
# End Source File
# Begin Source File

SOURCE=DasherComponent.h
# End Source File
# Begin Source File

SOURCE=DasherInput.h
# End Source File
# Begin Source File

SOURCE=DasherInterfaceBase.cpp
# End Source File
# Begin Source File

SOURCE=DasherInterfaceBase.h
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

SOURCE=DasherSettingsInterface.cpp
# End Source File
# Begin Source File

SOURCE=DasherSettingsInterface.h
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

SOURCE=Event.h
# End Source File
# Begin Source File

SOURCE=EventHandler.cpp
# End Source File
# Begin Source File

SOURCE=EventHandler.h
# End Source File
# Begin Source File

SOURCE=FrameRate.h
# End Source File
# Begin Source File

SOURCE=Parameters.h
# End Source File
# Begin Source File

SOURCE=.\SettingsStore.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsStore.h
# End Source File
# End Target
# End Project

