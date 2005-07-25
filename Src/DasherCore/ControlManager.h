#ifndef __controlmanager_h__
#define __controlmanager_h__

#include "NodeManager.h"

namespace Dasher {

  class CDasherModel;

  ///
  /// A node manager which deals with alphabets and language models.
  ///
  
  class CControlManager : public CNodeManager {
  public:

    CControlManager(CDasherModel *pModel);

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

    virtual CDasherNode *GetRoot();

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode );
    
    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode );

  private:
    CDasherModel *m_pModel;

  };

}


#endif
