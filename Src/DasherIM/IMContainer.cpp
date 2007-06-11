#include "stdafx.h"
#include "IMContainer.h"

HWND CIMContainer::Create() {
  HWND hWnd = ATL::CWindowImpl<CIMContainer>::Create(m_hWndParent);

  // TODO: Takedown in destructor
  m_pDasher = new CDasher(hWnd);

  return hWnd;
}

LRESULT CIMContainer::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if(m_pDasher)
    m_pDasher->Move(0, 0, LOWORD(lParam), HIWORD(lParam));

  return 0;
}

LRESULT CIMContainer::OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if (message == WM_DASHER_EVENT)
    return OnDasherEvent( message, wParam, lParam, bHandled);

  return 0;
}

LRESULT CIMContainer::OnDasherEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // Apparently putting the typecast directly in the switch doesn't work
  CEvent *pEvent( (CEvent *)lParam );
  
  // TODO: return if handled?
  switch(pEvent->m_iEventType) {
   case EV_EDIT: {
    Dasher::CEditEvent * pEvt(static_cast< Dasher::CEditEvent * >(pEvent));
     
    switch (pEvt->m_iEditType) {
      case 1:
       SendOutput(pEvt->m_sText);
       break;
      case 2:
	     SendDelete(pEvt->m_sText.size());
	     break;
      default:
        break;
    }
    break;
                 }
  default:
    break;
  }
  
  return 0;
}

void CIMContainer::SendOutput(std::string strText) {
  if(m_pCallback) {
    BSTR bstrText = SysAllocString((const OLECHAR *)strText.c_str());

    m_pCallback->SendString(bstrText, strText.size());
  }
}

void CIMContainer::SendDelete(int iCount) {
   if(m_pCallback) {
     for(int i(0); i < iCount; ++i) {
       m_pCallback->SendVirtualKey(VK_BACK, KEYEVENTF_SILENT);
       m_pCallback->SendVirtualKey(VK_BACK, KEYEVENTF_KEYUP | KEYEVENTF_SILENT);
     }
   }
}