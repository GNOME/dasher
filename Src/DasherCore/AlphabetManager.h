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

#include "NodeManager.h"
#include "LanguageModelling/LanguageModel.h"

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
  class CAlphabetManager : public CNodeManager {
  public:

    CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel, CLanguageModel::Context iLearnContext);

    ///
    /// Does nothing - alphabet manager isn't reference counted.
    ///

    virtual void Ref() {};
    
    ///
    /// Does nothing - alphabet manager isn't reference counted.
    ///
    
    virtual void Unref() {};

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode );
    void PopulateChildrenWithSymbol( CDasherNode *pNode, int iExistingSymbol, CDasherNode *pExistingChild );

    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode );

    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization);
    virtual void Undo( CDasherNode *pNode );

    virtual CDasherNode *RebuildParent(CDasherNode *pNode);

    virtual void SetFlag(CDasherNode *pNode, int iFlag, bool bValue);

    struct SRootData {
      char *szContext;
      int iOffset;
    };


    struct SAlphabetData {
      symbol iSymbol;
      int iPhase;
      CLanguageModel *pLanguageModel;
      CLanguageModel::Context iContext;
 
      int iGameOffset;
      int iOffset;
    };

  protected:
	virtual CDasherNode *CreateSymbolNode(CDasherNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd, symbol iExistingSymbol, CDasherNode *pExistingChild);
    virtual CLanguageModel::Context CreateSymbolContext(SAlphabetData *pParentData, symbol iSymbol);

    CLanguageModel *m_pLanguageModel;
	CNodeCreationManager *m_pNCManager;

  private:
    
    void BuildContext(std::string& strContext, bool bRoot, CLanguageModel::Context &oContext, symbol &iSymbol);

    void RecursiveIterateGroup(CDasherNode *pParent, SGroupInfo *pInfo, std::vector<symbol> *pSymbols, std::vector<unsigned int> *pCProb, int iMin, int iMax, symbol iExistingSymbol, CDasherNode *pExistingChild);

    CDasherNode *CreateGroupNode(CDasherNode *pParent, SGroupInfo *pInfo, std::vector<unsigned int> *pCProb, unsigned int iStart, unsigned int iEnd, unsigned int iMin, unsigned int iMax);
    CDasherNode *CreateSymbolNode(CDasherNode *pParent, symbol iSymbol, std::vector<unsigned int> *pCProb, unsigned int iStart, unsigned int iEnd, unsigned int iMin, unsigned int iMax, symbol iExistingSymbol, CDasherNode *pExistingChild);

    CLanguageModel::Context m_iLearnContext;
    CDasherInterfaceBase *m_pInterface;

  };
  /// @}

}


#endif
