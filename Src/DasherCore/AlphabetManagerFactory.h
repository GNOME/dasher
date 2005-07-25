#ifndef __alphabetmanagerfactory_h__
#define __alphabetmanagerfactory_h__

#include "AlphabetManager.h"
#include "NodeManagerFactory.h"
#include "LanguageModelling/LanguageModel.h"

namespace Dasher {

  class CDasherModel;
  
  class CAlphabetManagerFactory : public CNodeManagerFactory {
  public:
    CAlphabetManagerFactory( CDasherModel *pModel, CLanguageModel *pLanguageModel );
    ~CAlphabetManagerFactory();
    
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper);
    
  private:
    CAlphabetManager *m_pAlphabetManager;
  };
  
}

#endif
