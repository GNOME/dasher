#ifndef __GameGroup_h__
#define __GameGroup_h__

#include "../Common/WinCommon.h"
#include "../../DasherCore/DasherInterfaceBase.h"

#include <atlbase.h>
#include <atlwin.h>

class CGameGroup : public ATL::CWindowImpl<CGameGroup> {
public:
  CGameGroup(CDasherInterfaceBase *pDasherInterface);

  // ATL boilerplate code
  DECLARE_WND_CLASS_EX(NULL, 0, COLOR_ACTIVECAPTION);
  BEGIN_MSG_MAP(CGameGroup)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SHOWWINDOW, OnShow)
  END_MSG_MAP()

  // Message handlers:
  LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnShow(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


  // Create the window, with children
  HWND Create(HWND hParent);

  int GetHeight() {
    return bVisible?m_iHeight:0;
  }

  int GetWidth() {
    return bVisible?m_iWidth:0;
  }
  int GetLabelHeight() {
    return bVisible?m_iLabelHeight:0;
  }
  void Message(int message, const void * messagedata);
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
  //void SelectAlphabet();

  // Update the contents of the speed control
  void PopulateSpeed();

  // Update Dasher and the edit box to represent the current speed
  void UpdateSpeed(int iPos, int iDelta);

  // The Dasher interface with which this control communicates
  CDasherInterfaceBase *m_pDasherInterface;

  // Handles to child windows
  HWND m_hLevelEdit;
  HWND m_hLevelLabel;
  HWND m_hScoreEdit;
  HWND m_hScoreLabel;
  CWindow* m_pDemoButton;
  SIZE m_sDemoSize;
  SIZE m_sNextSize;
  CWindow* m_pNextButton;
  HWND m_hGameTextLabel;

  bool bVisible;
  int m_iHeight;
  int m_iLabelHeight;
  int m_iWidth;
};

#endif