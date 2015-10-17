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
#include "../../DasherCore/ControlManager.h"
#include <Oleacc.h>

class CCanvas;
class CFilenameGUI;

namespace Dasher {
  class CDasherInterfaceBase;
  class CEvent;
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
  
  int Move(bool bForwards, Dasher::CControlManager::EditDistance iDist);
  int Delete(bool bForwards, Dasher::CControlManager::EditDistance iDist);
  void SetKeyboardTarget(HWND hwnd);
  bool ConfirmAndSaveIfNeeded();
  bool Save();
  // Functions for Windows GUI to call
  void New();
  void Open();
  void SaveAs();
  std::string Import();
  void SetDirty();              // Parent window gets notification Edit window has changed.
  
  void Cut();
  void Copy();
  void Paste();
  void SelectAll();
  void Clear();
  
  void SetFont(std::string Name, long Size);
  
  void SetInterface(Dasher::CDasherInterfaceBase * DasherInterface);
  
  // called when a new character falls under the crosshair
  void output(const std::string & sText);
    
  // remove the previous character: called when we steer/reverse the crosshair out of a node
  void deletetext(const std::string & sText);
  
  void SetNewWithDate(bool bNewWithDate);

  //ACL Making these public so can be called directly from CDasher
  void HandleParameterChange(int iParameter);

 protected:
  bool m_dirty;
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
  
 private:  
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
  void TNew(const Tstring & filename);
  bool TOpen(const Tstring & filename);
  bool TSaveAs(const Tstring & filename);
  void GetRange(bool bForwards, Dasher::CControlManager::EditDistance iDist, int* iStart, int* iEnd);
  
  HFONT m_Font;
  std::string m_FontName;
  long m_FontSize;
  
  std::string m_Output;         // UTF-8 to go to training file
  UINT CodePage;                // for font and possible for finding the encoding
  
  DWORD threadid;
  HWND targetwindow;
  bool textentry;
#ifdef _UNICODE
  INPUT fakekey[2];
#endif
  
  void InsertText(Tstring InsertText);  // add symbol to edit control
  
  CAppSettings *m_pAppSettings;
  HWND m_hWnd;
};

#endif /* #ifndef __Edit_h__ */
