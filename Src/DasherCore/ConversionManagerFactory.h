#ifndef __conversion_manager_factory_h__
#define __conversion_manager_factory_h__

#include "NodeManagerFactory.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "DasherModel.h"

namespace Dasher {
  
  class CConversionManagerFactory : public CNodeManagerFactory {
  public:
    CConversionManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel);
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

  private:
    CDasherModel *m_pModel;
    CLanguageModel *m_pLanguageModel;
  };
  
}

#endif
