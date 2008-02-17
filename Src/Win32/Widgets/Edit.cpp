// Edit.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// NOTES:
//
// File I/O is very simplistic. It relies on the fact that there isn't
// going to be loads of text in the edit box. Otherwise I'm sure performance
// would be unacceptable.

#include "WinCommon.h"

#include "Edit.h"
#include "../../DasherCore/Event.h"
#include "FilenameGUI.h"
#include "../resource.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#ifndef _WIN32_WCE
#include "../ActionSpeech.h"
#endif

#include "../Common/DasherEncodingToCP.h"

using namespace Dasher;
using namespace std;
using namespace WinLocalisation;
using namespace WinUTF8;

CEdit::CEdit(CAppSettings *pAppSettings) {
  m_FontSize = 0;
  m_FontName = "";
  FileHandle = INVALID_HANDLE_VALUE;
  m_FilenameGUI = 0;
  threadid = 0;
  
  // TODO: Check that this is all working okay (it quite probably
  // isn't). In the long term need specialised editor classes.
  targetwindow = 0;
  textentry = false;

  m_pAppSettings = pAppSettings;

  CodePage = GetUserCodePage();
#ifndef _WIN32_WCE
  m_Font = GetCodePageFont(CodePage, 14);
#endif

  // TODO: Generalise this (and don't duplicate - read directly from
  // text buffer).
  speech.resize(0);

#ifndef _WIN32_WCE
  // TODO: Generalise actions, implement those present in Linux
  // version.
  m_pActionSpeech = new CActionSpeech;
  m_pActionSpeech->Activate();
#else
  m_pActionSpeech = 0;
#endif

}

HWND CEdit::Create(HWND hParent, bool bNewWithDate) {
  m_hWnd = CWindowImpl<CEdit>::Create(hParent, NULL, NULL, ES_NOHIDESEL | WS_CHILD | ES_MULTILINE | WS_VSCROLL | WS_VISIBLE, WS_EX_CLIENTEDGE);

  Tstring WindowTitle;
  WinLocalisation::GetResourceString(IDS_APP_TITLE, &WindowTitle);
  m_FilenameGUI = new CFilenameGUI(hParent, WindowTitle.c_str(), bNewWithDate);
  
  return m_hWnd;
}


CEdit::~CEdit() {
  DeleteObject(m_Font);

  delete m_FilenameGUI;
  if(FileHandle != INVALID_HANDLE_VALUE)
    CloseHandle(FileHandle);

#ifndef _WIN32_WCE
  m_pActionSpeech->Deactivate();
  delete m_pActionSpeech;
#endif
}

void CEdit::Move(int x, int y, int Width, int Height) {
  MoveWindow( x, y, Width, Height, TRUE);
}

void CEdit::New(const string &filename) {
  Tstring newFilename;
  UTF8string_to_wstring(filename, newFilename);
  TNew(newFilename);
}

bool CEdit::Open(const string &filename) {
  Tstring openFilename;
  UTF8string_to_wstring(filename, openFilename);
  return TOpen(openFilename);
}

bool CEdit::OpenAppendMode(const string &filename) {
  // TODO: Check that this works the way it's supposed to (having
  // first figured out what that is!)
  Tstring openFilename;
  UTF8string_to_wstring(filename, openFilename);
  return TOpenAppendMode(openFilename);
}

bool CEdit::SaveAs(const string &filename) {
  Tstring saveFilename;
  UTF8string_to_wstring(filename, saveFilename);
  return TSaveAs(saveFilename);
}

