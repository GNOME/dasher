#ifndef __trainer_h__
#define __trainer_h__

#include "LanguageModelling/PPMPYLanguageModel.h"
#include "TrainingHelper.h"

namespace Dasher {
  class CDasherInterfaceBase;
	
  class CTrainer : public CTrainingHelper {
  public:
    CTrainer(CLanguageModel *pLanguageModel, const CAlphabetMap *pAlphabet);

  protected:
    virtual void Train(CAlphabetMap::SymbolStream &syms);
    CLanguageModel *m_pLanguageModel;
  };
	
  /// Trains a PPMPYLanguageModel (dual alphabet), as for e.g. MandarinDasher.
  /// The training file is broken down into (delimiter, PY, CH) triples, each
  /// one unicode character. Every time a delimiter is seen,  we take the next
  /// unicode character as a symbol (syllable+tone) in the PinYin alphabet
  /// (identified by symbol _text_), and the character after that, as a symbol
  /// in the final=converted=Chinese alphabet. We then skip until the next delimiter.
  class CMandarinTrainer : public CTrainer {
  public:
    /// Construct a new MandarinTrainer
    /// \param pAlphabet mapping from text to symbol# in PY alphabet
    /// \param pCHAlphabet mapping from text to symbol# (rehashed by MandarinAlphMgr) in CHAlphabet
    /// \param strDelim delimiter character (1 unicode, maybe >1 octet; if not, will never be matched)
    CMandarinTrainer(CPPMPYLanguageModel *pLanguageModel, const CAlphabetMap *pAlphabet, const CAlphabetMap *pCHAlphabet, const std::string &strDelim);

  protected:
    //override...
    virtual void Train(CAlphabetMap::SymbolStream &syms);
    
  private:
    const CAlphabetMap *m_pCHAlphabet;
    ///Delimiter, as above. 
    const std::string m_strDelim;
  };

}

#endif
