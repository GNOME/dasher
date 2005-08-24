// StringUtils.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __StringUtils__
#define __StringUtils__

#include <sstream>

namespace wincommon
{
	
	inline void GetWindowText(HWND hWnd, std::wstring& str)
	{
		int nLength = GetWindowTextLength( hWnd );
		wchar_t* pszBuffer = new wchar_t [nLength+1];
		::GetWindowText(hWnd, pszBuffer , nLength+1);
		str = pszBuffer;
		delete [] pszBuffer;
	}

	inline bool GetWindowInt( HWND hWnd, int& i)
	{
		std::wstring str;
		wincommon::GetWindowText( hWnd, str);
		std::wistringstream iss(str);
		iss >> i;
		if (iss)
			return true;
		else
			return false;
	}


};

#endif 
