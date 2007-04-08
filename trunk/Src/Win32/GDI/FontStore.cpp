// FontStore.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "FontStore.h"

using namespace WinLocalisation;

/////////////////////////////////////////////////////////////////////////////

CFontStore::CFontStore(Tstring FontName) {

  for(int i = 0; i < 50; i++) {
    if(FontName == TEXT(""))
      m_vhfFonts.push_back(GetCodePageFont(GetUserCodePage(), LONG(-(i + 1))));
    else {
      HFONT Font;
      Font = CreateFont(int (-i), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName

      DASHER_ASSERT(Font != 0);
      m_vhfFonts.push_back(Font);
    }
  }

}

/////////////////////////////////////////////////////////////////////////////

CFontStore::~CFontStore() {
  while(m_vhfFonts.size()) {
    DeleteObject(m_vhfFonts.back());
    m_vhfFonts.pop_back();
  }
}

/////////////////////////////////////////////////////////////////////////////
