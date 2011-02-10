// AlphabetManager.h
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

#ifndef __alphabetmanager_h__
#define __alphabetmanager_h__

#include "LanguageModelling/LanguageModel.h"
#include "DasherNode.h"
#include "Parameters.h"
#include "NodeManager.h"
#include "Trainer.h"

class CNodeCreationManager;
struct SGroupInfo;

namespace Dasher {

  class CDasherInterfaceBase;
  
  /// \ingroup Model
  /// @{

  /// Implementation of CNodeManager for regular 'alphabet' nodes, ie
  /// the basic Dasher behaviour. Child nodes are populated according
  /// to the appropriate alphabet file, with sizes given by the
  /// language model.
  ///
  class CAlphabetManager : public CNodeManager {
  public:
    ///Create a new AlphabetManager. Note, not usable until CreateLanguageModel() called.
    CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphabetMap);
    ///Creates the LM, and stores in m_pLanguageModel. Must be called after construction,
    /// before the AlphMgr is used. Default implementation switches on LP_LANGUAGE_MODEL_ID.
    virtual void CreateLanguageModel(CEventHandler *pEventHandler, CSettingsStore *pSets);

    ///Gets a new trainer to train this LM. Caller is responsible for deallocating the
    /// trainer later.
    virtual CTrainer *GetTrainer();

    virtual ~CAlphabetManager();

