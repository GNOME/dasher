// SplashScreen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SplashScreen_h__
#define __SplashScreen_h__




class CSplash : public CWinWrap
{
public:
	CSplash(HWND Parent);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	static LRESULT CALLBACK TmpWndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
};


#endif /* #ifndef __SplashScreen_h__ */