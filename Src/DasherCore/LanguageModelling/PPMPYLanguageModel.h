// PPMPYLanguageModel.h
//
// Mandarin character - py prediction by a extension in PPM (subtrees attached to Symbol nodes)
//
// Started from a replicate of PPMLanguageModel
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PPMPYLanguageModel_h__
#define __PPMPYLanguageModel_h__

#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"

#include "PPMLanguageModel.h"

#include <vector>

namespace Dasher {

  ///
  /// \ingroup LM
  /// @{

  ///
  /// PPM language model (with PinYin). Implements a standard PPM model amongst chinese characters,
  /// but with each ppm-node additionally storing counts of possible Pinyin symbols which might be
  /// entered in that context. GetProbs returns probabilities for the next Pinyin symbol, which (NB!)
  /// is _not_ entered into the context; new method GetPartProbs is used to compute probabilities
  /// for the next chinese symbol (which should be entered into context), by filtering to a set.
  ///
  /// That is: from the superclass (CAbstractPPM) perspective, the alphabet is the chinese one;
  /// hence, contexts store chinese symbols only, and EnterSymbol+LearnSymbol should be called
  /// with _chinese_ symbol numbers. All PY-alph details are handled in this subclass, with extra
  /// LearnPYSymbol method for updating the LM's pinyin predictions.
  class CPPMPYLanguageModel : public CAbstractPPM {
  public:
    ///Construct a new PPMPYLanguageModel. 
    /// \param iNumCHSyms number of symbols in the alphabet in which we actually want to write (i.e. Chinese),
    /// i.e. from which contexts are formed; this is passed to the CAbstractPPM superclass.
    /// \param iNumPYSyms number of pinyin phonemes, i.e. which we generate probabilities for in GetProbs
    /// based (only) on the preceding _Chinese_ symbols.
    CPPMPYLanguageModel(CSettingsUser *pCreator, int iNumCHsyms, int iNumPYsyms);

    ///Learns a pinyin symbol in the specified context, but does not move the context on.
    void LearnPYSymbol(Context context, int Symbol);

    ///Predicts probabilities for the next Pinyin symbol (blending as per PPM,
    /// but using the pychild map rather than child CPPMPYnodes).
    /// \param Probs vector to fill with predictions for pinyin symbols: will be filled
    ///  with m_iNumPYsyms numbers plus an initial 0. 
    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm, int iUniform) const;
    
    ///Predicts probabilities for the next Chinese symbol, filtered to only include symbols within a specified set.
    /// Predictions are made as per PPM, but considering only counts for the specified symbols; this means
    /// the value of LP_LM_ALPHA is relative to the total counts of _those_ chinese symbols (in the specified
    /// context), not to the total count of all chinese symbols in that context.
    /// \param vChildren vector of (chinese symbol, probability) pairs; on entry, the first element of each pair
    /// indicates a possible chinese symbol; on exit, the second element will have been filled in.
    void GetPartProbs(Context context, std::vector<std::pair<symbol, unsigned int> > &vChildren, int norm, int iUniform);

    virtual bool WriteToFile(std::string strFilename);
    virtual bool ReadFromFile(std::string strFilename);

  protected:
    class CPPMPYnode : public CPPMnode {
    public:
      /// map from pinyin-symbol to count: the number of times each pinyin symbol has been seen in this context
      std::map<symbol,unsigned short int> pychild;
      inline CPPMPYnode(int sym) : CPPMnode(sym) {}
      inline CPPMPYnode() : CPPMnode() {}
    };
    CPPMPYnode *makeNode(int sym);
    
  private:
    int NodesAllocated;
    mutable CSimplePooledAlloc < CPPMPYnode > m_NodeAlloc;

    const int m_iNumPYsyms;
  };

  /// @}  
}                               // end namespace Dasher

#endif // __LanguageModelling__PPMPYLanguageModel_h__
