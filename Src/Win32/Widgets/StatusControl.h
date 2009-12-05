#ifndef __StatusControl_h__
#define __StatusControl_h__

#include "../Common/WinCommon.h"
#include "../../DasherCore/DasherInterfaceBase.h"

#include <atlbase.h>
#include <atlwin.h>

extern CONST UINT DASHER_SHOW_PREFS;
#define _DASHER_SHOW_PREFS (LPCWSTR)"dasher_show_prefs"

class CStatusControl : public ATL::CWindowImpl<CStatusControl> {
public:
	CStatusControl(Dasher::CDasherInterfaceBase *pDasherInterface);

  // ATL boilerplate code
  DECLARE_WND_SUPERCLASS(L"STATUSCONTROL", L"STATIC");

  BEGIN_MSG_MAP(CStatusControl)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
  END_MSG_MAP()

  // Message handlers:
  LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  // Create the window, with children
  HWND Create(HWND hParent);

  int GetHeight() {
    return m_iEditHeight + 2;
  }

private:
  // Create the child windows of the control
  void CreateChildren();

  // Initial layout of child windows
  void LayoutChildrenInitial();

  // Incremental update of child windows
  void LayoutChildrenUpdate();

  // Update the contents of the alphabet seletion combo
  void PopulateCombo();

  // Update Dasher to reflect the new alphabet selection
  void SelectAlphabet();

  // Update the contents of the speed control
  void PopulateSpeed();

  // Update Dasher and the edit box to represent the current speed
  void UpdateSpeed(int iPos, int iDelta);

  // The Dasher interface with which this control communicates
  Dasher::CDasherInterfaceBase *m_pDasherInterface;

  // Handles to child windows
  HWND m_hEdit;
  HWND m_hUpDown;
  HWND m_hCombo;
  HWND m_hSpeedLabel;
  HWND m_hAlphabetLabel;

  int m_iEditWidth;
  int m_iEditHeight;
};

#endif