bool CEdit::Save() {
  if(FileHandle == INVALID_HANDLE_VALUE) {
    if(m_filename == TEXT(""))
      return false;
    FileHandle = CreateFile(m_filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

    if(FileHandle == INVALID_HANDLE_VALUE)
      return false;
  }

  // Truncate File to 0 bytes.
  SetFilePointer(FileHandle, NULL, NULL, FILE_BEGIN);
  SetEndOfFile(FileHandle);

  // Get all the text from the edit control
  LRESULT EditLength = 1 + SendMessage( WM_GETTEXTLENGTH, 0, 0);
  TCHAR *EditText = new TCHAR[EditLength];
  EditLength = SendMessage( WM_GETTEXT, (WPARAM) EditLength, (LPARAM) EditText);

  DWORD NumberOfBytesWritten;   // Used by WriteFile

  // This is Windows therefore we tag Unicode files with BOMs (Byte Order Marks) {{{
  // Then notepad and other Windows apps can recognise the files.
  // Do NOT write BOMs in a UNIX version, they are not welcome there.
  // The BOM is just an encoding of U+FEFF (ZERO WIDTH NO-BREAK SPACE)
  // This is unambiguous as U+FFFE is not a valid Unicode character.
  // There could be a menu option for this, but most users won't know what a BOM is. }}}
  unsigned int WideLength = 0;
  wchar_t *WideText = 0;
  if((m_Encoding == Opts::UTF16LE) || (m_Encoding == Opts::UTF16BE)) {
    // These are the UTF-16 formats. If the string isn't already in UTF-16 we need
    // it to be so.
#ifdef _UNICODE
    WideLength = EditLength;
    WideText = EditText;
#else
    WideText = new wchar_t[EditLength + 1];
    WideLength = MultiByteToWideChar(CodePage, 0, EditText, -1, WideText, EditLength + 1);
#endif
  }
  switch (m_Encoding) {
  case Opts::UTF8:{            // there is no byte order, but BOM tags it as a UTF-8 file
      unsigned char BOM[3] = { 0xEF, 0xBB, 0xBF };
      WriteFile(FileHandle, &BOM, 3, &NumberOfBytesWritten, NULL);
      Tstring Tmp = EditText;
      string Output;
      wstring_to_UTF8string(EditText, Output);
      WriteFile(FileHandle, Output.c_str(), Output.size(), &NumberOfBytesWritten, NULL);
      break;
    }
  case Opts::UTF16LE:{
      // TODO I am assuming this machine is LE. Do any windows (perhaps CE) machines run on BE?
      unsigned char BOM[2] = { 0xFF, 0xFE };
      WriteFile(FileHandle, &BOM, 2, &NumberOfBytesWritten, NULL);
      WriteFile(FileHandle, WideText, WideLength * 2, &NumberOfBytesWritten, NULL);
#ifndef _UNICODE
      delete[]WideText;
#endif
      break;
    }
  case Opts::UTF16BE:{         // UTF-16BE
      // TODO I am again assuming this machine is LE.
      unsigned char BOM[2] = { 0xFE, 0xFF };
      WriteFile(FileHandle, &BOM, 2, &NumberOfBytesWritten, NULL);
      // There will be a better way. Perhaps use _swab instead.
      for(unsigned int i = 0; i < WideLength; i++) {
        const char *Hack = (char *)&WideText[i];
        WriteFile(FileHandle, Hack + 1, 1, &NumberOfBytesWritten, NULL);
        WriteFile(FileHandle, Hack, 1, &NumberOfBytesWritten, NULL);
      }
#ifndef _UNICODE
      delete[]WideText;
#endif
      break;
    }
  default:
#ifdef _UNICODE
    char *MultiByteText = new char[EditLength * 4];
    int MultiByteLength = WideCharToMultiByte(CodePage, 0, EditText, EditLength, MultiByteText, EditLength * 4, NULL, NULL);
    WriteFile(FileHandle, MultiByteText, MultiByteLength, &NumberOfBytesWritten, NULL);
    delete[]MultiByteText;
#else
    WriteFile(FileHandle, EditText, EditLength, &NumberOfBytesWritten, NULL);
#endif
    break;                      // do nothing
  }

  delete[]EditText;
  // The file handle is not closed here. We keep a write-lock on the file to stop other programs confusing us.

  m_FilenameGUI->SetDirty(false);
  m_dirty = false;
  return true;
}

void CEdit::New() {
  switch (m_FilenameGUI->QuerySaveFirst()) {
  case IDYES:
    if(!Save())
      if(!TSaveAs(m_FilenameGUI->SaveAs()))
        return;
    break;
  case IDNO:
    break;
  default:
    return;
  }
  TNew(TEXT(""));
}

void CEdit::Open() {
  switch (m_FilenameGUI->QuerySaveFirst()) {
  case IDYES:
    if(!Save())
      if(!TSaveAs(m_FilenameGUI->SaveAs()))
        return;
    break;
  case IDNO:
    break;
  default:
    return;
    break;
  }
  TOpen(m_FilenameGUI->Open());
}

void CEdit::OpenAppendMode() {
  switch (m_FilenameGUI->QuerySaveFirst()) {
  case IDYES:
    if(!Save())
      if(!TSaveAs(m_FilenameGUI->SaveAs()))
        return;
    break;
  case IDNO:
    break;
  default:
    return;
    break;
  }
  TOpenAppendMode(m_FilenameGUI->Open());
}

void CEdit::SaveAs() {
  TSaveAs(m_FilenameGUI->SaveAs());
}

std::string CEdit::Import() {
  string filename;
  wstring_to_UTF8string(m_FilenameGUI->Open(), filename);
  return filename;
}

void CEdit::SetDirty() {
  m_dirty = true;
  m_FilenameGUI->SetDirty(true);
}

void CEdit::TNew(const Tstring &filename) {
  // TODO: Send a message to the parent to say that the buffer has
  // changed (as in the Linux version).

  if(filename == TEXT(""))
    m_filename = m_FilenameGUI->New();
  else
    m_filename = filename;
  if(FileHandle != INVALID_HANDLE_VALUE)
    CloseHandle(FileHandle);
  FileHandle = INVALID_HANDLE_VALUE;
  AppendMode = false;
  Clear();
}

bool CEdit::TOpen(const Tstring &filename) {
  // Could try and detect unicode formats from BOMs like notepad.
  // Could also base codepage on menu.
  // Best thing is probably to trust any BOMs at the beginning of file, but otherwise
  // to believe menu. Unicode files don't necessarily have BOMs, especially from Unix.

  HANDLE TmpHandle = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                (HANDLE) NULL);

  if(TmpHandle == INVALID_HANDLE_VALUE)
    return false;

  if(FileHandle != INVALID_HANDLE_VALUE)
    CloseHandle(FileHandle);
  FileHandle = TmpHandle;
  m_filename = filename;

  SetFilePointer(FileHandle, NULL, NULL, FILE_BEGIN);

  DWORD filesize = GetFileSize(FileHandle, NULL);
  unsigned long amountread;

  char *filebuffer = new char[filesize];

  // Just read in whole file as char* and cast later.

  ReadFile(FileHandle, filebuffer, filesize, &amountread, NULL);

  string text;
  text = text + filebuffer;
  Tstring inserttext;
  UTF8string_to_wstring(text, inserttext);
  InsertText(inserttext);

  AppendMode = false;
  m_FilenameGUI->SetFilename(m_filename);
  m_FilenameGUI->SetDirty(false);
  m_dirty = false;
  return true;
}

