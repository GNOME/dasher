// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LMPage_h__
#define __LMPage_h__

#include "../resource.h"
#include "../AppSettings.h"

#include "../../DasherCore/Win32/DasherInterface.h"
#include "../../DasherCore/ColourIO.h"

class CLMPage:public CWinWrap {
public:
  CLMPage(HWND Parent, CDasherInterface * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  CDasherInterface * m_pDasherInterface;
  CAppSettings *m_pAppSettings;
  
  HWND slider;
  HWND uniformbox;

  TCHAR m_tcBuffer[1000];


  // Some status flags:
  void PopulateList();
  void InitCustomBox();
  bool UpdateInfo();
   bool Apply();
  bool Validate();
};

#endif  /* #ifndef __AlphabetBox_h__ */
