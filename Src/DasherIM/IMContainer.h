#ifndef __IMContainer_h__
#define __IMContainer_h__

#include "Dasher.h"

#include <atlwin.h>
#include <sip.h>

class CIMContainer : public ATL::CWindowImpl<CIMContainer> {
public:
  CIMContainer(HWND hWndParent) {
    m_hWndParent = hWndParent;
    m_pDasher = NULL;
    m_pCallback = NULL;
  };

  DECLARE_WND_CLASS(_T("IMCONTAINER"));

  BEGIN_MSG_MAP( CDasherWindow )
		MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_RANGE_HANDLER(0xC000,0xFFFF,OnOther)
	END_MSG_MAP()

	LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDasherEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  HWND Create();

  void SetIMCallback(IIMCallback *pCallback) {
    m_pCallback = pCallback;
  }

private:
  void SendOutput(std::string strText);
  void SendDelete(int iCount);

  HWND m_hWndParent;
  CDasher *m_pDasher;
  IIMCallback *m_pCallback;
};

#endif