// DasherWindow.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherWindow_h__
#define __DasherWindow_h__

#include "resource.h"

#include "Widgets/Splitter.h"
#include "Widgets/Menu.h"
#include "Widgets/SplashScreen.h"
#include "Widgets/WindowSelect.h"
#include "DasherMouseInput.h"

#include "../../DasherCore/Win32/WinOptions.h"

class CToolbar;
class CSlidebar;
namespace Dasher {
  class CDasher;
  class CDasherComponent;
};

// Abstract interfaces to the Dasher engine
#include "../DasherCore/Win32/DasherInterface.h"

class CDasherWindow : public CWinWrap, public CSplitterOwner {
public:
  CDasherWindow();
  ~CDasherWindow();

  void Show(int nCmdShow);
  int MessageLoop();

  void SaveWindowState() const;
  bool LoadWindowState();

protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

private:
  Dasher::CDasher *m_pDasher;

  HACCEL hAccelTable;

  // Fiddly Window initialisation
  Tstring CreateMyClass();

  // Method to set values of all settings in the menu
  void PopulateSettings();

  // Widgets:
  CToolbar *m_pToolbar;
  CEdit *m_pEdit;
  CCanvas *m_pCanvas;
  CSplitter *m_pSplitter;
  CSlidebar *m_pSlidebar;
  CMenu WinMenu;
  CSplash *Splash;

  HICON m_hIconSm;

  LPCWSTR AutoOffset;
  LPCWSTR DialogCaption;
  char tmpAutoOffset[25];

  // Misc window handling
  void Layout();

};

#endif /* #ifdef __MainWindow_h__ */
