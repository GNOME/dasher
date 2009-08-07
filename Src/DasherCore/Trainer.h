#ifndef __trainer_h__
#define __trainer_h__

#include "LanguageModelling/LanguageModel.h"
#include "TrainingHelper.h"

namespace Dasher {
  class CAlphabet;
  class CAlphIO;
  class CDasherInterfaceBase;
	
  class CTrainer : public CTrainingHelper {
  public:
    CTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet);
    void Train(const std::string &strUserLoc, const std::string &strSystemLoc);
	void Train(const std::string &strPath);

  protected:
	virtual void Train(const std::vector<symbol> &vSymbols);
	CLanguageModel *m_pLanguageModel;
  };
	
  class CMandarinTrainer : public CTrainer {
  public:
    CMandarinTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet, CAlphabet *pCHAlphabet);
    
    virtual void Train(const std::string &strUserLoc, const std::string &strSystemLoc);
    
  private:
    CAlphabet * m_pCHAlphabet;
  };

}

#endif
