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

#include "DasherTypes.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "DasherNode.h"
#include "SCENode.h"

// TODO: Conversion manager needs to deal with offsets and contexts - Will: See Phil for an explanation.

class CNodeCreationManager;

namespace Dasher {
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
  class CConversionManager {
  public:
    // TODO: We shouldn't need to know about this stuff, but the code is somewhat in knots at the moment
    CConversionManager(CNodeCreationManager *pNCManager, CAlphabet *pAlphabet);
    
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

    protected:
    class CConvNode : public CDasherNode {
    public:
      int mgrId() {return 2;}
      CConvNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CConversionManager *pMgr);
    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren();
    virtual int ExpectedNumChildren();
    
    ~CConvNode();

    ///Attempts to fill vContextSymbols with the context that would exist _after_ this node has been entered
    void GetContext(CDasherInterfaceBase *pInterface, std::vector<symbol> &vContextSymbols, int iOffset, int iLength);

    ///
    /// Called whenever a node belonging to this manager first 
    /// moves under the crosshair
    ///

    virtual void Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);

    ///
    /// Called when a node is left backwards
    ///

    virtual void Undo();

    protected:
      CConversionManager *m_pMgr;
    public: //to ConversionManager and subclasses only, of course...
        
    //TODO: REVISE
      symbol iSymbol;
      //     int iPhase;
      CLanguageModel *pLanguageModel;
      CLanguageModel::Context iContext;
      SCENode * pSCENode;
      bool bisRoot; // True for root conversion nodes 
      //int iGameOffset;
    };

  public:
    ///
    /// Get a new root node owned by this manager
    ///
    
    virtual CConvNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, int iOffset);
  protected:    
    
  virtual CConvNode *makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo);

	  
	CNodeCreationManager *m_pNCManager;
	CAlphabet *m_pAlphabet;
	
  private:

    /// 
    /// Dump tree to stdout (debug)
    ///
    
    void RecursiveDumpTree(SCENode *pCurrent, unsigned int iDepth);
	  
	///
	/// Reference count 
	///
	  
    int m_iRefCount;
   
  };
  /// @}
}

#endif