bool CEdit::TOpenAppendMode(const Tstring &filename) {
  AppendMode = true;
  return true;
}

bool CEdit::TSaveAs(const Tstring &filename) {
  HANDLE TmpHandle = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                (HANDLE) NULL);

  if(TmpHandle == INVALID_HANDLE_VALUE)
    return false;

  if(FileHandle != INVALID_HANDLE_VALUE)
    CloseHandle(FileHandle);
  FileHandle = TmpHandle;

  m_filename = filename;
  if(Save()) {
    m_FilenameGUI->SetFilename(m_filename);
    return true;
  }
  else
    return false;
}

void CEdit::Cut() {
  SendMessage(WM_CUT, 0, 0);
}

void CEdit::Copy() {
  SendMessage(WM_COPY, 0, 0);
/*
#ifndef _UNICODE
	HGLOBAL handle;
	DWORD* foo;
	handle = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DWORD));
	foo = (DWORD*) GlobalLock(handle);
	*foo = MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), SORT_DEFAULT);
	GlobalUnlock(handle);
	OpenClipboard(m_hwnd);
	SetClipboardData(CF_LOCALE, handle);
	CloseClipboard();
#endif
*/
}

void CEdit::CopyAll() {
  // One might think this would lead to flickering of selecting and
  // unselecting. It doesn't seem to. Using the clipboard directly
  // is fiddly, so this cheat is useful.
  DWORD start, finish;
  SendMessage(EM_GETSEL, (LONG) & start, (LONG) & finish);
  SendMessage(EM_SETSEL, 0, -1);
  SendMessage(WM_COPY, 0, 0);
  SendMessage(EM_SETSEL, (LONG) start, (LONG) finish);
}

