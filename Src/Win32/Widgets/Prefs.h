// Prefs.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PrefsBox_h__
#define __PrefsBox_h__

#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"

#include "../Dasher.h"

class CPrefs : public CWinWrap
{
public:
	CPrefs(HWND hParent, CDasher *pDasher );
	void PopulateWidgets();
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hCustomBox;
  CDasher *m_pDasher;
};


#endif /* #ifndef __PrefsBox_h__ */