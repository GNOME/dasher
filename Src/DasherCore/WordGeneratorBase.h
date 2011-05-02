#ifndef __WordGeneratorBase_h__
#define __WordGeneratorBase_h__

#include <string>
#include <sstream>
#include "Alphabet/AlphabetMap.h"
#include "Alphabet/AlphIO.h"

using namespace std;

namespace Dasher {
/**
 * The base class for all word generators. Word generators encapsulate
 * logic for simply generating words based on implementation-specific
 * conditions. The benefit of this is that a generator can
 * easily be written that selects words based on a function of the current
 * value of the Sri Lankan rupee and the amount of twitter feeds regarding
 * the winter olympics, for example.
 * 
 * 
 * A typical use case for any class deriving from CWordGeneratorBase
 * would be the following:
 *    1) Construct the object (providing any implementation-specific params)
 *  * 
 *    2) To retrieve a word, simply call GetNextWord. This will continue
 *       until the specific implementation determines that there are no
 *       more words for any reason. When there are no more, GetNextWord
 *       returns the empty string.
 * 
 * Usage Example:
 *    CWordGeneratorBase gen = CWordGeneratorBase(params...);
 *    std::string word;
 *    while ((word = gen.GetNextWord()) != "") {
 *         Operate on the word
 *    }
 */ 
class CWordGeneratorBase {
private:
  const CAlphInfo *m_pAlph;
  const CAlphabetMap *m_pAlphMap;
public:
  CWordGeneratorBase(const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap);

  virtual ~CWordGeneratorBase() { } 
  //
  void GetSymbols(std::vector<symbol> &into);
protected:
  /// Gets the next line from this generator
  /// @return the next line, or "" if exhausted
  virtual std::string GetLine() = 0;
  
};
}
#endif
