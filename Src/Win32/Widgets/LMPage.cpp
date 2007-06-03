// LMPage.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "LMPage.h"
#include "../resource.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CLMPage::CLMPage(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
:CPrefsPageBase(Parent, DI, pAppSettings) {
 
}

void CLMPage::PopulateList() {

}


bool CLMPage::Apply() {


  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

LRESULT CLMPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  
    return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
  //return FALSE;
}
