// ConversionManager.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __conversion_manager_h__
#define __conversion_manager_h__

#include "ConversionHelper.h"
#include "AlphabetManager.h"
#include "DasherModel.h"
#include "DasherTypes.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "NodeManager.h"
#include "SCENode.h"

#include <vector>

// TODO: Conversion manager needs to deal with offsets and contexts - Will: See Phil for an explanation.

namespace Dasher {
  class CDasherNode; // Forward declaration

  /// \ingroup Model
  /// @{

  /// This class manages nodes in conversion subtrees, typically used
  /// for languages where complex characters are entered through a
  /// composition process, for example Japanese and Chinese. 
  ///
  /// A new CConversionManager is created for each subtree, and
  /// therefore represents the conversion of a single phrase. The
  /// phrase to be converted is read by recursing through the parent
  /// tree. An instance of CConversionHelper is shared by several
  /// CConversionManagers, and performs the language dependent aspects
  /// of conversion. Specifically construction of the candidate
  /// lattice and assignment of weights.
  ///
  /// The general policy is to delay computation as far as possible,
  /// to avoid unnecessary computational load. The candidate lattice
  /// is therefore built at the first call to PopulateChildren, and
  /// weights are only assigned when the appropriate node has its
  /// child list populated.
  ///
  /// See CConversionHelper for details of the language specific
  /// aspects of conversion, and CNodeManager for details of the node
  /// management process.
  ///
  class CConversionManager : public CNodeManager {
  public:
    // TODO: We shouldn't need to know about this stuff, but the code is somewhat in knots at the moment
    CConversionManager(CNodeCreationManager *pNCManager, CConversionHelper *pHelper, CAlphabet *pAlphabet, int CMid);
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


      //      std::cout << "Unref, new count = " << m_iRefCount << std::endl;

      if(m_iRefCount == 0) {
	//	std::cout << "Deleting " << this << std::endl;
	delete this;
      }
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
    
    virtual void AssignChildSizes(SCENode **pNode, CLanguageModel::Context context, int iNChildren);

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

    CAlphabet *m_pAlphabet;

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
      bool bType; // True for termial nodes (bit of a hack)
      int iOffset;
      //int iGameOffset;
    };
  };
  /// @}
}

#endif
