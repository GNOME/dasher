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
#include <set>
namespace Dasher {

  class CDasherInterfaceBase;

  /// \ingroup Model
  /// @{

  /// Overides methods of AlphabetManager for changes needed for Mandarin Dasher
  ///
  class CMandarinAlphMgr : public CAlphabetManager {
  public:

    CMandarinAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphMap);
    ~CMandarinAlphMgr();
    
    ///WZ: Mandarin Dasher Change. Sets language model to PPMPY.
    void CreateLanguageModel(CEventHandler *pEventHandler, CSettingsStore *pSets);
    ///ACL: returns a MandarinTrainer too.
    CTrainer *GetTrainer();
    
    /*ACL note: used to override GetRoot,
     to attempt to clone the context of the previous node
     in the case that the previous node was a PinyinConversionHelper node
     (the common case - when a "conversion" was performed and chinese symbols reached,
     it then 'escaped' back to the alphabet manager root by calling GetAlphRoot...)
     Since this is no longer necessary (chinese symbol nodes are alph nodes directly,
     so subsume the previous role of alph 'root's rather than contain them),
     I don't think we need to override GetRoot anymore...?!?! */
    
  protected:
    ///Subclass CSymbolNode to disable learn-as-you-write (for Mandarin Dasher).
    /// This subclass used directly only for punctuation; chinese symbols use CMandSym, below.
    class CMandNode : public CSymbolNode {
    public:
      CMandarinAlphMgr *mgr() {return static_cast<CMandarinAlphMgr *>(CSymbolNode::mgr());}
      CMandNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CMandarinAlphMgr *pMgr, symbol iSymbol);
      virtual void SetFlag(int iFlag, bool bValue);
      virtual CDasherNode *RebuildParent() {return 0;}
    protected:
      /// Constructor for subclasses (CMandSym!) to specify own colour & label
      CMandNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const std::string &strDisplayText, CMandarinAlphMgr *pMgr, symbol iSymbol);
    };
    class CMandSym : public CMandNode {
    public:
      CMandSym(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, CMandarinAlphMgr *pMgr, symbol iSymbol);
    private:
      virtual const std::string &outputText();
    };
    ///Offers a choice between a set of chinese symbols, all corresponding to a single PY symbol.
    /// Relative sizes of the CH symbols is obtained by CPPMPYLanguageModel::GetPartProbs, passing
    /// the set of possible CH symbols.
    class CConvRoot : public CDasherNode {
    public:
      CMandarinAlphMgr *mgr() {return m_pMgr;}
      /// \param pConversions set of chinese-alphabet symbol numbers that the PY can convert to.
      CConvRoot(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CMandarinAlphMgr *pMgr, const std::set<symbol> *pConversions);
      void PopulateChildren();
      int ExpectedNumChildren();
      int iContext;
    private:        
      std::vector<std::pair<symbol, unsigned int> > m_vChInfo;
      CMandarinAlphMgr *m_pMgr;
      const std::set<symbol> *m_pConversions;
    };
    CMandNode *makeSymbol(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, symbol iSymbol);
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd);
    virtual CLanguageModel::Context CreateSymbolContext(CAlphNode *pParent, symbol iSymbol);

    int AssignColour(int parentClr, int childIndex);
    
    void AssignSizes(std::vector<std::pair<symbol,unsigned int> > &vChildren, Dasher::CLanguageModel::Context context);

    const CAlphInfo *m_pCHAlphabet;
    const CAlphabetMap *m_pCHAlphabetMap;
    ///Indexed by SPY (syll+tone) alphabet symbol number,
    // the set of CHAlphabet symbols it can be converted to.
    std::set<symbol> *m_pConversionsBySymbol;
  };
  /// @}

}


#endif
