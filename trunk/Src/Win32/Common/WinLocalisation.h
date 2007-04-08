// WinLocalisation.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WinLocalisation_h__
#define __WinLocalisation_h__

namespace WinLocalisation {
  UINT GetUserCodePage();
  HFONT GetCodePageFont(UINT CodePage, LONG Height = 0);
  void GetResourceString(UINT ResourceID, Tstring * ResourceString);
}
#endif  /* #ifndef __WinLocalisation_h__ */
