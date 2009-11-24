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
#define UNITALPH ALPHSIZE/DIVISION
#define UNITPY PYALPHSIZE/DIVISION


namespace Dasher {

  ///
  /// \ingroup LM
  /// @{

  ///
  /// PPM language model (with PinYin)
  ///

  class CPPMPYLanguageModel:public CLanguageModel, private NoClones {
  private:
    class CPPMPYnode {
    public:
      CPPMPYnode * find_symbol(int sym)const;
      CPPMPYnode * find_pysymbol(int pysym)const;
      //Each PPM node store DIVISION number of addresses for children, so that each node branches out DIVISION times (as compared to binary); this is aimed to give better run-time speed
      CPPMPYnode * child[DIVISION];
      CPPMPYnode *next;
      CPPMPYnode *vine;
      //Similarly (as last comment) for Pin Yin 
      CPPMPYnode * pychild[DIVISION];
      unsigned short int count;
      short int symbol;
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
    CPPMPYLanguageModel(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, const CSymbolAlphabet & alph, const CSymbolAlphabet & pyalph);

    virtual ~ CPPMPYLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    virtual void EnterSymbol(Context context, int Symbol);
    virtual void LearnSymbol(Context context, int Symbol);
    virtual void LearnPYSymbol(Context context, int Symbol);

    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm, int iUniform) const;
    void GetPYProbs(Context context, std::vector < unsigned int >&Probs, int norm, int iUniform);

    void GetPartProbs(Context context, const std::vector<SCENode *> &vChildren, int norm, int iUniform);

    void dump();

    virtual bool WriteToFile(std::string strFilename);
    virtual bool ReadFromFile(std::string strFilename);
    bool RecursiveWrite(CPPMPYnode *pNode, std::map<CPPMPYnode *, int> *pmapIdx, int *pNextIdx, std::ofstream *pOutputFile);
    int GetIndex(CPPMPYnode *pAddr, std::map<CPPMPYnode *, int> *pmapIdx, int *pNextIdx);
    CPPMPYnode *GetAddress(int iIndex, std::map<int, CPPMPYnode*> *pMap);

    CPPMPYnode *AddSymbolToNode(CPPMPYnode * pNode, int sym, int *update);
    CPPMPYnode *AddPYSymbolToNode(CPPMPYnode * pNode, int pysym, int *update);

    virtual void AddSymbol(CPPMPYContext & context, int sym);
    void AddPYSymbol(CPPMPYContext & context, int pysym);

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

    CSymbolAlphabet m_pyAlphabet;
    int m_iAlphSize;
    int m_iPYAlphSize;

  };

  /// @}

  inline Dasher::CPPMPYLanguageModel::CPPMPYnode::CPPMPYnode(int sym):symbol(sym) {
    //    child.clear();
    //    pychild.clear();

    next = vine = 0;
    count = 1;

    //Added: Mandarin; Setting initial values
    for(int i =0; i <DIVISION; i++){
      child[i] = NULL;
      pychild[i] = NULL;
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
      pychild[i] = NULL;
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
