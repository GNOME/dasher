# Microsoft Developer Studio Project File - Name="Dasher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Dasher - Win32 Win95 compatible Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Dasher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dasher.mak" CFG="Dasher - Win32 Win95 compatible Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dasher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Dasher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Dasher - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "Dasher - Win32 Win95 compatible Debug" (based on "Win32 (x86) Application")
!MESSAGE "Dasher - Win32 Win95 compatible Release" (based on "Win32 (x86) Application")
!MESSAGE "Dasher - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Dasher - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Release"
# PROP Intermediate_Dir "Target/Win32 Release/Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Dasher - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Debug/"
# PROP Intermediate_Dir "Target/Win32 Debug/Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Dasher - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Dasher___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "Dasher___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Unicode Debug/"
# PROP Intermediate_Dir "Target/Win32 Unicode Debug/Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_STATIC" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "Dasher - Win32 Win95 compatible Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Dasher___Win32_Win95_compatible_Debug"
# PROP BASE Intermediate_Dir "Dasher___Win32_Win95_compatible_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Target/Win32 Win95 compatible Debug"
# PROP Intermediate_Dir "Target/Win32 Win95 compatible Debug/Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /D "OriginalWin95" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Dasher - Win32 Win95 compatible Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Dasher___Win32_Win95_compatible_Release"
# PROP BASE Intermediate_Dir "Dasher___Win32_Win95_compatible_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Win95 compatible Release"
# PROP Intermediate_Dir "Target/Win32 Win95 compatible Release/Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /D "OriginalWin95" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Dasher - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Dasher___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "Dasher___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Target/Win32 Unicode Release"
# PROP Intermediate_Dir "Target/Win32 Unicode Release/Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STATIC" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_STATIC" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "Dasher - Win32 Release"
# Name "Dasher - Win32 Debug"
# Name "Dasher - Win32 Unicode Debug"
# Name "Dasher - Win32 Win95 compatible Debug"
# Name "Dasher - Win32 Win95 compatible Release"
# Name "Dasher - Win32 Unicode Release"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\CopyAllLarge.bmp
# End Source File
# Begin Source File

SOURCE=.\CopyAllSmall.bmp
# End Source File
# Begin Source File

SOURCE=.\Dasher.ico
# End Source File
# Begin Source File

SOURCE=.\Dasher.rc
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ".cpp"
# Begin Group "Widgets"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Widgets\Aboutbox.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\AboutBox.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\AlphabetBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\AlphabetBox.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Canvas.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Canvas.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\DasherEncodingToCP.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\DasherEncodingToCP.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Edit.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Edit.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\FilenameGUI.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\FilenameGUI.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Menu.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Screen.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Screen.inl
# End Source File
# Begin Source File

SOURCE=.\Widgets\Slidebar.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Slidebar.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\SplashScreen.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Splitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Splitter.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\Toolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\Toolbar.h
# End Source File
# Begin Source File

SOURCE=.\Widgets\WinOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgets\WinOptions.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\DasherWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\DasherWindow.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\WinHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\WinHelper.h
# End Source File
# Begin Source File

SOURCE=.\WinLocalisation.cpp
# End Source File
# Begin Source File

SOURCE=.\WinLocalisation.h
# End Source File
# Begin Source File

SOURCE=.\WinMain.cpp
# End Source File
# Begin Source File

SOURCE=.\WinUTF8.cpp
# End Source File
# Begin Source File

SOURCE=.\WinUTF8.h
# End Source File
# Begin Source File

SOURCE=.\WinWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\WinWrap.h
# End Source File
# End Group
# End Target
# End Project