void CEdit::Paste() {
  SendMessage(WM_PASTE, 0, 0);
}

void CEdit::SelectAll() {
  SendMessage(EM_SETSEL, 0, -1);
}

void CEdit::Clear() {
  SendMessage(WM_SETTEXT, 0, (LPARAM) TEXT(""));
  speech.resize(0);
}

void CEdit::SetEncoding(Dasher::Opts::FileEncodingFormats Encoding) {
  m_Encoding = Encoding;
}

void CEdit::SetFont(string Name, long Size) {
#ifndef _WIN32_WCE
  m_FontName = Name;
  m_FontSize = Size;

  Tstring FontName;
  UTF8string_to_wstring(Name, FontName);

  if(Size == 0)
    Size = 14;

  DeleteObject(m_Font);
  if(Name == "")
    m_Font = GetCodePageFont(CodePage, -Size);
  else
    m_Font = CreateFont(-Size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, FontName.c_str());    // DEFAULT_CHARSET => font made just from Size and FontName

  SendMessage(WM_SETFONT, (WPARAM) m_Font, true);
#else
  // not implemented
#pragma message ( "CEdit::SetFot not implemented on WinCE")
  //DASHER_ASSERT(0);
#endif
}

void CEdit::SetInterface(Dasher::CDasherInterfaceBase *DasherInterface) {
  m_pDasherInterface = DasherInterface;
#ifndef _WIN32_WCE
  // TODO: What on Earth is this doing here?
  //SetFont(m_FontName, m_FontSize);
#endif
}

std::string CEdit::get_context(int iOffset, int iLength) {
  TCHAR *wszContent = new TCHAR[iOffset + iLength + 1];
  
  SendMessage(WM_GETTEXT, (LONG) (iOffset + iLength + 1), (LONG) wszContent);

  std::string strReturn;
  wstring_to_UTF8string(wszContent + iOffset, strReturn);

  delete[] wszContent;

  return strReturn;
}


void CEdit::output(const std::string &sText) {
  wstring String;
  WinUTF8::UTF8string_to_wstring(sText, String);
  InsertText(String);

  if(m_pAppSettings->GetLongParameter(APP_LP_STYLE) == APP_STYLE_DIRECT) {
    const char *DisplayText = sText.c_str();
#ifdef UNICODE
    if(DisplayText[0] == 0xd && DisplayText[1] == 0xa) {
      // Newline, so we want to fake an enter
      fakekey[0].type = fakekey[1].type = INPUT_KEYBOARD;
      fakekey[0].ki.wVk = fakekey[1].ki.wVk = VK_RETURN;
      fakekey[0].ki.time = fakekey[1].ki.time = 0;
      fakekey[1].ki.dwFlags = KEYEVENTF_KEYUP;
      SendInput(2, fakekey, sizeof(INPUT));
    }
    else {    
      for(std::wstring::iterator it(String.begin()); it != String.end(); ++it) {
        fakekey[0].type = INPUT_KEYBOARD;
#ifdef DASHER_WINCE
        fakekey[0].ki.dwFlags = KEYEVENTF_KEYUP;
#else
        fakekey[0].ki.dwFlags = KEYEVENTF_UNICODE;
#endif
        fakekey[0].ki.wVk = 0;
        fakekey[0].ki.time = NULL;
        fakekey[0].ki.wScan = *it;
        SendInput(1, fakekey, sizeof(INPUT));
      }
    }
#else
    if(DisplayText[0] == 0xd && DisplayText[1] == 0xa) {
      // Newline, so we want to fake an enter
      SetFocus(targetwindow);
      keybd_event(VK_RETURN, 0, NULL, NULL);
      keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, NULL);
    }
    Tstring character;
    WinUTF8::UTF8string_to_wstring(DisplayText, &character, 1252);
    TCHAR test = character[0];
    SHORT outputvk = VkKeyScan(char (character[0]));
    SetFocus(targetwindow);
    if(HIBYTE(outputvk) && 6) {
      keybd_event(VK_SHIFT, 0, NULL, NULL);
      keybd_event(LOBYTE(outputvk), 0, NULL, NULL);
      keybd_event(LOBYTE(outputvk), 0, KEYEVENTF_KEYUP, NULL);
      keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, NULL);
    }
    else {
      keybd_event(LOBYTE(outputvk), 0, NULL, NULL);
      keybd_event(LOBYTE(outputvk), 0, KEYEVENTF_KEYUP, NULL);
    }
