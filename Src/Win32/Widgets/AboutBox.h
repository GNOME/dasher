// AboutBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __AboutBox_h__
#define __AboutBox_h__





class CAboutbox
{
public:
	CAboutbox(HWND Parent);
private:
	static LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};


#endif /* #ifndef __AboutBox_h__ */