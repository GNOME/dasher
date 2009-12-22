#ifndef __trainer_h__
#define __trainer_h__

#include "LanguageModelling/LanguageModel.h"
#include "TrainingHelper.h"

namespace Dasher {
  class CAlphIO;
  class CDasherInterfaceBase;
	
  class CTrainer : public CTrainingHelper {
  public:
    CTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet);

  protected:
    virtual void Train(CAlphabet::SymbolStream &syms);
    CLanguageModel *m_pLanguageModel;
  };
	
  class CMandarinTrainer : public CTrainer {
  public:
    CMandarinTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet, CAlphabet *pCHAlphabet);

    //override...
    virtual void LoadFile(const std::string &strPath);
    
  private:
    CAlphabet * m_pCHAlphabet;
  };

}

#endif
