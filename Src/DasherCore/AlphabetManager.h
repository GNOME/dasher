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
#include "NodeManager.h"
#include "Trainer.h"
#include "Alphabet/AlphInfo.h"
#include "SettingsStore.h"
#include "Observable.h"
#include "WordGeneratorBase.h"

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
  /// Note Dec11, refactoring to allow subclasses to change how character
  /// data is obtained from the alphabet. All information on valid symbol indices
  /// and the tree of groups, is obtained from m_pBaseGroup, which is created
  /// by a call to copyGroups. Besides this, the only routines accessing _symbol_
  /// data from the alphabet are: CreateLanguageModel; GetTrainer;
  /// GetColour (called from CSymbolNode constructor); CreateSymbolNode and
  /// CSymbolNode::outputText(). [many other routines access e.g. default context, training file, and so on]

  class CAlphabetManager : public CNodeManager, protected CSettingsUser {
  public:
    ///Create a new AlphabetManager. Note, not usable until Setup() called.
    CAlphabetManager(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet);
    
    ///Must be called after construction, before the AlphMgr is used. Calls
    /// InitMap(), looks for a usable context-switch delimiter, and
    /// calls CreateLanguageModel.
    void Setup();

    virtual void MakeLabels(CDasherScreen *pScreen);
    ///Gets a new trainer to train this LM. Caller is responsible for deallocating the
    /// trainer later.
    virtual CTrainer *GetTrainer();
    
    /// Gets a (Game) Word Generator to make target sentences for the current alphabet
    CWordGeneratorBase *GetGameWords();

    virtual ~CAlphabetManager();
    /// Flush to the user's training file everything written in this AlphMgr
    /// \param pInterface to use for I/O by calling WriteTrainFile(fname,txt)
    void WriteTrainFileFull(CDasherInterfaceBase *pInterface);
  protected:
    ///Initializes the alphabet map (m_map) from the characters in the alphabet.
    /// Called from Setup(), i.e. before the manager is or need be usable.
    /// The default adds all symbols in the alphabet to the map (inc. dealing
    /// with the paragraph symbol, if any), and DASHER_ASSERTs that all such
    /// characters have distinct texts.
    virtual void InitMap();
    
    ///Creates the LM, and stores in m_pLanguageModel.
    /// Default implementation switches on LP_LANGUAGE_MODEL_ID.
    /// Note subclasses changing the interpretation of the AlphInfo, should override
    /// this to take account of its new meaning.
    virtual void CreateLanguageModel();

    ///Base of all group+character information presented to the user;
    /// created by calling copyGroups on the alphabet.
    SGroupInfo *m_pBaseGroup;
    ///Called to create the base group the AlphMgr will use from the alphabet.
    /// The default implementation elides all single-element groups, and fills in
    /// m_mGroupLabels and m_vLabels using the supplied screen; subclasses may
    /// override to do more, but should call the superclass method to set up the
    /// labels too.
    /// (Note: each invocation creates labels for all symbols in pBase, *and*
    /// all symbols in any later siblings of pBase (by recursive call on pNext).
    /// Of those, symbols in any child groups may be made by recursive call on
    /// pChild, but only if pBase has >1 child node (symbol/group).)
    virtual SGroupInfo *copyGroups(const SGroupInfo *pBase, CDasherScreen *pScreen);
    
    ///A label for each group in the elided tree
    std::map<const SGroupInfo *,CDasherScreen::Label *> m_mGroupLabels;
    ///A label for each symbol, indexed by symbol id (element 0 = null)
    std::vector<CDasherScreen::Label *> m_vLabels;
    
    virtual const std::string &GetLabelText(symbol i) const;
    
    class CAlphNode;
    /// Abstract superclass for alphabet manager nodes, provides common implementation
    /// code for rebuilding parent nodes = reversing.
    class CAlphBase : public CDasherNode {
    public:
      CAlphabetManager *mgr() const {return m_pMgr;}
      ///Rebuilds this node's parent by recreating the previous 'root' node,
      /// then calling RebuildForwardsFromAncestor
      CDasherNode *RebuildParent();
      ///Called to build a symbol (leaf) node which is a descendant of the symbol or root node preceding this.
      /// Default implementation just calls the manager's CreateSymbolNode method to create a new node,
      /// but subclasses can override to graft themselves into the appropriate point beneath the previous node.
      /// \param pParent parent of the symbol node to create; could be the previous root, or an intervening node (e.g. group)
      /// \param iBkgCol background colour to show through any new transparent node created;
      /// if the existing node is grafted in, again this will already have been taken into account.
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol);
      ///Called to build a group node which is a descendant of the symbol or root node preceding this.
      /// Default implementation calls the manager's CreateGroupNode method to create a new node,
      /// but then populates that group (i.e. further descends the hierarchy) _if_ that group
      /// would contain this node (see IsInGroup). Subclasses can override to graft themselves into the hierarchy, if appropriate.
      /// \param pParent parent of the symbol node to create; could be the previous root, or an intervening node (e.g. group)
      virtual CDasherNode *RebuildGroup(CAlphNode *pParent, int iBkgCol, const SGroupInfo *pInfo);
      ///Just keep track of the last node output (for training file purposes)
      void Undo();
      ///Just keep track of the last node output (for training file purposes)
      void Output();
    protected:
      ///Called in process of rebuilding parent: fill in the hierarchy _beneath_ the
      /// the previous root node, by calling IterateChildGroups passing this node as
      /// last parameter, until the point where this node fits in is found,
      /// at which point RebuildSymbol/Group should graft it in.
      /// \param pNewNode newly-created root node beneath which this node should fit
      virtual void RebuildForwardsFromAncestor(CAlphNode *pNewNode);
      CAlphBase(int iOffset, int iColour, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr);
      CAlphabetManager *m_pMgr;
      ///Number of unicode characters entered by this node; i.e., the number
      /// to take off this node's offset, to get the offset of the most-recent
      /// root (e.g. previous symbol). Default is 0.
      virtual int numChars() {return 0;}
      ///return true if the specified group would contain this node
      /// (as a symbol or subgroup), any number of levels beneath it
      virtual bool isInGroup(const SGroupInfo *pGroup)=0;
    };
    ///Additionally stores LM contexts and probabilities calculated therefrom
    class CAlphNode : public CAlphBase {
    public:
      CAlphNode(int iOffset, int iColour, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr);
      CLanguageModel::Context iContext;
      ///
      /// Delete any storage alocated for this node
      ///
      virtual ~CAlphNode();
      ///Have to call this from CAlphabetManager, and from CGroupNode on a _different_ CAlphNode, hence public...
      virtual std::vector<unsigned int> *GetProbInfo();
      virtual int ExpectedNumChildren();
    private:
      std::vector<unsigned int> *m_pProbInfo;
    };
    class CSymbolNode : public CAlphNode {
    public:
      ///Standard constructor, gets colour from GetColour(symbol,offset) and label from current alphabet
      /// Note we treat GetColour() as always returning an opaque color.
      CSymbolNode(int iOffset, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr, symbol iSymbol);

      ///Create the children of this node, by starting traversal of the alphabet from the top
      virtual void PopulateChildren();
      virtual void Output();
      virtual void Undo();
      ///Override to provide symbol number, probability, _edit_ text from alphabet
      virtual SymbolProb GetSymbolProb() const;

      virtual void SetFlag(int iFlag, bool bValue);

      virtual bool GameSearchNode(symbol sym);
      virtual void GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, std::vector<symbol> &vContextSymbols, int iOffset, int iLength);
      virtual symbol GetAlphSymbol();
      ///Override: if the symbol to create is the same as this node's symbol, return this node instead of creating a new one
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol);
    protected:
      virtual const std::string &outputText() const;
      ///Text to write to user training file/buffer when this symbol output.
      /// Default just returns the output text escaped if necessary.
      virtual std::string trainText();
      /// Number of unicode _characters_ (not octets) for this symbol.
      /// Uniquely, a paragraph symbol can enter two distinct unicode characters
      /// (i.e. '\r' and '\n'); every other symbol enters only a single
      /// unicode char, even if that might take >1 octet.
      int numChars();
      ///Compatibility constructor, so that subclasses can specify their own colour & label
      CSymbolNode(int iOffset, int iColour, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr, symbol _iSymbol);
      ///Override: true iff pGroup encloses this symbol (according to its start/end symbol#)
      bool isInGroup(const SGroupInfo *pGroup);
      const symbol iSymbol;
    };

    class CGroupNode : public CAlphNode {
    public:
      CGroupNode(int iOffset, CDasherScreen::Label *pLabel, int iBkgCol, CAlphabetManager *pMgr, const SGroupInfo *pGroup);

      ///Override: if m_pGroup==NULL, i.e. whole/root-of alphabet, cannot rebuild.
      virtual CDasherNode *RebuildParent();

      ///Create children of this group node, by traversing the section of the alphabet
      /// indicated by m_pGroup.
      virtual void PopulateChildren();
      virtual int ExpectedNumChildren();
      virtual bool GameSearchNode(symbol sym);
      std::vector<unsigned int> *GetProbInfo();
      ///Override: if the group to create is the same as this node's group, return this node instead of creating a new one
      virtual CDasherNode *RebuildGroup(CAlphNode *pParent, int iBkgCol, const SGroupInfo *pInfo);
    protected:
      ///Override: true if pGroup encloses this one (by start/end symbol#)
      bool isInGroup(const SGroupInfo *pGroup);
    private:
      const SGroupInfo *m_pGroup;
    };

  public:
    ///
    /// Get a new root node owned by this manager
    /// pContext - node from which to extract context (e.g. perhaps an un-seen node);
    /// the new root is NOT made  a child, and initially has no parent.
    /// bEnteredLast - true if this "root" node should be considered as entering the preceding symbol
    /// Offset is the index of the character which _child_ nodes (i.e. between which this root allows selection)
    /// will enter. (Also used to build context for preceding characters.)
    /// Note, the new node will _not_ be NF_SEEN
    CAlphNode *GetRoot(CDasherNode *pContext, bool bEnteredLast, int iOffset);

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
    /// \param iBkgCol colour behind the new node, i.e. that should show through if the (group) node is transparent
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol);
    virtual CGroupNode *CreateGroupNode(CAlphNode *pParent, int iBkgCol, const SGroupInfo *pInfo);
    ///Called to create a new symbol root, e.g. for going backwards
    /// \param iOffset index of symbol entered by the node
    /// \param sym symbol number as returned as first element of GetContextSymbols
    virtual CAlphNode *CreateSymbolRoot(int iOffset, CLanguageModel::Context ctx, symbol sym);
    
    ///Called to compute colour for a symbol at a specified offset.
    /// Wraps CAlphabet::GetColour(sym), but (a) implements a default
    ///  scheme for symbols not specifying a colour, and (b) implements
    /// colour-cycling by phase (two cycles, using the LSBit of offset)
    virtual int GetColour(symbol sym, int iOffset) const;
    
    CDasherInterfaceBase * const m_pInterface;

    CLanguageModel *m_pLanguageModel;

    CNodeCreationManager *m_pNCManager;
    const CAlphInfo *m_pAlphabet;
    CAlphabetMap m_map;
    
  private:
    ///Wraps m_pLanguageModel->GetProbs to implement nonuniformity
    /// (also leaves space for NCManager::AddExtras to add control node)
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
    void IterateChildGroups(CAlphNode *pParent, const SGroupInfo *pParentGroup, CAlphBase *buildAround);

    ///Last node (owned by this manager) that was output; if a node
    /// is Undo()ne, this is set to its parent. This is used to detect
    /// context switches.
    CDasherNode *m_pLastOutput;
    ///Text actually written in the current context; both appended and truncated
    /// as nodes are Output() and Undo()ne.
    std::string strTrainfileBuffer;
    ///Context in (i.e. after) which anything in strTrainfileBuffer was written.
    /// Set when first character put in strTrainfileBuffer (following a context switch),
    /// as we may not be able to get the preceding characters if we wait too long.
    std::string strTrainfileContext;

    ///A character, 33<=c<=255, not in the alphabet; used to delimit contexts.
    ///"" if no such could be found (=> will be found on a per-context basis)
    std::string m_sDelim;
  };
/// @}

}


#endif
