// SocketPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SocketPage_h__
#define __SocketPage_h__

#include "PrefsPageBase.h"

#include "../resource.h"
#include "../AppSettings.h"

#include "../../DasherCore/DasherInterfaceBase.h"

class CSocketPage:public CPrefsPageBase {
public:
  CSocketPage(HWND Parent, CDasherInterfaceBase * DI, CAppSettings *pAppSettings);
private:

  void PopulateList();
  void InitCustomBox();
  bool UpdateInfo();
  bool Apply();
  bool Validate();

  // validation subroutines:
  bool validateTextBoxes(bool apply, bool noerror);
  bool checkMinOrMax(bool apply, bool noerror, int paramID, int idc);
  bool checkLabel(bool apply, bool noerror, int paramID, int idc);
  bool checkPort(bool apply, bool noerror);
};

#endif  /* #ifndef __SocketPage_h__ */
