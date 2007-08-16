#ifndef __alphabetmanagerfactory_h__
#define __alphabetmanagerfactory_h__

#include "AlphabetManager.h"
#include "NodeManagerFactory.h"
#include "LanguageModelling/LanguageModel.h"

class CNodeCreationManager;

namespace Dasher {
  class CAlphabet;
  class CAlphIO;
  class CDasherInterfaceBase;

  /// \ingroup Model
  /// @{
  class CAlphabetManagerFactory : public CNodeManagerFactory {
  public:
    class CTrainer {
    public:
      CTrainer(CLanguageModel *pLanguageModel);
      
      void Train(const std::vector < symbol > &vSymbols);
      
      ~CTrainer();
      
    private:
      CLanguageModel *m_pLanguageModel;
      CLanguageModel::Context m_Context;
    };

    CAlphabetManagerFactory(CDasherInterfaceBase *pInterface,
			    CEventHandler *pEventHandler,
			    CSettingsStore *pSettingsStore,
			    Dasher::CAlphIO *pAlphIO,
			    CNodeCreationManager *pNCManager);

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

    CLanguageModel::Context GetLearnContext() {
      return m_iLearnContext;
    };

    CAlphabet *GetAlphabet() {
      return m_pAlphabet;
    };

    int GetConversionID() {
      return m_iConversionID;
    };

    CTrainer *GetTrainer();

  private:
    CAlphabetManager *m_pAlphabetManager;

    CLanguageModel *m_pLanguageModel;     // pointer to the language model
    CLanguageModel::Context m_iLearnContext; // Used to add data to model as it is entered
    CAlphabet *m_pAlphabet;        // pointer to the alphabet

    int m_iConversionID;
  };
  /// @}  
}

#endif
