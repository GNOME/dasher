#ifndef __alphabetmanager_h__
#define __alphabetmanager_h__

#include "NodeManager.h"
#include "LanguageModelling/LanguageModel.h"

#include "Parameters.h"

namespace Dasher {

  class CDasherModel;

  ///
  /// A node manager which deals with alphabets and language models.
  ///
  
  class CAlphabetManager : public CNodeManager {
  public:

    CAlphabetManager( CDasherModel *pModel, CLanguageModel *pLanguageModel );

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

    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper);

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode );
    
    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode );

    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);
    virtual void Undo( CDasherNode *pNode );

  private:
    CLanguageModel *m_pLanguageModel;
    CDasherModel *m_pModel;

  };

}


#endif