  protected:
    class CGroupNode;
    class CAlphNode : public CDasherNode {
    public:
      virtual CAlphabetManager *mgr() {return m_pMgr;}
      CAlphNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const std::string &strDisplayText, CAlphabetManager *pMgr);
      CLanguageModel::Context iContext;
      ///
      /// Delete any storage alocated for this node
      ///      
      virtual ~CAlphNode();
      virtual CLanguageModel::Context CloneAlphContext(CLanguageModel *pLanguageModel);
      CDasherNode *RebuildParent(int iNewOffset);
      ///Have to call this from CAlphabetManager, and from CGroupNode on a _different_ CAlphNode, hence public...
      virtual std::vector<unsigned int> *GetProbInfo();
      virtual int ExpectedNumChildren();
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol)=0;
      virtual CDasherNode *RebuildGroup(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo)=0;
    private:
      std::vector<unsigned int> *m_pProbInfo;
    protected:
      CAlphabetManager *m_pMgr;
    };
    class CSymbolNode : public CAlphNode {
    public:
      ///Standard constructor, gets colour from GetColour(symbol,offset) and label from current alphabet
      /// \param strGroup caption of any enclosing group(s) of which this symbol is a singleton child
      /// - this is prepended onto the symbol caption. Note, we don't need the "background colour" of
      /// any such group, as GetColour() always returns an opaque color.
      CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, CAlphabetManager *pMgr, symbol iSymbol);

      ///
      /// Provide children for the supplied node
      ///

      virtual void PopulateChildren();
      virtual CDasherNode *RebuildParent();
      virtual void Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);
      virtual void Undo(int *pNumDeleted);

      virtual void SetFlag(int iFlag, bool bValue);

      virtual bool GameSearchNode(std::string strTargetUtf8Char);
      virtual void GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, std::vector<symbol> &vContextSymbols, int iOffset, int iLength);
      virtual symbol GetAlphSymbol();
      const symbol iSymbol;
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol);
      virtual CDasherNode *RebuildGroup(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo);
    protected:
      virtual const std::string &outputText();
      /// Number of unicode _characters_ (not octets) for this symbol.
      /// Uniquely, a paragraph symbol can enter two distinct unicode characters
      /// (i.e. '\r' and '\n'); every other symbol enters only a single 
      /// unicode char, even if that might take >1 octet.
      int numChars();
      ///Compatibility constructor, so that subclasses can specify their own colour & label
      CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const std::string &strDisplayText, CAlphabetManager *pMgr, symbol _iSymbol);
    };

    class CGroupNode : public CAlphNode {
    public:
      CGroupNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, CAlphabetManager *pMgr, const SGroupInfo *pGroup);
      
      ///
      /// Provide children for the supplied node
      ///
      virtual CDasherNode *RebuildParent();
      virtual void PopulateChildren();
      virtual int ExpectedNumChildren();
      virtual bool GameSearchNode(std::string strTargetUtf8Char);
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol);
      virtual CDasherNode *RebuildGroup(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo);
      std::vector<unsigned int> *GetProbInfo();
    private:
      const SGroupInfo *m_pGroup;
    };
    
  public:
    ///
    /// Get a new root node owned by this manager
    /// bEnteredLast - true if this "root" node should be considered as entering the preceding symbol
    /// Offset is the index of the character which _child_ nodes (i.e. between which this root allows selection)
    /// will enter. (Also used to build context for preceding characters.)
    /// Note, the new node will _not_ be NF_SEEN
    virtual CAlphNode *GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset);

    const CAlphInfo *GetAlphabet() const;
    
  protected:
    ///Called to get the symbols in the context for (preceding) a new node
    /// \param pParent node to assume has been output, when obtaining context
    /// \param iRootOffset offset of the node that will be constructed; i.e. context should include symbols
    /// up to & including this offset.
    /// \param pAlphMap use to convert entered text into symbol numbers
    /// (could be the managers m_pAlphabetMap, but subclasses can pass in something different)
    /// \return pair: first element is the last symbol in the context, _if_ a usable context
    /// could be extracted, else 0 (=> couldn't get context, using alphabet default); second
    /// element is the result of entering the symbols retrieved, into a fresh LM context.
    std::pair<symbol, CLanguageModel::Context> GetContextSymbols(CDasherNode *pParent, int iRootOffset, const CAlphabetMap *pAlphMap);
    
    ///Called to create a node for a given symbol (leaf), as a child of a specified parent node
    /// \param strGroup caption of any group containing this node, that will not be created:
    /// thus, should be prepended onto the caption of the node created.
    /// \param iBkgCol colour behind the new node, i.e. that should show through if the node is transparent
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol);
    virtual CGroupNode *CreateGroupNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo);

    ///Called to add any non-alphabet (non-symbol) children to a top-level node (root or symbol).
    /// Default is just to add the control node, if appropriate.
    virtual void AddExtras(CAlphNode *pParent, std::vector<unsigned int> *pCProb);

    ///Called to compute colour for a symbol at a specified offset.
    /// Wraps CAlphabet::GetColour(sym), but (a) implements a default
    ///  scheme for symbols not specifying a colour, and (b) implements
    /// colour-cycling by phase (two cycles, using the LSBit of offset)
    virtual int GetColour(symbol sym, int iOffset) const;
    
    CLanguageModel *m_pLanguageModel;
    CLanguageModel::Context m_iLearnContext;

    CNodeCreationManager *m_pNCManager;
    const CAlphInfo *m_pAlphabet;
    const CAlphabetMap *m_pAlphabetMap;

  private:
    ///Wraps m_pLanguageModel->GetProbs to implement nonuniformity & leave space for control node.
    /// Returns array of non-cumulative probs. Should this be protected and/or virtual???
    void GetProbs(std::vector<unsigned int> *pProbs, CLanguageModel::Context iContext);
    ///Constructs child nodes under the specified parent according to provided group.
    /// Nodes are created by calling CreateSymbolNode and CreateGroupNode, unless buildAround is non-null.
    /// \param pParentGroup group describing which symbols and/or subgroups should be constructed
    /// (these will fill the parent), or NULL meaning the entire alphabet (i.e. toplevel groups
    /// and symbols not in any group).
    /// \param buildAround if non-null, its RebuildSymbol and RebuildGroup methods will be called
    /// instead of the AlphabetManager's CreateSymbolNode/CreateGroupNode methods. This is used when
    /// rebuilding parents: passing in the pre-existing node here, allows it to intercept those calls
    /// and graft itself in in place of a new node, when appropriate.
    void IterateChildGroups(CAlphNode *pParent, const SGroupInfo *pParentGroup, CAlphNode *buildAround);

    CDasherInterfaceBase *m_pInterface;
    
  };
/// @}

}


#endif
