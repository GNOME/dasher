#ifndef __trainer_h__
#define __trainer_h__

#include "LanguageModelling/PPMPYLanguageModel.h"
#include "TrainingHelper.h"
#include "Alphabet/AlphInfo.h"

namespace Dasher {
  class CDasherInterfaceBase;
	
  class CTrainer : public CTrainingHelper {
  public:
    CTrainer(CLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pAlphabet);

  protected:
    virtual void Train(CAlphabetMap::SymbolStream &syms);
    CLanguageModel *m_pLanguageModel;
    
    ///Try to read a context-switch escape sequence from the symbolstream.
    /// \param sContext context to be reinitialized if a context-switch command is found
    /// \syms symbolstream to read, should be positioned just after the first occurrence of the escape character.
    /// \return true if a context-switch command was found (=> sContext reinitialized);
    ///  false, if instead a double-escape-character (=encoding of that actual symbol) was read
    bool readEscape(CLanguageModel::Context &sContext, CAlphabetMap::SymbolStream &syms);
    
    const CAlphInfo *m_pInfo;
    // symbol number in alphabet of the context-switch character (maybe 0 if not in alphabet!)
    int m_iCtxEsc;
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
    /// \param pInfo used for GetContextEscapeChar and GetDefaultContext (only), both as strings
    /// \param pPYAlphabet mapping from text to symbol# in PY alphabet
    /// \param pCHAlphabet mapping from text to symbol# (rehashed by MandarinAlphMgr) in CHAlphabet
    /// \param strDelim delimiter character (1 unicode, maybe >1 octet; if not, will never be matched)
    CMandarinTrainer(CPPMPYLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pPYAlphabet, const CAlphabetMap *pCHAlphabet, const std::string &strDelim);

  protected:
    //override...
    virtual void Train(CAlphabetMap::SymbolStream &syms);
    
  private:
    ///The pinyin alphabet (the chinese alphabet is passed into the superclass)
    const CAlphabetMap *m_pPYAlphabet;
    ///Delimiter, as above. 
    const std::string m_strDelim;
  };

}

#endif
