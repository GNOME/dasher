// KanjiConversionIME.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __KanjiConversionIME_h__
#define __KanjiConversionIME_h__

// For Kanji support
#include <windows.h>
#include <imm.h>

#include "KanjiConversion.h"

namespace Dasher {
  /// \ingroup LM
  /// \{
  class CKanjiConversionIME:public CKanjiConversion {
  public:
    CKanjiConversionIME();
    ~CKanjiConversionIME();

    int ConvertKanji(std::string);
    HIMC hIMC;
  };
  /// \}
}
#endif /* #ifndef __KanjiConversionCanna_H__ */
