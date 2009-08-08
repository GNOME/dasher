#ifndef __conversion_manager_factory_h__
#define __conversion_manager_factory_h__

#include "ConversionHelper.h"
#include "ConversionManager.h"
//#include "DasherModel.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here

#include <map>

namespace Dasher {
  class CDasherModel; // Forward declaraion
  class CConversionManager;

  /// \ingroup Model
  /// @{
  class CConversionManagerFactory {
  public:
    CConversionManagerFactory(Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, int iID, Dasher::CAlphIO *pCAlphIO, CAlphabet *pAlphabet);
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);
    ~CConversionManagerFactory();
	  
  private:
    CConversionHelper *GetHelper(Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, int iID, Dasher::CAlphIO *pCAlphIO);

    CConversionHelper *GetHelperChinese(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, Dasher::CAlphIO *pCAlphIO);

    CNodeCreationManager *m_pNCManager;
    CConversionManager *m_pMgr;
    CAlphabet *m_pAlphabet;
    
    int m_iCMCount;
    int pagecount;//test
  };
  /// @}
}

#endif
