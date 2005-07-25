#ifndef __controlmanager_h__
#define __controlmanager_h__

#include "NodeManager.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here

#include <vector>

namespace Dasher {

  class CDasherModel;

  ///
  /// A node manager which deals with alphabets and language models.
  ///
  
  class CControlManager : public CNodeManager {
  public:

    class CControlNode {
    public: // Worry about encapsulation later
      std::vector<CControlNode *> vChildren;
      std::string strLabel;
      int iID;
    };

    CControlManager(CDasherModel *pModel, CLanguageModel *pLanguageModel );

    ///
    /// Does nothing - control manager isn't reference counted.
    ///

    virtual void Ref() {};
    
    ///
    /// Does nothing - control manager isn't reference counted.
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
    
    virtual void Output( CDasherNode *pNode );
    virtual void Undo( CDasherNode *pNode );

  private:
    CDasherModel *m_pModel;
    CLanguageModel *m_pLanguageModel;

    CControlNode *m_pControlRoot;
  };

}


#endif
