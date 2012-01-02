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
#include "NodeManager.h"
#include "Alphabet/AlphInfo.h"
#include "SettingsStore.h"

// TODO: Conversion manager needs to deal with offsets and contexts - Will: See Phil for an explanation.

class CNodeCreationManager;

namespace Dasher {
  class CDasherInterfaceBase;
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
  class CConversionManager : protected CSettingsUser, public CNodeManager {
    friend class CConvertingAlphMgr;
  protected:  class CConvNode; //fwd decl
  public:
    CConversionManager(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, CLanguageModel *pLanguageModel);

    ///
    /// Get a new root node owned by this manager
    ///
    
    virtual CConvNode *GetRoot(int iOffset, CLanguageModel::Context newCtx);

    ///Tells us to use the specified screen to create node labels.
    /// (note we cache the screen and create labels lazily)
    void ChangeScreen(CDasherScreen *pScreen);
    
  protected:
    
    class CConvNode : public CDasherNode {
    public:
      CConversionManager *mgr() const {return m_pMgr;}
      CConvNode(int iOffset, int iColour, CDasherScreen::Label *pLabel, CConversionManager *pMgr);
    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren();
    virtual int ExpectedNumChildren();
      virtual void SetFlag(int iFlag, bool bValue);

    ~CConvNode();

    ///Attempts to fill vContextSymbols with the context that would exist _after_ this node has been entered
    void GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, std::vector<symbol> &vContextSymbols, int iOffset, int iLength);

    ///
    /// Called whenever a node belonging to this manager first
    /// moves under the crosshair
    ///

    virtual void Output();

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
      CLanguageModel::Context iContext;
      SCENode * pSCENode;
      bool bisRoot; // True for root conversion nodes
      //int iGameOffset;
    };

    virtual CConvNode *makeNode(int iOffset, int iColour, CDasherScreen::Label *pLabel);

    CDasherInterfaceBase *m_pInterface;
	CNodeCreationManager *m_pNCManager;
	const CAlphInfo *m_pAlphabet;
	
    ///Utility method for subclasses, lazily makes+caches node labels
    /// \return a node label displaying the specified string
    CDasherScreen::Label *GetLabel(const char *pszConversion);
    
    /// Convert a given string to a lattice of candidates. Sizes for
	/// candidates aren't assigned at this point. The input string
	/// should be UTF-8 encoded.
	///
	/// @param strSource UTF-8 encoded input string.
	/// @param pRoot Used to return the root of the conversion lattice.
	/// @param childCount Unsure - document this.
	/// @param CMid A unique identifier for the conversion helper 'context'.
	///
	/// @return True if conversion succeeded, false otherwise
	virtual bool Convert(const std::string &strSource, SCENode ** pRoot) = 0;
    
	/// Assign sizes to the children of a given conversion node. This
	/// happens when the conversion manager populates the children of
	/// the Dasher node so as to avoid unnecessary computation.
	///
	/// @param pStart The parent of the nodes to be sized.
	/// @param context Unsure - document this, shouldn't be in general class (include userdata pointer).
	/// @param normalization Normalisation constant for the child sizes (TODO: check that this is a sensible value, ie the same as Dasher normalisation).
	/// @param uniform Unsure - document this.
	///
    virtual void AssignSizes(const std::vector<SCENode *> &vChildren, Dasher::CLanguageModel::Context context, long normalization, int uniform)=0;
    
	/// Assign colours to the children of a given conversion node.
	/// This function needs a rethink.
	///
	/// @param parentClr
	/// @param pNode
	/// @param childIndex
	///
	/// @return
	///
	virtual int AssignColour(int parentClr, SCENode * pNode, int childIndex) {
      int which = -1;
      
      for (int i=0; i<2; i++)
        for(int j=0; j<3; j++)
          if (parentClr == colourStore[i][j])
            which = i;
      
      if(which == -1)
        return colourStore[0][childIndex%3];
      else if(which == 0)
        return colourStore[1][childIndex%3];
      else
        return colourStore[0][childIndex%3];
	};
    
    ///
    /// Calculate sizes for each of the children - default
    /// implementation assigns decending probabilities in a power law
    /// fashion (so assumes ordering), but specific subclasses are
    /// free to implement their own behaviour. The only restriction is
    /// that sizes should be positive and sum to the appropriate
    /// normalisation constant
    ///
    
    virtual void AssignChildSizes(const std::vector<SCENode *> &vChildren, CLanguageModel::Context context);
    
    ///
    /// Build the conversion tree (lattice) for the given string -
    /// evaluated late to prevent unnecessary conversions when the
    /// children of the root node are never instantiated
    ///
    
    virtual void BuildTree(CConvNode *pRoot);

  private:

    std::map<std::string, CDasherScreen::Label *> m_vLabels;
    CDasherScreen *m_pScreen;

    ///
    /// Dump tree to provided stream (debug)
    ///

    void RecursiveDumpTree(std::ostream &out, SCENode *pCurrent, unsigned int iDepth);

    CLanguageModel *m_pLanguageModel;
    
    CLanguageModel::Context m_iLearnContext;
    
	int colourStore[2][3];

  };
  /// @}
}

#endif
