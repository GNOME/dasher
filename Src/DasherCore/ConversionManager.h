#ifndef __conversion_manager_h__
#define __conversion_manager_h__

#include "ConversionHelper.h"
#include "AlphabetManager.h"
#include "DasherModel.h"
#include "DasherTypes.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "NodeManager.h"

#include <vector>

namespace Dasher {
  class CDasherNode; // Forward declaration

  ///
  /// Conversion manager class - handles nodes representing the
  /// conversion of strings in languages where symbols are converted
  ///
  
  class CConversionManager : public CNodeManager {
  public:
    // TODO: We shouldn't need to know about this stuff, but the code is somewhat in knots at the moment
    CConversionManager(CNodeCreationManager *pNCManager, CConversionHelper *pHelper, int CMid);
    ~CConversionManager();

    ///
    /// Increment reference count
    ///

    virtual void Ref() {
      ++m_iRefCount;
    };
    
    ///
    /// Decrement reference count
    ///
    
    virtual void Unref() {
      --m_iRefCount;

      if(m_iRefCount == 0)
	delete this;
    };

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

    ///
    /// Calculate sizes for each of the children - default
    /// implementation assigns decending probabilities in a power law
    /// fashion (so assumes ordering), but specific subclasses are
    /// free to implement their own behaviour. The only restriction is
    /// that sizes should be posivive and sum to the appropriate
    /// normalisation constant
    ///
    
    virtual void AssignChildSizes(SCENode *pNode, CLanguageModel::Context context, int iNChildren);

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

    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);

    ///
    /// Called when a node is left backwards
    ///

    virtual void Undo( CDasherNode *pNode );

    ///
    /// Entered backwards
    ///

    virtual void Enter(CDasherNode *pNode) {};

    /// 
    /// Left forwards
    ///

    virtual void Leave(CDasherNode *pNode) {};

    ///
    /// Rebuild the parent of a given node - used for when backoff occurs beyond the start of the tree
    ///

    virtual CDasherNode *RebuildParent(CDasherNode *pNode, int iGeneration) {
      return 0;
    }

    virtual void SetFlag(CDasherNode *pNode, int iFlag, bool bValue);

  private:

    /// 
    /// Build the conversion tree (lattice) for the given string -
    /// evaluated late to prevent unnecessary conversions when the
    /// children of the root node are never instantiated
    ///
    
    void BuildTree(CDasherNode *pRoot);

    /// 
    /// Recursively delete the conversion tree when we're done
    ///

    bool RecursiveDelTree(SCENode* pNode);

    /// 
    /// Dump tree to stdout (debug)
    ///
    
    void RecursiveDumpTree(SCENode *pCurrent, unsigned int iDepth);

    ///
    /// Flag whether the tree has already been built
    ///

    bool m_bTreeBuilt;

    ///
    /// Root of the tree (TODO: Why is this a double pointer?)
    ///

    SCENode **m_pRoot;

    ///
    /// Dasher model (TODO: We ideally shouldn't need to know about this)
    ///
    
    CNodeCreationManager *m_pNCManager;

    ///
    /// Language model (TODO: We don't need to know about this, surely)
    ///

    CLanguageModel *m_pLanguageModel;

    ///
    /// Conversion helper
    ///
    
    CConversionHelper *m_pHelper;

    ///
    /// Reference count 
    ///


    CLanguageModel::Context m_iLearnContext;

    
    int m_iRefCount;

    ///
    /// Unique identifier, used to talk to the conversion helper so
    /// that it can be shared between multiple conversion nodes
    /// without state collisions
    ///

    int m_iCMID;


    //TODO: REVISE
    struct SConversionData {
      //     symbol iSymbol;
      //     int iPhase;
      CLanguageModel *pLanguageModel;
      CLanguageModel::Context iContext;
      SCENode * pSCENode;
      //int iGameOffset;
    };
  };
}

#endif
