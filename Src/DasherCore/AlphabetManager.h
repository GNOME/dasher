#ifndef __alphabetmanager_h__
#define __alphabetmanager_h__

#include "NodeManager.h"
#include "LanguageModelling/LanguageModel.h"

#include "Parameters.h"

class CNodeCreationManager;

namespace Dasher {

  ///
  /// A node manager which deals with alphabets and language models.
  ///
  
  class CAlphabetManager : public CNodeManager {
  public:

    CAlphabetManager(CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel, CLanguageModel::Context iLearnContext, bool bGameMode, const std::string &strGameModeText);

    ///
    /// Does nothing - alphabet manager isn't reference counted.
    ///

    virtual void Ref() {};
    
    ///
    /// Does nothing - alphabet manager isn't reference counted.
    ///
    
    virtual void Unref() {};

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode );
    void PopulateChildrenWithSymbol( CDasherNode *pNode, int iExistingSymbol, CDasherNode *pExistingChild );

    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode );

    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);
    virtual void Undo( CDasherNode *pNode );

    virtual CDasherNode *RebuildParent(CDasherNode *pNode, int iGeneration);

    virtual void SetFlag(CDasherNode *pNode, int iFlag, bool bValue);

  private:
    CLanguageModel *m_pLanguageModel;
    CNodeCreationManager *m_pNCManager;
    CLanguageModel::Context m_iLearnContext;
    std::string m_strGameString;
    bool m_bGameMode;

    struct SAlphabetData {
      symbol iSymbol;
      int iPhase;
      CLanguageModel *pLanguageModel;
      CLanguageModel::Context iContext;
      int iGameOffset;
    };

  };

}


#endif
