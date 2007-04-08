// KanjiConversionCanna.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __KanjiConversionCanna_h__
#define __KanjiConversionCanna_h__

// For Kanji support
#include <canna/jrkanji.h>
#include <canna/RK.h>
#include <iconv.h>

#include "KanjiConversion.h"

#define BUFSIZE 10240

namespace Dasher {
  /// \ingroup LM
  /// \{
  class CKanjiConversionCanna:public CKanjiConversion {
  public:
    CKanjiConversionCanna();
    virtual ~ CKanjiConversionCanna();

    int ConvertKanji(std::string);
    int context_id;
  };
  /// \}
}
#endif /* #ifndef __KanjiConversionCanna_H__ */
