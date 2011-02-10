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
    
    //Override to use chinese, not pinyin, alphabet to turn extracted text into symbol numbers; and to create
    // chinese symbol nodes, not pinyin ones.
    CAlphNode *GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset);    
    
  protected:
    ///Subclass of CSymbolNode for (converted) chinese-alphabet symbols:
    /// these (a) disable learn-as-you-write (not supported in Mandarin Dasher),
    /// (b) use the chinese (not pinyin, as CSymbolNode would) alphabet for text to display/enter
    /// (c) determine their colour using GetCHColour rather than GetColour.
    class CMandSym : public CSymbolNode {
    public:
      CMandarinAlphMgr *mgr() {return static_cast<CMandarinAlphMgr *>(CSymbolNode::mgr());}
      ///Symbol constructor: display text from CHAlphabet, colour from GetCHColour
      /// \param strGroup caption of any group(s) containing this symbol for which no nodes created; prepended to display text.
      CMandSym(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, CMandarinAlphMgr *pMgr, symbol iSymbol);
      ///Disable learn-as-you-write
      virtual void SetFlag(int iFlag, bool bValue);
      ///Rebuilding not supported
      virtual CDasherNode *RebuildParent() {return 0;}
    private:
      virtual const std::string &outputText();
    };
    ///Offers a choice between a set of chinese symbols, all corresponding to a single PY symbol.
    /// Relative sizes of the CH symbols is obtained by CPPMPYLanguageModel::GetPartProbs, passing
    /// the set of possible CH symbols.
    class CConvRoot : public CDasherNode {
    public:
      CMandarinAlphMgr *mgr() {return m_pMgr;}
      /// \param pConversions set of chinese-alphabet symbol numbers that the PY can convert to; must have >1 element.
      CConvRoot(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, CMandarinAlphMgr *pMgr, const std::set<symbol> *pConversions);
      void PopulateChildren();
      int ExpectedNumChildren();
      CLanguageModel::Context iContext;
    private:        
      std::vector<std::pair<symbol, unsigned int> > m_vChInfo;
      CMandarinAlphMgr *m_pMgr;
      const std::set<symbol> *m_pConversions;
    };
    ///Called to create the node for a pinyin leaf symbol;
    /// Overridden to call either CreateConvRoot or CreateCHSymbol, according to #chinese symbols under specified pinyin
    /// \param iSymbol Symbol number in pinyin alphabet
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol);

    ///Creates a CConvRoot, for a Pinyin symbol with multiple possible chinese symbols.
    /// Colour is always 9 (opaque), so no need for background colour.
    /// \param pParent parent node, context will be taken from here
    /// \param iPYsym Symbol (leaf) in pinyin alphabet
    CConvRoot *CreateConvRoot(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, symbol iPYsym);
      
    ///Creates a node for (i.e. that will actually enter) a chinese symbol
    /// \param pParent parent node: could be a CGroupNode (directly), if some pinyin symbol in that group had only
    /// one corresponding chinese symbol (=> this), or a CConvRoot (if this chinese symbol is one of many possible
    /// chinese symbols for a particular pinyin).
    /// \param iContext parent node's context, from which to generate context for this node
    /// \param strGroup caption of any elided groups (prepended to this node's caption)
    /// \param iCHsym symbol number in chinese alphabet
    CMandSym *CreateCHSymbol(CDasherNode *pParent, CLanguageModel::Context iContext, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, symbol iCHsym);

    void AssignSizes(std::vector<std::pair<symbol,unsigned int> > &vChildren, Dasher::CLanguageModel::Context context);

    ///Gets colour for a specified chinese symbol and offset.
    /// Wraps CHalphabet getcolour in case anything specified; if not,
    /// supplies defaults (different from GetColour(sym,int)!). Also
    /// implements 2-phase colour cycling by low-bit of offset (as GetColour).
    int GetCHColour(symbol CHsym, int iOffset) const;
    
    const CAlphInfo *m_pCHAlphabet;
    const CAlphabetMap *m_pCHAlphabetMap;
    ///Indexed by SPY (syll+tone) alphabet symbol number,
    // the set of CHAlphabet symbols it can be converted to.
    std::set<symbol> *m_pConversionsBySymbol;
  };
  /// @}

}


#endif
