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
  class CAlphabetManager {
  public:

    CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel);

  protected:
    class CAlphNode : public CDasherNode {
    public:
      int mgrId() {return 0;}
      CAlphNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CAlphabetManager *pMgr);
    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren();

    ///
    /// Delete any storage alocated for this node
    ///

    virtual ~CAlphNode();

    virtual void Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);
    virtual void Undo();

    virtual CDasherNode *RebuildParent();

    virtual void SetFlag(int iFlag, bool bValue);

    virtual bool GameSearchNode(std::string strTargetUtf8Char);
    
    virtual CLanguageModel::Context CloneAlphContext(CLanguageModel *pLanguageModel);
    virtual symbol GetAlphSymbol();
    private:
      CAlphabetManager *m_pMgr;
    public: //to AlphabetManager and subclasses only, of course...
    
      symbol iSymbol;
      int iPhase;
      CLanguageModel *pLanguageModel;
      CLanguageModel::Context iContext;
 
      int iGameOffset;
    };

  public:
    ///
    /// Get a new root node owned by this manager
    ///
    // Note that iOffset may be -1, to indicate no root data; in which case,
    // szContext should be null. (szContext may be null in other cases too!)
    virtual CAlphNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, char *szContext, int iOffset);

  protected:
    ///
    /// Factory method for CAlphNode construction, so subclasses can override.
    ///
    virtual CAlphNode *makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo);    

    
    void PopulateChildrenWithSymbol( CAlphNode *pNode, int iExistingSymbol, CDasherNode *pExistingChild );

    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd, symbol iExistingSymbol, CDasherNode *pExistingChild);
    virtual CLanguageModel::Context CreateSymbolContext(CAlphNode *pParent, symbol iSymbol);

    CLanguageModel *m_pLanguageModel;
    CNodeCreationManager *m_pNCManager;

  private:
    
    void BuildContext(std::string& strContext, bool bRoot, CLanguageModel::Context &oContext, symbol &iSymbol);

    void RecursiveIterateGroup(CAlphNode *pParent, SGroupInfo *pInfo, std::vector<symbol> *pSymbols, std::vector<unsigned int> *pCProb, int iMin, int iMax, symbol iExistingSymbol, CDasherNode *pExistingChild);

    CAlphNode *CreateGroupNode(CAlphNode *pParent, SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd);

    CLanguageModel::Context m_iLearnContext;
    CDasherInterfaceBase *m_pInterface;

  };
  /// @}

}


#endif
