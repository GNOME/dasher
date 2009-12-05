// LMPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LMPage_h__
#define __LMPage_h__

#include "PrefsPageBase.h"

#include "../resource.h"
#include "../AppSettings.h"

#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../DasherCore/ColourIO.h"

class CLMPage:public CPrefsPageBase {
public:
  CLMPage(HWND Parent, Dasher::CDasherInterfaceBase * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  
 

  // Some status flags:
  void PopulateList();
  void InitCustomBox();
  bool UpdateInfo();
   bool Apply();
 
};

#endif  /* #ifndef __AlphabetBox_h__ */
