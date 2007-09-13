// Slidebar.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

// TODO: This class is fairly pointless now - it's basically a wrapper around 
// a rebar control, which is almost definitely duplicating WTL functionality

#include "../Common/WinCommon.h"
#include "../resource.h"
#include "Slidebar.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CSlidebar::CSlidebar(HWND ParentWindow, CDasherInterfaceBase *NewDasherInterface) {
  m_hRebar = CreateWindowEx(WS_EX_WINDOWEDGE,
                            REBARCLASSNAME,
                            NULL,
                            WS_CHILD  | //WS_CLIPSIBLINGS |
                             RBS_VARHEIGHT |
                            CCS_BOTTOM | CCS_NODIVIDER,
                            0,0,0,0,
                            ParentWindow, NULL, WinHelper::hInstApp,
                            NULL);

  REBARINFO rbi;
  rbi.cbSize = sizeof(REBARINFO);                                      
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;

  SendMessage(m_hRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

  REBARBANDINFO rbBand;
  rbBand.cbSize = sizeof(REBARBANDINFO); 
  rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE;
  rbBand.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;

  m_pStatusControl = new CStatusControl(NewDasherInterface);
  m_pStatusControl->Create(ParentWindow);

  RECT rc;
  m_pStatusControl->GetWindowRect(&rc);

  rbBand.hwndChild  = m_pStatusControl->m_hWnd;
  rbBand.cxMinChild = 0;
  rbBand.cyMinChild = rc.bottom - rc.top;
  rbBand.cx         = 200;

  SendMessage(m_hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
}

void CSlidebar::Resize() {
  SendMessage(m_pStatusControl->m_hWnd, WM_SIZE, 0, 0);
  int j = 0;
int i = 5/j;  
}

int CSlidebar::GetHeight() {
  RECT sRect;
  GetWindowRect(m_hRebar, &sRect);
  return sRect.bottom - sRect.top;
}