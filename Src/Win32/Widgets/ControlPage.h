// ControlPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ControlPage_h__
#define __ControlPage_h__

#include "PrefsPageBase.h"


#include "../../DasherCore/DasherInterfaceBase.h"

class CAppSettings;

class CControlPage:public CPrefsPageBase {
public:
  CControlPage(HWND Parent, Dasher::CDasherInterfaceBase * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  Dasher::CDasherInterfaceBase *m_pDasherInterface;

  TCHAR m_tcBuffer[1000];
  
  // Handles to important widgets
  HWND SB_slider;
  HWND speedbox;
  HWND m_hMousePosStyle;
  
  void PopulateList();
  bool Apply();
  bool GetSelectedListboxText(int lbxId, std::string & selectedText);
  void EnablePrefferencesForSelectedModule(int lboxId, int btnId);
};

#endif  // ndef
