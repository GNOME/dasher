// FilenameGUI.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

/*
	TODO: For some reason I can't get VC++ 6 SP4 or SP5 (I upgraded just to see) on win2k SP1
	to debug this module without raising "user breakpoints". While this can be due to
	deliberate int3 instructions, in my experience it is normally indicative of junk code
	being loaded due to a buffer overrun. I am concerned.
	
	There seem to be a few common gotchas using the OPENFILNAME structure, but I don't see
	what I'm doing wrong. In fact the WINUI\CMDDLG\COMDLG32 SDK sample suffers from exactly
	the same problem on my system, I even downloaded the latest SDK to see if it has been
	fixed. Now I really don't know what to do, short of reinstalling Windows and Devstudio,
	which I really don't want to do.
	
	I have wasted faar too much time fiddlying and Googling already. Could someone tell me
	what the problem is? Could they also tell me if it will be just fine on other computers?
	
	Update - on another computer (also win2k) with VC++ the problem isn't there.
	I don't have time to work out the difference, I'll just try and compile any releases on
	that computer (uist for inference group members).
	
	IAM 08/02
*/

#include "WinCommon.h"

#include "FilenameGUI.h"
#include "../resource.h"

#ifndef _MAX_FNAME
#define _MAX_FNAME                 64
#endif

using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

const Tstring CFilenameGUI::Empty = TEXT("");

CFilenameGUI::CFilenameGUI(HWND WindowWithTitlebar, Tstring AppName, bool NewWithDate)
:WindowWithTitlebar(WindowWithTitlebar), AppName(AppName), NewWithDate(NewWithDate), Dirty(false), FileAndPath(TEXT("")) {

  TCHAR CurrentDirectory[_MAX_DIR];
  if(GetCurrentDirectory(_MAX_DIR, CurrentDirectory) > 0) {
    OriginalPath = CurrentDirectory;
    string foo;
    if(OriginalPath[OriginalPath.size() - 1] != TEXT('\\'))     // Yuck. Can't find a call to get
      OriginalPath += TEXT('\\');       // the directory separator
  }
  else
    OriginalPath = TEXT("");

  if(NewWithDate) {
    New();
  }
  else
    SetWindowTitle();
}

const Tstring & CFilenameGUI::New() {
  if(!NewWithDate) {
    FileAndPath = TEXT("");
  }
  else {
    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);

    // I don't care that different countries write dates differently.
    // It's not as if the following is even how I would write a date normally.
    // The point is that this is a _filename_ - and filenames sort in the
    // correct order iff you format the date with this ordering.
    TCHAR Buffer[32];
    _stprintf(Buffer, TEXT("%04d-%02d-%02d_%02d-%02d-%02d.txt"), SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);

    FileAndPath = OriginalPath + Buffer;
  }

  Dirty = false;
  SetWindowTitle();

  return FileAndPath;
}

const Tstring & CFilenameGUI::Open() {
  OPENFILENAME OpenDialogStructure;
  TCHAR FullOpenFilename[_MAX_PATH] = TEXT("");

  // Initialize OPENFILENAME
  ZeroMemory(&OpenDialogStructure, sizeof(OpenDialogStructure));
#ifdef OPENFILENAME_SIZE_VERSION_400
  OpenDialogStructure.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
  OpenDialogStructure.lStructSize = sizeof(OpenDialogStructure);
#endif
  OpenDialogStructure.hwndOwner = WindowWithTitlebar;
  OpenDialogStructure.lpstrFile = FullOpenFilename;
  OpenDialogStructure.nMaxFile = MAX_PATH;
  OpenDialogStructure.lpstrFilter = TEXT("All Files\0*.*\0Text Documents (*.txt)\0*.txt\0");
  OpenDialogStructure.nFilterIndex = 1;
  OpenDialogStructure.lpstrDefExt = TEXT("txt");
  OpenDialogStructure.lpstrInitialDir = NULL;
  OpenDialogStructure.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

  // Display the Open dialog box. 
  if(GetOpenFileName(&OpenDialogStructure) != TRUE)
    return Empty;

  // Remember file details if we have them
  FileAndPath = OpenDialogStructure.lpstrFile;

  return FileAndPath;
}

const Tstring & CFilenameGUI::Save() {
  if(FileAndPath != TEXT(""))
    return FileAndPath;         // Return current filename if there is one
  else
    // Or as fairly standard in Windows apps, we revert to SaveAs behaviour
    return SaveAs();
}

const Tstring & CFilenameGUI::SaveAs() {
  // Lots of this is copy and pasted from FileOpen(), which means my programing style has gone to pot
  // TODO make this better
  OPENFILENAME OpenDialogStructure;
  TCHAR FullOpenFilename[MAX_PATH] = TEXT("");

  // Initialize OPENFILENAME
  ZeroMemory(&OpenDialogStructure, sizeof(OpenDialogStructure));
#ifdef OPENFILENAME_SIZE_VERSION_400
  OpenDialogStructure.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
  OpenDialogStructure.lStructSize = sizeof(OpenDialogStructure);
#endif
  OpenDialogStructure.hwndOwner = WindowWithTitlebar;
  OpenDialogStructure.lpstrFile = FullOpenFilename;
  OpenDialogStructure.nMaxFile = MAX_PATH;
  OpenDialogStructure.lpstrFilter = TEXT("All Files\0*.*\0Text Documents (*.txt)\0*.txt\0");
  OpenDialogStructure.nFilterIndex = 1;
  OpenDialogStructure.lpstrDefExt = TEXT("txt");
  OpenDialogStructure.lpstrInitialDir = NULL;
  OpenDialogStructure.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

  // Display the SaveAs dialog box. 
  if(GetSaveFileName(&OpenDialogStructure) != TRUE)
    return Empty;

  // Remember file details if we have them
  FileAndPath = OpenDialogStructure.lpstrFile;

  return FileAndPath;
}

void CFilenameGUI::SetNewWithDate(bool Value) {
  NewWithDate = Value;
}

void CFilenameGUI::SetDirty(bool Value) {
  if(Dirty != Value) {
    Dirty = Value;
    SetWindowTitle();
  }
}

void CFilenameGUI::SetWindowTitle() {
  Tstring TitleText;

  TCHAR PrettyName[_MAX_FNAME];
  if(GetFileTitle(FileAndPath.c_str(), PrettyName, _MAX_FNAME) == 0)
    TitleText = PrettyName;
  else
    WinLocalisation::GetResourceString(IDS_UNTITLED_FILE, &TitleText);

  if(Dirty)
    TitleText += TEXT("*");
  TitleText += TEXT(" - ");
  TitleText += AppName;
  SendMessage(WindowWithTitlebar, WM_SETTEXT, 0, (LPARAM) TitleText.c_str());
}

int CFilenameGUI::QuerySaveFirst() {
  if(Dirty) {
    Tstring ResourceString;
    Tstring Title;
    WinLocalisation::GetResourceString(IDS_QUERY_SAVE_CHANGES, &ResourceString);
    WinLocalisation::GetResourceString(IDS_UNSAVED_CHANGES, &Title);

    return MessageBox(WindowWithTitlebar, ResourceString.c_str(), Title.c_str(), MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON1 | MB_APPLMODAL);
  }

  return IDNO;
}

void CFilenameGUI::SetFilename(const Tstring &FileName) {
  FileAndPath = FileName;
  Dirty = false;
  SetWindowTitle();
}
