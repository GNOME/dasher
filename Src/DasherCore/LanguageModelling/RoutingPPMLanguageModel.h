//
//  RoutingPPMLanguageModel.h
//  Dasher
//
//  Created by Alan Lawrence on 13/12/11.
//  Copyright 2011 Cambridge University. All rights reserved.
//

#ifndef __RoutingPPMLanguageModel_h__
#define __RoutingPPMLanguageModel_h__

#include "PPMLanguageModel.h"

#include <set>

namespace Dasher {
  
  ///
  /// \ingroup LM
  /// @{
  
  ///
  /// Routing Language Model: tries to independently learn a sequence of 'base'
  /// symbols, as per PPM, and also which of multiple routes are used to enter them;
  /// predicts probabilities for (base*route) by dividing probability for base sym
  /// (as per first part of model) up between its possible routes according to
  /// second part of model.
  ///
  /// All contexts are base syms only, so extends PPM over base sym; but overrides
  /// GetProbs to return larger array of probs over (base*route), also LearnSymbol
  /// (to learn base*route; if only base is available, call LearnBaseSymbol).
  /// EnterSymbol (which doesn't do learning) takes base symbols (for context) only.
  class CRoutingPPMLanguageModel : public CAbstractPPM {
  public:
    /// \param pBaseSyms vector identifying the base symbol for each (base+route).
    ///        Thus, size indicates the number of (base+route)s.
    /// \param pRoutes vector identifying all possible route#s for each base sym
    ///        Thus, size indicates the number of base syms.
    /// \param bRoutesContextSensitive if false, the distribution over (routes by
    ///        which a given symbol is entered) is considered independent of context;
    ///        if true, likely routes are learnt according to the preceding context.
    CRoutingPPMLanguageModel(CSettingsUser *pCreator, const std::vector<symbol> *pBaseSyms, const std::vector<std::set<symbol> > *pRoutes, bool bRoutesContextSensitive);
    
    /// Learns a base symbol (but not which route we are likely to enter it by).
    /// Includes moving on the context to include that base sym.
    void LearnBaseSymbol(Context context, int Symbol);
    
    /// Learns a base+route, including moving the context on to include the base.
    void LearnSymbol(Context context, int Symbol);

    ///Note we can only ever enter base symbols.

    ///Returns the most likely route by which a symbol might have been entered
    /// \param ctx context whose most-recent character identifies the base symbol
    ///        in which we are interested.
    symbol GetBestRoute(Context ctx);
        
    ///Predicts probabilities for all (base*route)s.
    /// \param Probs vector to fill with predictions; will be filled m_pBaseSyms->size()
    ///  elements (including initial 0)
    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm, int iUniform) const;

    ///disable file i/o
    virtual bool WriteToFile(std::string strFilename);
    virtual bool ReadFromFile(std::string strFilename);
    
  protected:
    ///Subclass to additionally store counts of route by which this context (i.e.
    /// the last base symbol within) was entered, when we know that.
    class CRoutingPPMnode : public CPPMnode {
    public:
      ///map from route (to the last base sym only) to count by which that route
      /// was definitely used.
      std::map<symbol,unsigned short int> m_routes;
      inline CRoutingPPMnode(int sym) : CPPMnode(sym) {}
      inline CRoutingPPMnode() : CPPMnode() {}
    };
    ///Always returns a CRoutingPPMnode. TODO, work through class and use standard
    /// map-less PPMnodes for unambiguous base syms (which have only one route) ?
    CRoutingPPMnode *makeNode(int sym);
    
  private:
    int NodesAllocated;
    CSimplePooledAlloc < CRoutingPPMnode > m_NodeAlloc;
    const std::vector<symbol> *m_pBaseSyms;
    const std::vector<std::set<symbol> > *m_pRoutes;
    const bool m_bRoutesContextSensitive;
  };
  
  /// @}  
}                               // end namespace Dasher

#endif
