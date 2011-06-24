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

  /// Subclass of AlphabetManager, generalizing what's needed for Mandarin Dasher.
  /// This class, along with PPMPYLanguageModel and MandarinTrainer, implements
  /// a two-layer / dual-alphabet system for writing symbols in one alphabet (the
  /// "Chinese" or CH alphabet) by first selecting a symbol in a different, e.g.
  /// phonetic, alphabet (here, the "Pinyin" or PY alphabet).
  /// The idea is to relax the usual idea that each sentence in the target (CH) alphabet
  /// appears in exactly one place on the real line [0,1]; instead, it may appear in arbitrarily
  /// many different (sub)intervals, but with different PY.
  /// The possible mappings between PY and CH must be (a) fixed regardless of context, and
  /// (b) enter precisely one CH symbol for each PY selected; but may be many-many, i.e. the same CH symbol
  /// may appear under multiple PY symbols. It is defined by two alphabet files, tho only the
  /// PY alphabet need be a legal alphabet definition - see constructor.
  /// The language model treats all occurrences of the same CH symbol the same regardless of PY,
  /// and builds a context of CH symbols only; however, for a given CH context, it predicts
  /// both the next PY symbol, and the next CH symbol, using distinct counts. (See GetConversions).
  /// In use, the user first navigates into a PY symbol, but this may not enter any text:
  /// instead it may offer a choice between multiple CH symbols or "conversions";
  /// the user navigates into one of these, which is then written, and the process repeats
  /// (PY-CH-PY-CH...). Some PY symbols offer no choice, i.e. only a single CH symbol, in
  /// which case the "navigate into CH" step disappears.
  /// This class is used for alphabets (e.g. PY) with conversionid==2; the conversion target
  /// attribute of that alphabet identifies the CH alphabet (probably hidden itself).
  class CMandarinAlphMgr : public CAlphabetManager {
  public:
    /// Create a MandarinAlphabetManager!
    /// \param pAlphabet the Pinyin alphabet. This should have a hierarchy of groups to be
    /// displayed to the user, and symbols to be predicted by the LM, whose text attributes
    /// are used to parse the training text. However the symbol display texts are NOT presented
    /// to the user; instead, for each PY symbol, the CH alphabet (i.e. the PY alphabet's
    /// "conversion target") must contain exactly one group with the same _displaytext_.
    /// All (CH) symbols within this (inc. in subgroups, CH hierarchy ignored) are presented to the
    /// user; however, CH symbols are identified by their _text_, i.e. (unlike normal alphabets)
    /// the same text may appear in multiple places in the CHAlphabet, and these will be identified
    /// together (i.e. by hashing on text). Hence, it is not possible to call makeMap() on
    /// the CHAlphabet (this requires the text attributes to be all different), so we rehash here.
    /// \param pAlphabetMap mapping from text to symbol# of the PY alphabet; used for training files.
    CMandarinAlphMgr(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphIO *pAlphIO);
    ~CMandarinAlphMgr();
    
    ///WZ: Mandarin Dasher Change. Sets language model to PPMPY.
    void CreateLanguageModel();
    ///ACL: returns a MandarinTrainer too.
    CTrainer *GetTrainer();
    
    //Override to use chinese, not pinyin, alphabet to turn extracted text into symbol numbers; and to create
    // chinese symbol nodes, not pinyin ones.
    CAlphNode *GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset);    
    
  protected:
    void MakeLabels(CDasherScreen *pScreen);
    class CConvRoot;
    ///Subclass of CSymbolNode for (converted) chinese-alphabet symbols:
    /// these use the chinese alphabet in place of the pinyin one for text to display/enter,
    /// and get their colour using GetCHColour rather than GetColour.
    class CMandSym : public CSymbolNode {
    public:
      CMandarinAlphMgr *mgr() {return static_cast<CMandarinAlphMgr *>(CSymbolNode::mgr());}
      ///Symbol constructor: display text from CHAlphabet, colour from GetCHColour
      CMandSym(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CMandarinAlphMgr *pMgr, symbol iSymbol, symbol pyParent);
      CDasherNode *RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, symbol iSymbol);
      CMandSym *RebuildCHSymbol(CConvRoot *pParent, unsigned int iLbnd, unsigned int iHbnd, symbol iNewSym);
    protected:
      ///Override to compute which pinyin symbol to make our parent...
      void RebuildForwardsFromAncestor(CAlphNode *pNewNode);
      bool isInGroup(const SGroupInfo *pGroup);
    private:
      ///Override to output a triple (delimiter,PY,CH)
      virtual std::string trainText();
      ///Override to use use chinese, not pinyin, alphabet
      virtual const std::string &outputText();
      ///The Pinyin symbol used to produce this chinese symbol, if known (0 if not!)
      symbol m_pyParent;
    };
      ///Offers a choice between a set of chinese symbols, all corresponding to a single PY symbol.
      /// Relative sizes of the CH symbols is obtained by CPPMPYLanguageModel::GetPartProbs, passing
      /// the set of possible CH symbols.
      class CConvRoot : public CAlphBase {
    public:
      /// \param pySym symbol in pinyin alphabet; must have >1 possible chinese conversion.
      CConvRoot(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CMandarinAlphMgr *pMgr, symbol pySym);
      CMandarinAlphMgr *mgr() {return static_cast<CMandarinAlphMgr *>(CAlphBase::mgr());}
      void PopulateChildren();
      void PopulateChildrenWithExisting(CMandSym *existing);
      int ExpectedNumChildren();
      CLanguageModel::Context iContext;
      void SetFlag(int iFlag, bool bValue);
      const symbol m_pySym;
      CDasherNode *RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, symbol iSymbol);
    protected:
      bool isInGroup(const SGroupInfo *pGroup);
    private:        
      std::vector<std::pair<symbol, unsigned int> > m_vChInfo;
    };
    ///Called to create the node for a pinyin leaf symbol;
    /// Overridden to call either CreateConvRoot or CreateCHSymbol, according to #chinese symbols under specified pinyin
    /// \param iSymbol Symbol number in pinyin alphabet
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, symbol iSymbol);

    ///Creates a CConvRoot, for a Pinyin symbol with multiple possible chinese symbols.
    /// Colour is always 9 (opaque), so no need for background colour.
    /// \param pParent parent node, context will be taken from here
    /// \param iPYsym Symbol (leaf) in pinyin alphabet
    CConvRoot *CreateConvRoot(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, symbol iPYsym);
      
    ///Creates a node for (i.e. that will actually enter) a chinese symbol
    /// \param pParent parent node: could be a CGroupNode (directly), if some pinyin symbol in that group had only
    /// one corresponding chinese symbol (=> this), or a CConvRoot (if this chinese symbol is one of many possible
    /// chinese symbols for a particular pinyin).
    /// \param iContext parent node's context, from which to generate context for this node
    /// \param iCHsym symbol number in chinese alphabet
    /// \param pyParent pinyin-alphabet symbol which was used to enter this chinese symbol (if known, else 0)
    CMandSym *CreateCHSymbol(CDasherNode *pParent, CLanguageModel::Context iContext, unsigned int iLbnd, unsigned int iHbnd, symbol iCHsym, symbol pyParent);

    ///Gets the possible chinese symbols for a pinyin one, along with their probabilities in the specified context.
    ///Probabilities are computed by CPPMPYLanguageModel::GetPartProbs, then renormalized here. (TODO unnecessary?)
    /// \param vChildren initially empty vector which procedure fills with pairs: first element chinese symbol number,
    /// second element probability (/NORMALIZATION).    
    void GetConversions(std::vector<std::pair<symbol,unsigned int> > &vChildren, symbol pySym, Dasher::CLanguageModel::Context context);

    ///Gets colour for a specified chinese symbol and offset.
    /// Wraps CHalphabet getcolour in case anything specified; if not,
    /// supplies defaults (different from GetColour(sym,int)!). Also
    /// implements 2-phase colour cycling by low-bit of offset (as GetColour).
    int GetCHColour(symbol CHsym, int iOffset) const;
    
    /// Texts (multiple-octet but single unicode chars) for chinese characters - every element unique
    /// Element 0 is blank, for the "unknown symbol" (easiest to store it)
    std::vector<std::string> m_CHtext;
    /// Display texts, as per previous
    std::vector<std::string> m_CHdisplayText;
    //colour, as per previous
    std::vector<int> m_CHcolours;
    /// Map from unicode char to index into m_CH{text,displayText}
    CAlphabetMap m_CHAlphabetMap;
    
    ///Indexed by SPY (syll+tone) alphabet symbol number,
    // the CHAlphabet symbols it can be converted to, in order
    // of appearance in the CHAlphabet group.
    std::vector<symbol> *m_pConversionsBySymbol;

    ///Indexed by chinese-alphabet symbol number (sparsely: where multiple
    /// chinese-alphabet symbols have the same text, we use only the one
    /// returned by CAlphabetMap::GetSymbols() for that text)
    /// the set of pinyin syllable+tones which could yield that symbol.
    std::map<symbol,std::set<symbol> > m_PinyinByChinese;
    
    //Used to create labels lazily
    CDasherScreen *m_pScreen;
    CDasherScreen::Label *GetLabel(int iCHsym);
  };
  /// @}

}


#endif