#endif
  }
  m_Output += sText;

  UTF8string_to_wstring(sText, newchar);
  speech += newchar;

  // Slightly hacky word by word preview
  if(newchar == L" ") {
    if(m_pAppSettings->GetBoolParameter(APP_BP_SPEECH_WORD) && m_pActionSpeech->GetActive())
      m_pActionSpeech->Preview(m_strCurrentWord);
    m_strCurrentWord = L"";
  } 
  else {
    m_strCurrentWord += newchar;
  }
}

void CEdit::Move(int iDirection, int iDist) {

  // Unfortunately there doesn't seem to be a sane way of obtaining the caret
  // position (as opposed to the bounds of the selection), so we're just going
  // to have to assume that the caret is at the end...

  int iStart;
  int iEnd;
  SendMessage(EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

//  int iStartLine;
  int iEndLine;
//  int iLineOffset;
//  int iLineLength;
//  int iLineStart;
//  int iNumLines;
  int iNumChars;

  HLOCAL hMemHandle;
  std::wstring strBufferText;

  if(iDirection == EDIT_FORWARDS) {
    switch(iDist) {
    case EDIT_CHAR:
      //if(iStart != iEnd)
        ++iEnd;
      iStart = iEnd;
      break;
    case EDIT_WORD:
      // Hmm... words are hard - this is a rough and ready approximation:

#ifndef _WIN32_WCE
      // TODO: Fix this on Windows CE
      iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      hMemHandle = (HLOCAL)SendMessage( EM_GETHANDLE, 0, 0);
      strBufferText = std::wstring((WCHAR*)LocalLock(hMemHandle), iNumChars);
      LocalUnlock(hMemHandle);

      iEnd = strBufferText.find(' ', iEnd+1);
      if(iEnd == -1)
        iEnd = iNumChars + 1;
      else
        iEnd = iEnd + 1;
      iStart = iEnd;
#endif
      break;
    case EDIT_LINE:
/*      iEndLine = SendMessage( EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
      iLineOffset = iEnd - SendMessage( EM_LINEINDEX, (WPARAM)iEndLine, 0);
      iNumLines = SendMessage( EM_GETLINECOUNT, 0, 0);
      if( iEndLine < iNumLines - 1) {
        ++iEndLine;
        iLineStart = SendMessage( EM_LINEINDEX, (WPARAM)iEndLine, 0);
        iLineLength = SendMessage( EM_LINELENGTH, (WPARAM)iEndLine, 0);
        if( iLineOffset < iLineLength )
          iEnd = iLineStart+iLineOffset;
        else
          iEnd = iLineStart+iLineLength;
      }
	  else if(iEndLine == iNumLines - 1) {
		// we're on the last line so go to end of file
		iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
        iEnd = iNumChars + 1;
      }
*/    
      // Make it behave like the 'End' key, unless we're at the end of the current line.
	  // Then go down a line.
	  iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 1), 0) - 1; // end of this line
	  if(iStart==iEnd)  // we were already at the end so go down a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 2), 0) - 1;
	  iStart = iEnd;
      break;
    case EDIT_FILE: 
      iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      iEnd = iNumChars + 1;
      iStart = iEnd;
      break;
    }
  }
  else {
    switch(iDist) {
    case EDIT_CHAR:
      if( iStart == iEnd )
        --iStart;
      iEnd = iStart;
      break;
    case EDIT_WORD:
#ifndef _WIN32_WCE
      // TODO: Fix this on Windows CE
      iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      hMemHandle = (HLOCAL)SendMessage(EM_GETHANDLE, 0, 0);
      strBufferText = std::wstring((WCHAR*)LocalLock(hMemHandle), iNumChars);
      LocalUnlock(hMemHandle);

      if(iEnd > 0) {
        iEnd = strBufferText.rfind(' ', iEnd-2);
        if(iEnd == -1)
          iEnd = 0;
        else
          iEnd = iEnd + 1;
      }
      iStart = iEnd;
#endif
      break;
    case EDIT_LINE:
/*
      iStartLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iStart, 0);
      iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
      iLineOffset = iEnd - SendMessage(EM_LINEINDEX, (WPARAM)iEndLine, 0);
      if( iStartLine > 0)
        --iStartLine;
	  else if( iStartLine == 0)
	  {
	    // we're on the first line so go to start of file...
	    iStart = iEnd = 0;
		break;
	  }
      iLineStart = SendMessage(EM_LINEINDEX, (WPARAM)iStartLine, 0);
      iLineLength = SendMessage(EM_LINELENGTH, (WPARAM)iStartLine, 0);
      if( iLineOffset < iLineLength )
        iStart = iLineStart+iLineOffset;
      else
        iStart = iLineStart+iLineLength;
*/
	  iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine), 0); // start of this line
	  if(iStart==iEnd)  // we were already at the start so go up a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine - 1), 0);
	  iStart = iEnd;
      break;
    case EDIT_FILE:
      iStart = 0;
      iEnd = 0;
      break;
    }
  }

  SendMessage(EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
  SendMessage(EM_SCROLLCARET, 0, 0); //scroll the caret into view!
}

