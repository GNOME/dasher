#ifndef __conversion_manager_factory_h__
#define __conversion_manager_factory_h__

#include "ConversionHelper.h"
//#include "DasherModel.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "NodeManagerFactory.h"

#include <map>

namespace Dasher {
  
  class CDasherModel; // Forward declaraion

  class CConversionManagerFactory : public CNodeManagerFactory {
  public:
    CConversionManagerFactory(Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, int iID, Dasher::CAlphIO *pCAlphIO);
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

  private:
    CConversionHelper *GetHelper(Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, int iID, Dasher::CAlphIO *pCAlphIO);

    CNodeCreationManager *m_pNCManager;
    CConversionHelper *m_pHelper;
    
    int m_iCMCount;
    int pagecount;//test
  };
  
}

#endif
