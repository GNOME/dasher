// KanjiConversion.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __KanjiConversion_h__
#define __KanjiConversion_h__

#include <string>
#include <vector>

namespace Dasher {
  class CKanjiConversion;
} class CKanjiConversion {
public:
  CKanjiConversion() {
    IsInit = 0;
  };
  ~CKanjiConversion() {
    IsInit = 0;
  };

  virtual int ConvertKanji(std::string) = 0;
  //int context_id;

  class CPhrase {
  public:
    CPhrase() {
    };
    CPhrase(CPhrase * p) {
      candidate_list = p->candidate_list;
    };
    ~CPhrase() {
      candidate_list.clear();
    };
    std::vector < std::string > candidate_list;
  };

  std::vector < CPhrase > phrase;       //      a list of phrases

  bool IsInit;                  // true if dictionary is loaded
};

#endif /* #ifndef __KanjiConversion_H__ */
