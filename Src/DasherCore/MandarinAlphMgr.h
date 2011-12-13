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
  /// This class, along with PPMPYLanguageModel, allows the user to write symbols
  /// in the target alphabet (e.g. Chinese) aka "CH", by first selecting a group,
  /// e.g. Pin Yin sound+tone aka "PY" then picking the output symbol within that
  /// group - with the same output symbol potentially appearing in multiple groups.
  /// Thus each sentence in the target (CH) alphabet may appear in arbitrarily
  /// many different (sub)intervals of the real line, but each with different PY.
  /// The possible mappings between PY and CH are obtained entirely from the
  /// alphabet definition, but must be (a) fixed regardless of context, and
  /// (b) enter precisely one CH symbol for each PY selected.
  ///
  /// In use, the user first navigates into a PY symbol, but this may not enter any text:
  /// instead it may offer a choice between multiple CH symbols or "conversions";
  /// the user navigates into one of these, which is then written, and the process repeats
  /// (PY-CH-PY-CH...). Some PY symbols offer no choice, i.e. only a single CH symbol, in
  /// which case the "navigate into CH" step disappears.
  ///
  /// The language model treats all occurrences of the same CH symbol the same regardless of PY,
  /// and builds a context of CH symbols only; however, for a given CH context, it predicts
  /// both the next PY symbol, and the next CH symbol, using distinct counts. (See GetConversions).
  /// Note that the group structure of the alphabet definition determines the PY-CH
  /// mapping (as follows), i.e. changing the structure can change the treatment
  /// given it by the LM (unlike standard Dasher, where groups are presentational
  /// only): the _set_ of symbols which are the direct children of each group (not
  /// in subgroups) are assigned together to one PY symbol for LM purposes, and
  /// moreover will all be presented together as the first children of the group,
  /// i.e. before any subgroups. (Indeed, they may even be sorted, according to the
  /// PYProbabilitySortThreshold.) The name attribute of the group is used in training
  /// files for disambiguating which PY was used to enter a CH symbol - see
  /// nested class CMandarinTrainer.
  ///
  /// This class is used for alphabets with conversionid==2. 
  class CMandarinAlphMgr : public CAlphabetManager {
  private:    
    /// Trains a PPMPYLanguageModel. The training file is expected to consist of
    /// a sequence of CH syms (+ context switch commands), where CH syms may (and
    /// should, if they appear more than once in the alphabet) be preceded by 
    /// annotations <py> (angle brackets are the default delimiters, alternatives
    /// may be provided in the start & stop attributes of the alphabet conversionid
    /// tag). The PY must identify exactly one group containing the following CH symbol.
    class CMandarinTrainer : public CTrainer {
    public:
      /// Construct a new MandarinTrainer. Reads alphabet etc. directly from pMgr.
      CMandarinTrainer(CMessageDisplay *pMsgs, CMandarinAlphMgr *pMgr);
    protected:
      //override...
      virtual void Train(CAlphabetMap::SymbolStream &syms);
    private:
      CMandarinAlphMgr * const m_pMgr;
      int m_iStartSym;
      symbol getPYsym(bool bHavePy, const string &strPy, symbol symCh);
    };
    
  public:
    /// Create a MandarinAlphabetManager!
    /// \param pAlphabet the Pinyin alphabet. This should have a hierarchy of groups to be
    /// displayed to the user, where all CH symbols that are direct children of the same group,
    /// are to be modelled as sharing the PY sound+tone represented by that group.
    CMandarinAlphMgr(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet);
    ~CMandarinAlphMgr();
    
    ///ACL: returns a MandarinTrainer too.
    CTrainer *GetTrainer();
    
    ///Disable game mode. The target sentence might appear in several places...!!
    CWordGeneratorBase *GetGameWords() {return NULL;}

  protected:
    ///Initializes all our data:
    /// *Chinese alphabet symbols are rehashed into m_vCHtext, m_vCHdisplayText, and m_vCHcolours;
    /// *m_pPYgroups is initialized to a group tree of PY sounds;
    /// *m_vConversionsByGroup, m_vGroupsByConversion record the mapping between the two

    /// the character information methods (GetDisplayText, etc.,
    /// also MakeMap) return information on the target(chinese)-alphabet symbols, which
    /// are rehashed from the original/input alphabet to remove duplicates;
    void InitMap();
    ///WZ: Mandarin Dasher Change. Sets language model to PPMPY.
    void CreateLanguageModel();
    
    ///Process SGroupInfo's from the alphabet into form suitable for m_pPYgroups
    /// \param pBase group from alphabet (i.e. containing unhashed CH symbol numbers)
    /// \return new group, in terms of PY sound+tone/group numbers
    SGroupInfo *makePYgroup(const SGroupInfo *in);
    
    ///Override just to cache the screen so we can make (CH) labels lazily
    void MakeLabels(CDasherScreen *pScreen);

    ///Override just to create root of tree from m_pPYgroups instead of m_pAlphabet
    SGroupInfo *copyGroups(const SGroupInfo *pBase, CDasherScreen *pScreen);

    //Remove label on the "symbols" i.e. PY sounds; we use the label
    // internally to identify a CH group, but for display purposes the symbol is
    // identified entirely by the PY groups around it.
    const std::string &GetLabelText(symbol i) const;

    ///Override to create a CMandSym; context and sym both obtained from the
    // ChineseAlphabet's (hashed) symbols stored in m_map.
    CAlphNode *CreateSymbolRoot(int iOffset, CLanguageModel::Context ctx, symbol sym);

    class CConvRoot;
    ///Subclass of CSymbolNode for (converted) chinese-alphabet symbols:
    /// we need to keep track of the pinyin used to enter it, to output
    /// correct training text and for rebuilding.
    class CMandSym : public CSymbolNode {
    public:
      CMandarinAlphMgr *mgr() const {return static_cast<CMandarinAlphMgr *>(CSymbolNode::mgr());}
      ///Symbol constructor: display text from (CH)Alphabet, colour as superclass = from GetColour
      CMandSym(int iOffset, CMandarinAlphMgr *pMgr, symbol iSymbol, symbol pyParent);
      CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol);
      CMandSym *RebuildCHSymbol(CConvRoot *pParent, symbol iNewSym);
    protected:
      ///Override to compute which pinyin symbol to make our parent...
      void RebuildForwardsFromAncestor(CAlphNode *pNewNode);
      bool isInGroup(const SGroupInfo *pGroup);
      ///Override to read from m_vCHtext
      virtual const std::string &outputText() const;
      ///Override to also output <preceding PY> if ambiguous
      virtual std::string trainText();
    private:
      ///The Pinyin symbol used to produce this chinese symbol, if known (0 if not!)
      symbol m_pyParent;
    };
    ///Offers a choice between a set of chinese symbols, all corresponding to a single PY symbol.
    /// Relative sizes of the CH symbols is obtained by CPPMPYLanguageModel::GetPartProbs, passing
    /// the set of possible CH symbols.
    class CConvRoot : public CAlphBase {
    public:
      /// \param pySym symbol in pinyin alphabet; must have >1 possible chinese conversion.
      CConvRoot(int iOffset, CMandarinAlphMgr *pMgr, symbol pySym);
      CMandarinAlphMgr *mgr() const {return static_cast<CMandarinAlphMgr *>(CAlphBase::mgr());}
      void PopulateChildren();
      void PopulateChildrenWithExisting(CMandSym *existing);
      int ExpectedNumChildren();
      CLanguageModel::Context iContext;
      void SetFlag(int iFlag, bool bValue);
      const symbol m_pySym;
      ///A "symbol" to be rebuilt, is a PY sound, i.e. potentially this
      CDasherNode *RebuildSymbol(CAlphNode *pParent, symbol iSymbol);
    protected:
      bool isInGroup(const SGroupInfo *pGroup);
    private:
      std::vector<std::pair<symbol, unsigned int> > m_vChInfo;
    };
    ///Called to create the node for a pinyin leaf symbol;
    /// Overridden to call either CreateConvRoot or CreateCHSymbol, according to #chinese symbols under specified pinyin
    /// \param iSymbol Symbol number in pinyin alphabet
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol);

    ///Creates a CConvRoot, for a Pinyin symbol with multiple possible chinese symbols.
    /// Colour is always 9 (opaque), so no need for background colour.
    /// \param pParent parent node, context will be taken from here
    /// \param iPYsym Symbol (leaf) in pinyin alphabet
    CConvRoot *CreateConvRoot(CAlphNode *pParent, symbol iPYsym);
      
    ///Creates a node for (i.e. that will actually enter) a chinese symbol
    /// \param pParent parent node: could be a CGroupNode (directly), if some pinyin symbol in that group had only
    /// one corresponding chinese symbol (=> this), or a CConvRoot (if this chinese symbol is one of many possible
    /// chinese symbols for a particular pinyin).
    /// \param iContext parent node's context, from which to generate context for this node
    /// \param iCHsym symbol number in chinese alphabet
    /// \param pyParent pinyin-alphabet symbol which was used to enter this chinese symbol (if known, else 0)
    CMandSym *CreateCHSymbol(CDasherNode *pParent, CLanguageModel::Context iContext, symbol iCHsym, symbol pyParent);

    ///Gets the possible chinese symbols for a pinyin one, along with their probabilities in the specified context.
    ///Probabilities are computed by CPPMPYLanguageModel::GetPartProbs, then renormalized here. (TODO unnecessary?)
    /// \param vChildren initially empty vector which procedure fills with pairs: first element chinese symbol number,
    /// second element probability (/NORMALIZATION).    
    void GetConversions(std::vector<std::pair<symbol,unsigned int> > &vChildren, symbol pySym, Dasher::CLanguageModel::Context context);

    ///Override to get colour for a specified chinese symbol and offset.
    /// Wraps m_vCHcolours getcolour in case anything specified; if not,
    /// supplies defaults (different from GetColour(sym,int)!). Also
    /// implements 2-phase colour cycling by low-bit of offset (as superclass).
    int GetColour(symbol CHsym, int iOffset) const;
    
    ///The same structure as the the original alphabet group tree, but for each
    /// group in the original which contained symbols (directly, not in subgroups),
    /// the new group tree allocates exactly one index (for that PY group), being
    /// the first index in the SGroupInfo. We run the superclass on this group tree,
    /// so it will call CreateSymbolNode for each index (=PY sound) here, and we
    /// override that to create PY "symbols" i.e. CConvRoots
    SGroupInfo *m_pPYgroups;
    
    ///Texts (multiple-octet but single unicode chars) for chinese characters - every element unique
    /// Element 0 is blank, for the "unknown symbol" (easiest to store it)
    ///TODO We're replicating a lot of CAlphInfo here for character storage; could
    /// we just inline a CAlphInfo object and use that?
    std::vector<std::string> m_vCHtext;
    /// Display texts, as per previous
    std::vector<std::string> m_vCHdisplayText;
    ///colour, as per previous
    std::vector<int> m_vCHcolours;
    ///Index of paragraph symbol after rehashing
    int m_iCHpara;
    ///Labels for rehashed chinese symbols, as previous
    std::vector<CDasherScreen::Label *> m_vCHLabels;
    
    ///Keys are sound (Pinyin) numbers, i.e. the first index in an SGroupInfo
    /// containing any child symbols; values are the list of target(CH) alphabet
    // symbols, in the order they appeared in the original group in the input CAlphInfo.
    std::vector<std::vector<symbol> > m_vConversionsByGroup;
    
    ///Keys are (rehashed) chinese-alphabet symbol numbers; values
    /// are the set of pinyin sounds which convert to that key.
    std::vector<std::set<symbol> > m_vGroupsByConversion;
    
    ///Keys are sound (pinyin) numbers; values are the name attributes
    /// of the corresponding SGroupInfos to which those numbers (in the new
    /// tree) are allocated - i.e. the name in the original alphabet
    /// file, of the group containing the conversions which we have joined together
    /// under the new pinyin #.
    std::vector<std::string> m_vGroupNames;

    //Used to create labels lazily
    CDasherScreen *m_pScreen;
    CDasherScreen::Label *GetCHLabel(int iCHsym);
  };
  /// @}

}


#endif
