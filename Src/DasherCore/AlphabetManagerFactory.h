#ifndef __alphabetmanagerfactory_h__
#define __alphabetmanagerfactory_h__

#include "AlphabetManager.h"
#include "NodeManagerFactory.h"
#include "LanguageModelling/LanguageModel.h"

class CNodeCreationManager;

namespace Dasher {
  
  class CAlphabetManagerFactory : public CNodeManagerFactory {
  public:
    CAlphabetManagerFactory(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, Dasher::CAlphIO *pAlphIO, CNodeCreationManager *pNCManager,  bool bGameMode, const std::string &strGameModeText);

    //    CAlphabetManagerFactory( CDasherModel *pModel, CLanguageModel *pLanguageModel, bool bGameMode, const std::string &strGameModeText );
    ~CAlphabetManagerFactory();
    
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);
    
    ///
    /// Functions for backwards compatibility - hopefully these will
    /// be eliminated to a great extent sometime in the future
    ///

    CLanguageModel *GetLanguageModel() {
      return m_pLanguageModel;
    };

    CAlphabet *GetAlphabet() {
      return m_pAlphabet;
    };

    int GetConversionID() {
      return m_iConversionID;
    };

  private:
    CAlphabetManager *m_pAlphabetManager;

    CLanguageModel *m_pLanguageModel;     // pointer to the language model
    CAlphabet *m_pAlphabet;        // pointer to the alphabet

    int m_iConversionID;
  };
  
}

#endif
