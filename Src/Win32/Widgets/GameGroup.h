#ifndef __GameGroup_h__
#define __GameGroup_h__

#include "../Common/WinCommon.h"

#include "../../DasherCore/GameMessages.h"

#include <atlbase.h>
#include <atlwin.h>

#define ID_DEMOBUTTON 1
#define ID_NEXTBUTTON 2
#define ID_GAMELABEL 3
#define ID_SCOREEDIT 4
#define ID_LEVELEDIT 5

class CEdit;

namespace Dasher {
  class CDasherInterfaceBase;
}

class CGameGroup : public ATL::CWindowImpl<CGameGroup> {
  
public:
  CGameGroup(Dasher::CDasherInterfaceBase *pDasherInterface, CEdit* pEdit);

  // ATL boilerplate code
  DECLARE_WND_CLASS_EX(NULL, 0, COLOR_MENUBAR);
  BEGIN_MSG_MAP(CGameGroup)
    COMMAND_HANDLER(ID_DEMOBUTTON,BN_CLICKED,OnDemoClick)
    COMMAND_HANDLER(ID_NEXTBUTTON,BN_CLICKED,OnNextClick)

    MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnEditPaint)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnEditPaint)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SHOWWINDOW, OnShow)
  END_MSG_MAP()

  // Message handlers:
  LRESULT OnEditPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnShow(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnDemoClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnNextClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
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
  void SetEditFont(std::string Name, long Size);

private:
  // Create the child windows of the control
  void CreateChildren();

  // Initial layout of child windows
  void LayoutChildrenInitial();

  // Incremental update of child windows
  void LayoutChildrenUpdate();

  void UpdateTargetLabel();

  // Update Dasher to reflect the new alphabet selection
  //void SelectAlphabet();
  // The Dasher interface with which this control communicates
  Dasher::CDasherInterfaceBase *m_pDasherInterface;

  // Handles to child windows
  CWindow* m_pLevelEdit;
  CWindow* m_pLevelLabel;
  
  CWindow* m_pScoreEdit;
  CWindow* m_pScoreLabel;

  CWindow* m_pDemoButton;
  SIZE m_sDemoSize;
  SIZE m_sNextSize;
  CWindow* m_pNextButton;
  CWindow* m_pGameTextLabel;

  CEdit* m_pEdit; // The editor

  bool bVisible;
  int m_iHeight;
  int m_iLabelHeight;
  int m_iWidth;
  int m_iButtonX, m_iButtonY, m_iSpacing;
  std::string m_strTarget;
  std::string m_strOutput;
};

#endif