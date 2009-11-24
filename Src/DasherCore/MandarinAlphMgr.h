// MandarinAlphMgr.h
//
// Copyright (c) 2009 The Dasher Team
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

#ifndef __mandarinalphmgr_h__
#define __mandarinalphmgr_h__

#include "AlphabetManager.h"

namespace Dasher {

  class CDasherInterfaceBase;

  /// \ingroup Model
  /// @{

  /// Overides methods of AlphabetManager for changes needed for Mandarin Dasher
  ///
  class CMandarinAlphMgr : public CAlphabetManager {
  public:

    CMandarinAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel);

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CAlphNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, bool bEnteredLast, int iOffset);

  protected:
    class CMandNode : public CSymbolNode {
    public:
      CMandNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CAlphabetManager *pMgr, symbol iSymbol);
      virtual void SetFlag(int iFlag, bool bValue);
    };
    CSymbolNode *makeSymbol(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, symbol iSymbol);
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd);
    virtual CLanguageModel::Context CreateSymbolContext(CAlphNode *pParent, symbol iSymbol);
  };
  /// @}

}


#endif
