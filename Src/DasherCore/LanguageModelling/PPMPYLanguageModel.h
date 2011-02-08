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

#include "LanguageModel.h"
#include "PPMPYLanguageModel.h"
#include "../SCENode.h"

#include <vector>
#include <fstream>
#include <set>
//Define alphabet sizes
#define ALPHSIZE 7610
#define PYALPHSIZE 1300
//Implement a multi-branch tree, instead of a binary tree to gain speed: a trade-off between speed and memory; the choice of branch is implied by ranking of the symbol being searched/added 
#define DIVISION 5
#define UNITALPH (ALPHSIZE/DIVISION)
#define UNITPY (PYALPHSIZE/DIVISION)


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
  class CPPMPYLanguageModel:public CLanguageModel, private NoClones {
  private:
    class CPPMPYnode {
    public:
      CPPMPYnode * find_symbol(int sym)const;
      //Each PPM node store DIVISION number of addresses for children, so that each node branches out DIVISION times (as compared to binary); this is aimed to give better run-time speed
      CPPMPYnode * child[DIVISION];
      CPPMPYnode *next;
      CPPMPYnode *vine;
      /// map from pinyin-symbol to count: the number of times each pinyin symbol has been seen in this context
      std::map<symbol,unsigned short int> pychild;
      unsigned short int count;
      symbol sym;
      CPPMPYnode(int sym);
      CPPMPYnode();
    };
	  
    class CPPMPYContext {
    public:
      CPPMPYContext(CPPMPYContext const &input) {
        head = input.head;
        order = input.order;
      } CPPMPYContext(CPPMPYnode * _head = 0, int _order = 0):head(_head), order(_order) {
      };
      ~CPPMPYContext() {
      };
      void dump();
      CPPMPYnode *head;
      int order;
    };
	  
  public:
    ///Construct a new PPMPYLanguageModel.
    /// \param pAlph alphabet containing the actual symbols we want to write (i.e. Chinese)
    /// \param pPyAlph alphabet of pinyin phonemes; we will predict probabilities for these
    /// based (only) on the preceding _Chinese_ symbols.
    CPPMPYLanguageModel(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, const CAlphInfo *pAlph, const CAlphInfo *pPyAlph);

    virtual ~ CPPMPYLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    ///Advance the context by entering a chinese symbol
    virtual void EnterSymbol(Context context, int Symbol);
    ///Train the LM with the specified Chinese symbol in that context (moves context on)
    virtual void LearnSymbol(Context context, int Symbol);
    ///Learns a pinyin symbol in the specified context, but does not move the context on.
    virtual void LearnPYSymbol(Context context, int Symbol);

    ///Predicts probabilities for the next Pinyin symbol (blending as per PPM,
    /// but using the pychild map rather than child CPPMPYnodes).
    /// \param Probs vector to fill with predictions for pinyin symbols: will be filled
    ///  with m_pPyAlphabet->GetNumberTextSymbols() numbers plus an initial 0. 
    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm, int iUniform) const;
    
    ///Predicts probabilities for the next Chinese symbol, filtered to only include symbols within a specified set.
    /// Predictions are made as per PPM, but considering only counts for the specified symbols; this means
    /// the value of LP_LM_ALPHA is relative to the total counts of _those_ chinese symbols (in the specified
    /// context), not to the total count of all chinese symbols in that context.
    /// \param vChildren vector of (chinese symbol, probability) pairs; on entry, the first element of each pair
    /// indicates a possible chinese symbol; on exit, the second element will have been filled in.
    void GetPartProbs(Context context, std::vector<std::pair<symbol, unsigned int> > &vChildren, int norm, int iUniform);

    void dump();

    virtual bool WriteToFile(std::string strFilename);
    virtual bool ReadFromFile(std::string strFilename);
    bool RecursiveWrite(CPPMPYnode *pNode, std::map<CPPMPYnode *, int> *pmapIdx, int *pNextIdx, std::ofstream *pOutputFile);
    int GetIndex(CPPMPYnode *pAddr, std::map<CPPMPYnode *, int> *pmapIdx, int *pNextIdx);
    CPPMPYnode *GetAddress(int iIndex, std::map<int, CPPMPYnode*> *pMap);

    CPPMPYnode *AddSymbolToNode(CPPMPYnode * pNode, int sym);

    void dumpSymbol(int sym);
    void dumpString(char *str, int pos, int len);
    void dumpTrie(CPPMPYnode * t, int d);

    CPPMPYContext *m_pRootContext;
    CPPMPYnode *m_pRoot;

    int m_iMaxOrder;
    double m_dBackOffConstat;

    int NodesAllocated; //inclusive of both Character and PY nodes

    bool bUpdateExclusion;


    
    mutable CSimplePooledAlloc < CPPMPYnode > m_NodeAlloc;
    CPooledAlloc < CPPMPYContext > m_ContextAlloc;

    std::set<const CPPMPYContext *> m_setContexts;

  private:

    const CAlphInfo *m_pPyAlphabet;
    int m_iAlphSize;

  };

  /// @}

  inline Dasher::CPPMPYLanguageModel::CPPMPYnode::CPPMPYnode(int _sym):sym(_sym) {
    //    child.clear();
    //    pychild.clear();

    next = vine = 0;
    count = 1;

    //Added: Mandarin; Setting initial values
    for(int i =0; i <DIVISION; i++){
      child[i] = NULL;
    }
  }

  inline CPPMPYLanguageModel::CPPMPYnode::CPPMPYnode() {
    //   child.clear();
    //   pychild.clear();    

    next = vine = 0;

    count = 1;

    //Added: Mandarin; Setting initial values
    for(int i =0; i <DIVISION; i++){
      child[i] = NULL;
    }

  }

  inline CLanguageModel::Context CPPMPYLanguageModel::CreateEmptyContext() {
    CPPMPYContext *pCont = m_ContextAlloc.Alloc();
    *pCont = *m_pRootContext;

    //    m_setContexts.insert(pCont);

    return (Context) pCont;
  }

  inline CLanguageModel::Context CPPMPYLanguageModel::CloneContext(Context Copy) {
    CPPMPYContext *pCont = m_ContextAlloc.Alloc();
    CPPMPYContext *pCopy = (CPPMPYContext *) Copy;
    *pCont = *pCopy;

    //    m_setContexts.insert(pCont);

    return (Context) pCont;
  }

  inline void CPPMPYLanguageModel::ReleaseContext(Context release) {

    //    m_setContexts.erase(m_setContexts.find((CPPMPYContext *) release));

    m_ContextAlloc.Free((CPPMPYContext *) release);
  }
}                               // end namespace Dasher

#endif // __LanguageModelling__PPMPYLanguageModel_h__
