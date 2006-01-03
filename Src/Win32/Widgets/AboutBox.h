// AboutBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AboutBox_h__
#define __AboutBox_h__
#include "../resource.h"

/////////////////////////////////////////////////////////////////////////////

class CAboutBox : public CDialogImpl<CAboutBox>
{
public:

	CAboutBox() {} 

	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		COMMAND_ID_HANDLER(IDOK, OnClose)
	END_MSG_MAP()


private:
	LRESULT OnClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

};

#endif  /* #ifndef __AboutBox_h__ */
