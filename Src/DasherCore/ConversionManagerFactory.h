#ifndef __conversion_manager_factory_h__
#define __conversion_manager_factory_h__

#include "ConversionHelper.h"
#include "DasherModel.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "NodeManagerFactory.h"

namespace Dasher {
  
  class CConversionManagerFactory : public CNodeManagerFactory {
  public:
    CConversionManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel);
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

  private:
    CDasherModel *m_pModel;
    CLanguageModel *m_pLanguageModel;
    CConversionHelper *m_pHelper;
    
    int m_iCMCount;
    int pagecount;//test
  };
  
}

#endif