void CEdit::Delete(int iDirection, int iDist) {
  int iStart;
  int iEnd;
  int iEndLine;
//  int iLineOffset;
//  int iLineLength;
//  int iLineStart;
//  int iNumLines;
  int iNumChars;

  HLOCAL hMemHandle;
  std::wstring strBufferText;

  SendMessage(EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

  if(iDirection == EDIT_FORWARDS) {
    switch(iDist) {
    case EDIT_CHAR:
      ++iEnd;
      break;
    case EDIT_WORD:
#ifndef _WIN32_WCE
      // TODO: Fix in Windows CE
      iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      hMemHandle = (HLOCAL)SendMessage(EM_GETHANDLE, 0, 0);
      strBufferText = std::wstring((WCHAR*)LocalLock(hMemHandle), iNumChars);
      LocalUnlock(hMemHandle);

      iEnd = strBufferText.find(' ', iEnd+1);
      if(iEnd == -1)
        iEnd = iNumChars + 1;
#endif
      break;
    case EDIT_LINE:
/*
      iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
      iLineOffset = iEnd - SendMessage(EM_LINEINDEX, (WPARAM)iEndLine, 0);
      iNumLines = SendMessage(EM_GETLINECOUNT, 0, 0);
      if( iEndLine < iNumLines - 1) {
        ++iEndLine;
        iLineStart = SendMessage(EM_LINEINDEX, (WPARAM)iEndLine, 0);
        iLineLength = SendMessage(EM_LINELENGTH, (WPARAM)iEndLine, 0);
        if( iLineOffset < iLineLength )
          iEnd = iLineStart+iLineOffset;
        else
          iEnd = iLineStart+iLineLength;
      }
  */
	  iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 1), 0); // end of this line
	  if(iStart==iEnd)  // we were already at the end so go down a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 2), 0);
      break;
    case EDIT_FILE: 
      iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      iEnd = iNumChars + 1;
      break;
    }
  }
  else {
    switch(iDist) {
    case EDIT_CHAR:
      --iEnd;
      break;
    case EDIT_WORD:
#ifndef _WIN32_WCE
      iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      hMemHandle = (HLOCAL)SendMessage(EM_GETHANDLE, 0, 0);
      strBufferText = std::wstring((WCHAR*)LocalLock(hMemHandle), iNumChars);
      LocalUnlock(hMemHandle);

      if(iEnd > 0) {
        iEnd = strBufferText.rfind(' ', iEnd-2);
        if(iEnd == -1)
          iEnd = 0;
        else
          iEnd = iEnd + 1;
      }
#endif
      break;
    case EDIT_LINE:
/*       iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
      iLineOffset = iEnd - SendMessage(EM_LINEINDEX, (WPARAM)iEndLine, 0);
      iNumLines = SendMessage(EM_GETLINECOUNT, 0, 0);
      if(iEndLine > 0) {
        --iEndLine;
        iLineStart = SendMessage(EM_LINEINDEX, (WPARAM)iEndLine, 0);
        iLineLength = SendMessage(EM_LINELENGTH, (WPARAM)iEndLine, 0);
        if( iLineOffset < iLineLength )
          iEnd = iLineStart+iLineOffset;
        else
          iEnd = iLineStart+iLineLength;
      }
	  */
	  iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine), 0); // start of this line
	  if(iStart==iEnd)  // we were already at the start so go up a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine - 1), 0);

      break;
    case EDIT_FILE:
      iEnd = 0;
      break;
    }
  }

  SendMessage(EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
  SendMessage(EM_REPLACESEL, (WPARAM)true, (LPARAM)"");
}

