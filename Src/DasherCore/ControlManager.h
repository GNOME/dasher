#ifndef __controlmanager_h__
#define __controlmanager_h__

#include "NodeManager.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here

#include <vector>
#include <map>

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
      int iColour;
    };

    enum { CTL_ROOT, CTL_STOP, CTL_PAUSE, CTL_MOVE, CTL_MOVE_FORWARD, 
	   CTL_MOVE_FORWARD_CHAR, CTL_MOVE_FORWARD_WORD, CTL_MOVE_FORWARD_LINE,
	   CTL_MOVE_FORWARD_FILE, CTL_MOVE_BACKWARD, CTL_MOVE_BACKWARD_CHAR, 
	   CTL_MOVE_BACKWARD_WORD, CTL_MOVE_BACKWARD_LINE, CTL_MOVE_BACKWARD_FILE,
	   CTL_DELETE, CTL_DELETE_FORWARD, 
	   CTL_DELETE_FORWARD_CHAR, CTL_DELETE_FORWARD_WORD, CTL_DELETE_FORWARD_LINE,
	   CTL_DELETE_FORWARD_FILE, CTL_DELETE_BACKWARD, CTL_DELETE_BACKWARD_CHAR,
	   CTL_DELETE_BACKWARD_WORD, CTL_DELETE_BACKWARD_LINE, CTL_DELETE_BACKWARD_FILE,
	   CTL_USER
    };

    CControlManager(CDasherModel *pModel, CLanguageModel *pLanguageModel );
    ~CControlManager();

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
    
    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization );
    virtual void Undo( CDasherNode *pNode );

    void RegisterNode( int iID, std::string strLabel, int iColour );
    void ConnectNode(int iChild, int iParent, int iAfter);


  private:
    CDasherModel *m_pModel;
    CLanguageModel *m_pLanguageModel;

    std::map<int,CControlNode*> m_mapControlMap;

  };

}


#endif
