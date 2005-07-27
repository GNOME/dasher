// WinLocalisation.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "WinLocalisation.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

UINT WinLocalisation::GetUserCodePage() {
  // Pity this is for the system, not the user:
  //return GetACP();

  TCHAR CharCodePage[7];
  GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, CharCodePage, 7);

  return _ttoi(CharCodePage);
}

// // Could get name and other useful stuff about current codepage:
// CPINFOEX CPInfoEx;
// GetCPInfoEx(CurrentCodePage, 0, &CPInfoEx);
// // This Doesn't work in Win95 though...

HFONT WinLocalisation::GetCodePageFont(UINT CodePage, LONG Height) {
  CHARSETINFO CharsetInfo;
  UINT MyCharset;
  if(TranslateCharsetInfo((DWORD FAR *) CodePage, &CharsetInfo, TCI_SRCCODEPAGE)) {
    MyCharset = CharsetInfo.ciCharset;
  }
  else
    MyCharset = DEFAULT_CHARSET;

  return CreateFont(Height, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, MyCharset,       // Apart from the character set and size I don't care about the font.
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, NULL);
}

void WinLocalisation::GetResourceString(UINT ResourceID, Tstring *ResourceString) {
  const unsigned int MAX_LOADSTRING = 100;
  TCHAR szTitle[MAX_LOADSTRING];
  LoadString(WinHelper::hInstApp, ResourceID, szTitle, MAX_LOADSTRING);
  *ResourceString = szTitle;
}