/////////////////////////////////////////////////////////////////////////////

void CEdit::SetKeyboardTarget(HWND hwnd) 
{
  m_bForwardKeyboard = true;
  m_hTarget = hwnd;
}

LRESULT CEdit::OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  m_bForwardKeyboard = false;
  bHandled = FALSE; // let the EDIT class handle it
  return 0;
}

LRESULT CEdit::OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  int iStart;
  int iEnd;
  
  SendMessage(EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

  m_pDasherInterface->SetOffset(iStart);

  bHandled = FALSE; // let the EDIT class handle it
  return 0;
}

HRESULT CEdit::OnChar(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if(!m_bForwardKeyboard)
    bHandled = FALSE; // let the EDIT class handle it
  else
    bHandled = TRUE; // traps the message, preventing it from reaching the EDIT control
  
  return 0;
}

HRESULT CEdit::OnKeyDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if(m_bForwardKeyboard) {
    SendMessage(m_hTarget,message,wParam,lParam);
    bHandled = TRUE; // traps the message, preventing it from reaching the EDIT control
  }
  else
    bHandled = FALSE; // let the EDIT class handle it
  return 0;
}

HRESULT CEdit::OnKeyUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if(m_bForwardKeyboard) {
    SendMessage(m_hTarget,message,wParam,lParam);
    bHandled = TRUE; // traps the message, preventing it from reaching the EDIT control
  }
  else {
    // if we enter text or move around the edit control, update the Dasher display
    //if (Canvas->Running()==false) {   // FIXME - reimplement this
    //      m_pDasherInterface->ChangeEdit();
    //}
    InvalidateRect(NULL, FALSE);
    bHandled = FALSE; // let the EDIT class handle it
  }
  
  return 0;
}

HRESULT CEdit::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE;
  return SendMessage( GetParent() , message, wParam, lParam);
}

void CEdit::InsertText(Tstring InsertText) {
  SendMessage(EM_REPLACESEL, TRUE, (LPARAM) InsertText.c_str());
}

/// Delete text from the editbox

