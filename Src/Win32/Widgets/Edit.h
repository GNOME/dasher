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

#ifndef __Edit_h__
#define __Edit_h__

#define _ATL_APARTMENT_THREADED
#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override something, 

//but do not change the name of _Module

extern CComModule _Module;

#include <atlcom.h>

#include "../AppSettings.h"
#include "../DasherAction.h"
#include "../../DasherCore/DasherTypes.h"

#include <Oleacc.h>

class CCanvas;
class CFilenameGUI;

namespace Dasher {
  class CDasherInterfaceBase;
  class CEvent;
};

enum {
  EDIT_FORWARDS,
  EDIT_BACKWARDS
};

enum {
  EDIT_CHAR,
  EDIT_WORD,
  EDIT_LINE,
  EDIT_FILE
};

class CEdit : public ATL::CWindowImpl<CEdit> {
 public:
  
  CEdit(CAppSettings *pAppSettings);
  ~CEdit();
  
  HWND Create(HWND hParent, bool bNewWithDate);
  
  // Superclass the built-in EDIT window class
  DECLARE_WND_SUPERCLASS(NULL, _T("EDIT") )
    
  BEGIN_MSG_MAP( CEdit )
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
    MESSAGE_HANDLER(WM_CHAR, OnChar)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
  END_MSG_MAP()
    
    
  HRESULT OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  HRESULT OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  HRESULT OnChar(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  HRESULT OnKeyDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  HRESULT OnKeyUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  HRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


  void Move(int x, int y, int Width, int Height);
	
  HWND GetHwnd() {
    return m_hWnd;
  } 
  
  // As EN_UPDATE message go to parent, need this. void UserSave(HANDLE FileHandle);
  void UserOpen(HANDLE FileHandle);
  
  void Move(int iDirection, int iDist);
  void Delete(int iDirection, int iDist);
  void SetKeyboardTarget(HWND hwnd);
  
  // Overriding file virtual functions
  //void TimeStampNewFiles(bool Value);
  void New(const std::string & filename);
  bool Open(const std::string & filename);
  bool OpenAppendMode(const std::string & filename);
  bool SaveAs(const std::string & filename);
  bool Save();
  // Functions for Windows GUI to call
  void New();
  void Open();
  void OpenAppendMode();
  void SaveAs();
  std::string Import();
  void SetDirty();              // Parent window gets notification Edit window has changed.
  
  void Cut();
  void Copy();
  void CopyAll();
  void Paste();
  void SelectAll();
  void Clear();
  
  void SetEncoding(Dasher::Opts::FileEncodingFormats Encoding);
  void SetFont(std::string Name, long Size);
  
  void SetInterface(Dasher::CDasherInterfaceBase * DasherInterface);
  
  // Get context (new version)
  std::string get_context(int iOffset, int iLength);

  // called when characters fall of the LHS of the screen
  void output(const std::string & sText);
    
  // remove the previous character
  void deletetext(const std::string & sText);
  
  // speak text
  void speak(int what);
  
  void SetNewWithDate(bool bNewWithDate);
  void HandleEvent(Dasher::CEvent *pEvent);

 protected:
  bool m_dirty;
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
  
 private:
  void HandleParameterChange(int iParameter);
  void HandleEditEvent(Dasher::CEvent *pEvent);
  void HandleStop();
  
  Dasher::CDasherInterfaceBase *m_pDasherInterface;
  
  HWND Parent;
  WNDPROC TextWndFunc;
  
  HWND m_hTarget;
  bool m_bForwardKeyboard;
  
  HANDLE FileHandle;            // Keeping a lock on files makes File I/O safer,
  // especially for the append mode!
  CFilenameGUI *m_FilenameGUI;
  Tstring m_filename;
  HWND textwindow;
  bool AppendMode;
  void TNew(const Tstring & filename);
  bool TOpen(const Tstring & filename);
  bool TOpenAppendMode(const Tstring & filename);
  bool TSaveAs(const Tstring & filename);
  
  HFONT m_Font;
  std::string m_FontName;
  long m_FontSize;
  
  std::string m_Output;         // UTF-8 to go to training file
  UINT CodePage;                // for font and possible for finding the encoding
  Dasher::Opts::FileEncodingFormats m_Encoding; // file encoding option (may say to use codepage or user setting)
  
  DWORD threadid;
  HWND targetwindow;
  bool textentry;
#ifdef _UNICODE
  INPUT fakekey[2];
#endif
    
  Tstring speech;
  Tstring lastspeech;
  Tstring newchar;
  
  void InsertText(Tstring InsertText);  // add symbol to edit control
  
  CAppSettings *m_pAppSettings;
  HWND m_hWnd;
  
  CDasherAction *m_pActionSpeech;

  std::wstring m_strCurrentWord;
};

#endif /* #ifndef __Edit_h__ */
