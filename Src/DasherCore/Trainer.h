#ifndef __trainer_h__
#define __trainer_h__

#include "LanguageModelling/LanguageModel.h"
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
	
  class CMandarinTrainer : public CTrainer {
  public:
    CMandarinTrainer(CLanguageModel *pLanguageModel, const CAlphabetMap *pAlphabet, const CAlphabetMap *pCHAlphabet);

    //override...
    virtual void LoadFile(const std::string &strPath);
    
  private:
    const CAlphabetMap *m_pCHAlphabet;
  };

}

#endif