void CEdit::deletetext(const std::string &sText) {
  // Lookup the unicode string that we need to delete - we only actually 
  // need the length of the string, but this is important eg for newline
  // characters which are actually two symbols

  wstring String;
  WinUTF8::UTF8string_to_wstring(sText, String);

  int iLength(String.size());

  // Get the start and end of the current selection, and decrement the start
  // by the number of characters to be deleted

  DWORD start, finish;
  SendMessage(EM_GETSEL, (LONG) & start, (LONG) & finish);
  start -= iLength;
  SendMessage(EM_SETSEL, (LONG) start, (LONG) finish);

  // Replace the selection with a null string

  TCHAR out[2];
  wsprintf(out, TEXT(""));
  SendMessage(EM_REPLACESEL, TRUE, (LONG) out);

  // FIXME - I *think* we still only want to send one keyboard event to delete a 
  // newline pair, but we're now assuming we'll never have two real characters for
  // a single symbol

//  if(targetwindow != NULL && textentry == true) {
if(m_pAppSettings->GetLongParameter(APP_LP_STYLE) == APP_STYLE_DIRECT) {

#ifdef _UNICODE
    fakekey[0].type = fakekey[1].type = INPUT_KEYBOARD;
    fakekey[0].ki.wVk = fakekey[1].ki.wVk = VK_BACK;
    fakekey[0].ki.time = fakekey[1].ki.time = 0;
    fakekey[1].ki.dwFlags = KEYEVENTF_KEYUP;

	::SetFocus(targetwindow);
    SendInput(2, fakekey, sizeof(INPUT));
#else
    SetFocus(targetwindow);
    keybd_event(VK_BACK, 0, NULL, NULL);
    keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, NULL);
#endif
  }

  // Shorten the speech buffer (?)
  if(speech.length() >= iLength) {
    speech.resize(speech.length() - iLength);
  }

  // Shorten the speech buffer (?)
  if(m_strCurrentWord.length() >= iLength) {
    m_strCurrentWord.resize(m_strCurrentWord.length() - iLength);
  }

  // And the output buffer (?)
  if(m_Output.length() >= iLength) {
    m_Output.resize(m_Output.length() - iLength);
  }
}

void CEdit::speak(int what) {
  if(!m_pActionSpeech->GetActive())
    return;

  // TODO: The remainder of this function is somewhat horrible and hacky...
  
  // TODO: Horrible hack - don't speak in direct entry mode
  if(m_pAppSettings->GetLongParameter(APP_LP_STYLE) == APP_STYLE_DIRECT)
    return;

  std::wstring strSpeech;

  if(what == 1) { // All
    int speechlength = GetWindowTextLength();
    LPTSTR allspeech = new TCHAR[speechlength + 1];
    GetWindowText(allspeech, speechlength + 1);
    strSpeech = allspeech;
    lastspeech = allspeech;
    delete allspeech;
    speech.resize(0);
  }
  else if(what == 2) { // New
    strSpeech = speech;
    lastspeech = speech;
    speech.resize(0);
  }
  else if(what == 3) {
    strSpeech = lastspeech;
  }

  m_pActionSpeech->Execute(strSpeech);
}

void CEdit::SetNewWithDate(bool bNewWithDate) {
  if(m_FilenameGUI)
    m_FilenameGUI->SetNewWithDate(bNewWithDate);
}

void CEdit::HandleEvent(Dasher::CEvent *pEvent) {
  switch(pEvent->m_iEventType) {
  case EV_PARAM_NOTIFY:
    HandleParameterChange(((CParameterNotificationEvent *)pEvent)->m_iParameter);
    break;
  case EV_EDIT:
    HandleEditEvent(pEvent);
    break;
  case EV_STOP:
    HandleStop();
    break;
  case EV_EDIT_CONTEXT:
    // TODO: Make this return the context properly. cf Linux 
    //m_pDasherInterface->SetContext("");
    break;
  }
}

void CEdit::HandleParameterChange(int iParameter) {
  switch(iParameter) {
  case APP_SP_EDIT_FONT:
  case APP_LP_EDIT_FONT_SIZE:
    SetFont(m_pAppSettings->GetStringParameter(APP_SP_EDIT_FONT), m_pAppSettings->GetLongParameter(APP_LP_EDIT_FONT_SIZE));
    break;
  default:
    break;
  }
}

void CEdit::HandleEditEvent(Dasher::CEvent *pEvent) {
  Dasher::CEditEvent * pEvt(static_cast< Dasher::CEditEvent * >(pEvent));

  switch (pEvt->m_iEditType) {
    case 1:
      output(pEvt->m_sText);
      break;
    case 2:
      deletetext(pEvt->m_sText);
      break;
  }
}

void CEdit::HandleStop() {
  // TODO: These should be more generally implemented as 
  if(m_pAppSettings->GetBoolParameter(APP_BP_SPEECH_MODE))
    speak(2);

  if(m_pAppSettings->GetBoolParameter(APP_BP_COPY_ALL_ON_STOP))
    CopyAll();
}
