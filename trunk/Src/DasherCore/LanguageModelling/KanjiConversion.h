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
  /// \ingroup LM
  /// \{
  class CKanjiConversion {
  public:
    CKanjiConversion();
    virtual ~ CKanjiConversion();

    virtual int ConvertKanji(std::string);

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
  /// \}
  
  inline CKanjiConversion::CKanjiConversion(){
    IsInit = 0;
  }
  
  inline CKanjiConversion::~CKanjiConversion(){
    IsInit = 0;
  }
  
  inline int CKanjiConversion::ConvertKanji(std::string s){
    return 0;
  }
}

#endif /* #ifndef __KanjiConversion_H__ */
