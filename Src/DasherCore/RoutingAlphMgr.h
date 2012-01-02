//
//  RoutingAlphMgr.h
//  Dasher
//
//  Created by Alan Lawrence on 13/12/11.
//  Copyright 2011 Cambridge University. All rights reserved.
//

#ifndef __RoutingAlphMgr_h__
#define __RoutingAlphMgr_h__

#include "../Common/Common.h"

#include "AlphabetManager.h"
#include "LanguageModelling/RoutingPPMLanguageModel.h"

namespace Dasher {
  
  class CDasherInterfaceBase;
  
  /// \ingroup Model
  /// @{
  
  ///An AlphabetManager that works with alphabets containing duplicate symbols;
  /// hence, an alternative to MandarinAlphMgr. Uses a RoutingPPMLanguageModel,
  /// to separately learn both the output symbols and the ways the user wishes
  /// to write them.
  /// The alphabet + group structure is presented to the user exactly as per
  /// alphabet definition, including repeated symbols, sized as per the LM; groups
  /// play no part in modelling, just being sized to fit around their contents,
  /// as per standard Dasher. However, just as in MandarinAlphMgr, the names of
  /// the groups are used in training files to disambiguate which route was used
  /// to enter a symbol - see nested class CRoutingTrainer. (The aim is that both
  /// this and CMandarinAlphMgr can be used with the same training files.)
  ///
  /// Note we use the term 'base' or 'base symbol' to indicate a particular character
  /// appearing in the output; this may appear multiple times in the alphabet, in
  /// which case each occurrence is called a 'route'.
  ///
  /// This class is used for alphabets with conversionid 3 or 4; the former differs
  /// in treating the route by which the user likes to enter a particular base symbol,
  /// as not dependent on context.
  class CRoutingAlphMgr : public CAlphabetManager {
  public:
    /// Create a RoutingAlphMgr! Changes are in InitMap() and CreateLanguageModel()...
    CRoutingAlphMgr(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet);
    
    ///Override to return a CRoutingTrainer
    CTrainer *GetTrainer();
    
    ///Disable game mode. The target sentence might appear in several places...!!
    CWordGeneratorBase *GetGameWords() {return NULL;}

  protected:
    ///Fills map w/ rehashed base symbols, filling m_vBaseSyms, m_vRoutes,
    /// and m_vGroupsByRoute to record which symbols were identified together.
    void InitMap();
    ///Override to create a RoutingPPMLanguageModel
    void CreateLanguageModel();

    ///Creates a symbol, i.e. including route.
    /// Both ctx and sym were reconstructed from m_map (filled by InitMap), so
    /// are in terms of hashed base symbols; thus, this method identifies the best
    /// route by which that base may have been entered, and creates a symbol node
    /// for that.
    CAlphNode *CreateSymbolRoot(int iOffset, CLanguageModel::Context ctx, symbol sym);
    
    /// Override to create a CRoutedSym and enter only base sym into the LM
    ///\param iSymbol symbol number from the alphabet defn, i.e. identifies both
    /// base symbol and route
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol);

    ///Subclass to override trainText
    class CRoutedSym : public CSymbolNode {
    public:
      string trainText();
      CRoutedSym(int iOffset, CDasherScreen::Label *pLabel, CRoutingAlphMgr *pMgr, symbol iSymbol);
    protected:
      CRoutingAlphMgr *mgr() const {return static_cast<CRoutingAlphMgr*>(m_pMgr);}
    };
    ///Override to provide different defaults! Otherwise as GetColour,
    /// this uses the character data in the alphabet anyway.
    int GetColour(symbol CHsym, int iOffset) const;
  private:
    ///for each (not necessarily unique) symbol in the alphabet, the id of the unique base symbol with that text
    std::vector<symbol> m_vBaseSyms;
    ///for each base symbol, the symbol#'s of all syms-with-routes with that text
    std::vector<std::set<symbol> > m_vRoutes;
    ///closest containing group for each route
    std::vector<const SGroupInfo*> m_vGroupsByRoute;
    
    /// Trains a RoutingPPMLanguageModel. Just as for MandarinAlphMgr/PPMPY, the
    /// training file is expected to consist of a sequence of CH syms (+ context
    /// switch commands), where CH syms may be preceded by annotations <py> 
    /// (angle brackets are the default delimiters, alternatives may be provided
    /// in the start & stop attributes of the alphabet conversionid tag). The PY
    /// should identify exactly one group containing the following CH symbol (or
    /// will be ignored, but the LM handles ambiguous base symbols where no route
    /// is specified, somewhat better than PPMPY).
    class CRoutingTrainer : public CTrainer {
    public:
      CRoutingTrainer(CMessageDisplay *pMsgs, CRoutingAlphMgr *pMgr);
    protected:
      //override...
      virtual void Train(CAlphabetMap::SymbolStream &syms);
    private:
      CRoutingAlphMgr * const m_pMgr;
      ///Symbol # of the start-of-annotation, or 0 if out-of-alphabet
      int m_iStartSym;
      symbol getRoute(bool bHaveRoute, const string &strRoute, symbol baseSym);
    };
  };
  /// @}
  
}

#endif
