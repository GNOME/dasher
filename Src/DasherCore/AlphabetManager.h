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

    CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphabetMap, CLanguageModel *pLanguageModel);
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
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd)=0;
      virtual CGroupNode *RebuildGroup(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd)=0;
    private:
      std::vector<unsigned int> *m_pProbInfo;
    protected:
      CAlphabetManager *m_pMgr;
    };
    class CSymbolNode : public CAlphNode {
    public:
      ///Standard constructor, gets colour+label by looking up symbol in current alphabet (& computing phase from offset)
      CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CAlphabetManager *pMgr, symbol iSymbol);

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
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd);
      virtual CGroupNode *RebuildGroup(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd);
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
      CGroupNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CAlphabetManager *pMgr, const SGroupInfo *pGroup);
      
      ///
      /// Provide children for the supplied node
      ///
      virtual CDasherNode *RebuildParent();
      virtual void PopulateChildren();
      virtual int ExpectedNumChildren();
      virtual bool GameSearchNode(std::string strTargetUtf8Char);
      virtual CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd);
      virtual CGroupNode *RebuildGroup(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd);
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
    ///
    /// Factory method for CAlphNode construction, so subclasses can override.
    ///
    virtual CSymbolNode *makeSymbol(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, symbol iSymbol);
    virtual CGroupNode *makeGroup(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const SGroupInfo *pGroup);
    
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd);
    virtual CLanguageModel::Context CreateSymbolContext(CAlphNode *pParent, symbol iSymbol);
    virtual CGroupNode *CreateGroupNode(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd);

    ///Called to add any non-alphabet (non-symbol) children to a top-level node (root or symbol).
    /// Default is just to add the control node, if appropriate.
    virtual void AddExtras(CAlphNode *pParent, std::vector<unsigned int> *pCProb);

    CLanguageModel *m_pLanguageModel;
    CNodeCreationManager *m_pNCManager;
    const CAlphInfo *m_pAlphabet;
    const CAlphabetMap *m_pAlphabetMap;

  private:
    ///Wraps m_pLanguageModel->GetProbs to implement nonuniformity & leave space for control node.
    /// Returns array of non-cumulative probs. Should this be protected and/or virtual???
    void GetProbs(std::vector<unsigned int> *pProbs, CLanguageModel::Context iContext);
    void IterateChildGroups(CAlphNode *pParent, const SGroupInfo *pParentGroup, CAlphNode *buildAround);

    CLanguageModel::Context m_iLearnContext;
    CDasherInterfaceBase *m_pInterface;
    
  };
/// @}

}


#endif
