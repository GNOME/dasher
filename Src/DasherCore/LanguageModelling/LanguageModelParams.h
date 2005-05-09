#ifndef __LanguageModelling_LanguageModelParams_h__
#define __LanguageModelling_LanguageModelParams_h__

#include <map>
#include <string>

// Persistant storage so that parameters survive the language model being regenerated.

namespace Dasher {
  class CLanguageModelParams{
  public:

    int GetValue( std::string pname ) {
      return pmap[ pname ];
    };

    void SetValue( std::string pname, int value ) {
      pmap[ pname ] = value;
    };

    std::map< std::string, int > pmap;
  };
}

#endif
