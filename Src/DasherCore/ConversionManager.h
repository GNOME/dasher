#ifndef __conversion_manager_h__
#define __conversion_manager_h__

#include "NodeManager.h"
#include "DasherTypes.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "DasherModel.h"

namespace Dasher {
  class CDasherNode; // Forward declaration

  ///
  /// Virtual base class for node managers
  ///
  
  class CConversionManager : public CNodeManager {
  public:
    // TODO: We shouldn't need to know about this stuff, but the code is somewhat in knots at the moment
    CConversionManager(CDasherModel *pModel, CLanguageModel *pLanguageModel );

    ///
    /// Increment reference count
    ///

    virtual void Ref();
    
    ///
    /// Decrement reference count
    ///
    
    virtual void Unref();

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode );
    
    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode );

    ///
    /// Called whenever a node belonging to this manager first 
    /// moves under the crosshair
    ///

    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {};
    virtual void Undo( CDasherNode *pNode ) {};

    virtual void Enter(CDasherNode *pNode) {};
    virtual void Leave(CDasherNode *pNode) {};

    virtual CDasherNode *RebuildParent(CDasherNode *pNode, int iGeneration) {
      return 0;
    }
  private:
    class CConversionManagerNode {
    public:
      std::string m_strSymbol;
      CConversionManagerNode *m_pChild;
      CConversionManagerNode *m_pNext;
      int m_iNumChildren;

      CConversionManagerNode *FindChild(const std::string &strSymbol) {
	CConversionManagerNode *pCurrentChild(m_pChild);

	while(pCurrentChild) {

	  if(pCurrentChild->m_strSymbol == strSymbol)
	    return pCurrentChild;

	  pCurrentChild = pCurrentChild->m_pNext;
	}

	return 0;
      }
    };


    CConversionManagerNode *m_pRoot;

    CDasherModel *m_pModel;
    CLanguageModel *m_pLanguageModel;
  };

}

#endif
