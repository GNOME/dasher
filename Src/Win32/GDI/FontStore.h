// FontStore.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003 David Ward
//
/////////////////////////////////////////////////////////////////////////////

// A container for fonts

// You specifiy the font name, and the container is built with a load of fonts of increasing size
// The font can be retrieved with GetFont(iIndex)

#ifndef __GDI_FontStore_h__
#define __GDI_FontStore_h__

#include "../../Common/NoClones.h"
#include "../../Common/myassert.h"
#include <vector>

class CFontStore:public NoClones {
public:
  CFontStore(Tstring FontName);
  ~CFontStore();

  HFONT GetFont(int iSize) const;

private:

    std::vector < HFONT > m_vhfFonts;

};

/////////////////////////////////////////////////////////////////////////////

inline HFONT CFontStore::GetFont(int iSize) const {
  DASHER_ASSERT(iSize >= 0);
  int iMax = m_vhfFonts.size() - 1;
  if(iSize >= iMax)
    iSize = iMax;
  return m_vhfFonts[iSize];
}

#endif
