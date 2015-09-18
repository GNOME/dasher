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
}

void CEdit::Move(int x, int y, int Width, int Height) {
  MoveWindow( x, y, Width, Height, TRUE);
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

  CString wideText;
  GetWindowText(wideText);
  CStringA mbcsText(wideText);
  DWORD NumberOfBytesWritten;   // Used by WriteFile
  WriteFile(FileHandle, mbcsText, mbcsText.GetLength(), &NumberOfBytesWritten, NULL);
  // The file handle is not closed here. We keep a write-lock on the file to stop other programs confusing us.

  m_FilenameGUI->SetDirty(false);
  m_dirty = false;
  return true;
}

bool CEdit::ConfirmAndSaveIfNeeded() {
  if (!m_pAppSettings->GetBoolParameter(APP_BP_CONFIRM_UNSAVED))
    return true;
  
  switch (m_FilenameGUI->QuerySaveFirst()) {
  case IDYES:
    if (!Save())
      if (!TSaveAs(m_FilenameGUI->SaveAs()))
        return false;
    break;
  case IDNO:
    break;
  default:
    return false;
  }
  return true;
}

void CEdit::New() {
  if (ConfirmAndSaveIfNeeded())
    TNew(TEXT(""));
}

void CEdit::Open() {
  if (ConfirmAndSaveIfNeeded())
    TOpen(m_FilenameGUI->Open());
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

  m_FilenameGUI->SetFilename(m_filename);
  m_FilenameGUI->SetDirty(false);
  m_dirty = false;
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
}

void CEdit::Paste() {
  SendMessage(WM_PASTE, 0, 0);
}

void CEdit::SelectAll() {
  SendMessage(EM_SETSEL, 0, -1);
}

void CEdit::Clear() {
  SendMessage(WM_SETTEXT, 0, (LPARAM) TEXT(""));
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
#ifdef _WIN32_WCE
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
}

int CEdit::Move(bool bForwards, CControlManager::EditDistance iDist) {

  // Unfortunately there doesn't seem to be a sane way of obtaining the caret
  // position (as opposed to the bounds of the selection), so we're just going
  // to have to assume that the caret is at the end...

  int iStart;
  int iEnd;
  SendMessage(EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

  HLOCAL hMemHandle;
  std::wstring strBufferText;

  if(bForwards) {
    switch(iDist) {
    case CControlManager::EDIT_CHAR:
      //if(iStart != iEnd)
        ++iEnd;
      break;
    case CControlManager::EDIT_WORD: {
      // Hmm... words are hard - this is a rough and ready approximation:

#ifndef _WIN32_WCE
      // TODO: Fix this on Windows CE
      int iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      hMemHandle = (HLOCAL)SendMessage( EM_GETHANDLE, 0, 0);
      strBufferText = std::wstring((WCHAR*)LocalLock(hMemHandle), iNumChars);
      LocalUnlock(hMemHandle);

      iEnd = strBufferText.find(' ', iEnd+1);
      if(iEnd == -1)
        iEnd = iNumChars + 1;
      else
        iEnd = iEnd + 1;
#endif
    }
      break;
    case CControlManager::EDIT_LINE: {
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
	  int iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 1), 0) - 1; // end of this line
	  if(iStart==iEnd)  // we were already at the end so go down a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 2), 0) - 1;
    }
      break;
    case CControlManager::EDIT_FILE: {
      int iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      iEnd = iNumChars + 1;
    }
      break;
    }
  }
  else {
    switch(iDist) {
    case CControlManager::EDIT_CHAR:
        //ACL this case at least differs from Delete(bool,EditDistance):
        // there we decrement iEnd whether or not iStart==iEnd.
      if( iStart == iEnd )
        --iEnd;
      break;
    case CControlManager::EDIT_WORD: {
#ifndef _WIN32_WCE
      // TODO: Fix this on Windows CE
      int iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
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
    }
      break;
    case CControlManager::EDIT_LINE: {
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
	  int iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine), 0); // start of this line
	  if(iStart==iEnd)  // we were already at the start so go up a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine - 1), 0);
    }
      break;
    case CControlManager::EDIT_FILE:
      iEnd = 0;
      break;
    }
  }
  iStart = iEnd;
  SendMessage(EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
  SendMessage(EM_SCROLLCARET, 0, 0); //scroll the caret into view!
  return iStart;
}

int CEdit::Delete(bool bForwards, CControlManager::EditDistance iDist) {
  int iStart;
  int iEnd;

  HLOCAL hMemHandle;
  std::wstring strBufferText;

  SendMessage(EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

  if(bForwards) {
    switch(iDist) {
    case CControlManager::EDIT_CHAR:
      ++iEnd;
      break;
    case CControlManager::EDIT_WORD: {
#ifndef _WIN32_WCE
      // TODO: Fix in Windows CE
      int iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      hMemHandle = (HLOCAL)SendMessage(EM_GETHANDLE, 0, 0);
      strBufferText = std::wstring((WCHAR*)LocalLock(hMemHandle), iNumChars);
      LocalUnlock(hMemHandle);

      iEnd = strBufferText.find(' ', iEnd+1);
      if(iEnd == -1)
        iEnd = iNumChars + 1;
#endif
    }  
      break;
    case CControlManager::EDIT_LINE: {
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
	  int iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 1), 0); // end of this line
	  if(iStart==iEnd)  // we were already at the end so go down a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine + 2), 0);
    }
      break;
    case CControlManager::EDIT_FILE: {
      int iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
      iEnd = iNumChars + 1;
    }
      break;
    }
  }
  else {
    switch(iDist) {
    case CControlManager::EDIT_CHAR:
        //ACL this case at least differs from that for Move(bool, EditDistance):
        // there we only decrement if iStart==iEnd.
      --iEnd;
      break;
    case CControlManager::EDIT_WORD: {
#ifndef _WIN32_WCE
      int iNumChars = SendMessage(WM_GETTEXTLENGTH, 0, 0);
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
    }
      break;
    case CControlManager::EDIT_LINE: {
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
	  int iEndLine = SendMessage(EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
	  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine), 0); // start of this line
	  if(iStart==iEnd)  // we were already at the start so go up a line
		  iEnd = SendMessage(EM_LINEINDEX, (WPARAM)(iEndLine - 1), 0);
    }
      break;
    case CControlManager::EDIT_FILE:
      iEnd = 0;
      break;
    }
  }

  SendMessage(EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
  SendMessage(EM_REPLACESEL, (WPARAM)true, (LPARAM)TEXT(""));
  return min(iStart, iEnd);
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

  // And the output buffer (?)
  if(m_Output.length() >= iLength) {
    m_Output.resize(m_Output.length() - iLength);
  }
}

void CEdit::SetNewWithDate(bool bNewWithDate) {
  if(m_FilenameGUI)
    m_FilenameGUI->SetNewWithDate(bNewWithDate);
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
