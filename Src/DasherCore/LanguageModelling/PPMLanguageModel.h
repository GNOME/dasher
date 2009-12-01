// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PPMLanguageModel_h__
#define __PPMLanguageModel_h__

#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"

#include "LanguageModel.h"

#include "stdlib.h"
#include <vector>
#include <fstream>
#include <set>

namespace Dasher {

  ///
  /// \ingroup LM
  /// @{
  ///
  /// PPM language model
  ///
  class CPPMLanguageModel:public CLanguageModel, private NoClones {
  private:
    class ChildIterator;
    class CPPMnode {
    private:
      union {
        CPPMnode **m_ppChildren;
        CPPMnode *m_pChild;
      };
      ///Elements in above array, including nulls, as follows:
      /// (a) negative -> absolute value is number of elems in m_ppChildren, but use direct indexing
      /// (b) 1 -> use m_pChild as direct pointer to CPPMnode (no array)
      /// (c) 2-MAX_RUN -> m_ppChildren is unordered array of that many elems
      /// (d) >MAX_RUN ->  m_ppChildren is an inline hash (overflow to next elem) with that many slots
      int m_iNumChildSlots;
      friend class CPPMLanguageModel;
	  public:
      ChildIterator children() const;
      const ChildIterator end() const;
      void AddChild(CPPMnode *pNewChild, int numSymbols);
      CPPMnode * find_symbol(symbol sym)const;
      CPPMnode *vine;
      unsigned short int count;
      symbol sym;
      CPPMnode(symbol sym);
      CPPMnode();
      ~CPPMnode();
      bool eq(CPPMnode *other, std::map<CPPMnode *,CPPMnode *> &equivs);
	  };
    class ChildIterator {
    private:
      void nxt() {
        if (m_ppChild == m_ppStop) return;
        while ((--m_ppChild) != m_ppStop)
          if (*m_ppChild) break;
      }
    public:
      bool operator==(const ChildIterator &other) const {return m_ppChild==other.m_ppChild && m_ppStop == other.m_ppStop;}
      bool operator!=(const ChildIterator &other) const {return m_ppChild!=other.m_ppChild || m_ppStop!=other.m_ppStop;}
      CPPMnode *operator*() const {return (m_ppChild == m_ppStop) ? NULL : *m_ppChild;}
      ChildIterator &operator++() {nxt(); return *this;} //prefix
      ChildIterator operator++(int) {ChildIterator temp(*this); nxt(); return temp;}
      //operator CPPMnode *() {return node;} //implicit conversion
      //operator bool();                     //implicit conversion 2
      ChildIterator(CPPMnode *const *ppChild, CPPMnode *const *ppStop) : m_ppChild(ppChild), m_ppStop(ppStop) {nxt();}
    private:
      CPPMnode *const *m_ppChild, *const *m_ppStop;
    };

    class CPPMContext {
    public:
      CPPMContext(CPPMContext const &input) {
        head = input.head;
        order = input.order;
      } CPPMContext(CPPMnode * _head = 0, int _order = 0):head(_head), order(_order) {
      };
      ~CPPMContext() {
      };
      void dump();
      CPPMnode *head;
      int order;
    };
  public:
    CPPMLanguageModel(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, const CSymbolAlphabet & alph);
    bool eq(CPPMLanguageModel *other);
    virtual ~ CPPMLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    virtual void EnterSymbol(Context context, int Symbol);
    virtual void LearnSymbol(Context context, int Symbol);

    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm, int iUniform) const;

    void dump();

    virtual bool WriteToFile(std::string strFilename);
    virtual bool ReadFromFile(std::string strFilename);
    bool RecursiveWrite(CPPMnode *pNode, CPPMnode *pNextSibling, std::map<CPPMnode *, int> *pmapIdx, int *pNextIdx, std::ofstream *pOutputFile);
    int GetIndex(CPPMnode *pAddr, std::map<CPPMnode *, int> *pmapIdx, int *pNextIdx);
    CPPMnode *GetAddress(int iIndex, std::map<int, CPPMnode*> *pMap);

    CPPMnode *AddSymbolToNode(CPPMnode * pNode, symbol sym, int *update);

    virtual void AddSymbol(CPPMContext & context, symbol sym);
    void dumpSymbol(symbol sym);
    void dumpString(char *str, int pos, int len);
    void dumpTrie(CPPMnode * t, int d);

    CPPMContext *m_pRootContext;
    CPPMnode *m_pRoot;

    int m_iMaxOrder;
    double m_dBackOffConstat;

    int NodesAllocated;

    bool bUpdateExclusion;

    mutable CSimplePooledAlloc < CPPMnode > m_NodeAlloc;
    CPooledAlloc < CPPMContext > m_ContextAlloc;

    std::set<const CPPMContext *> m_setContexts;
  };

  /// @}
  inline CPPMLanguageModel::ChildIterator CPPMLanguageModel::CPPMnode::children() const {
    //if m_iNumChildSlots = 0 / 1, m_ppChildren is direct pointer, else ptr to array (of pointers)
    CPPMnode *const *ppChild = (m_iNumChildSlots == 0 || m_iNumChildSlots == 1) ? &m_pChild : m_ppChildren;
    return ChildIterator(ppChild + abs(m_iNumChildSlots), ppChild - 1);
  }
  
  inline const CPPMLanguageModel::ChildIterator CPPMLanguageModel::CPPMnode::end() const {
    //if m_iNumChildSlots = 0 / 1, m_ppChildren is direct pointer, else ptr to array (of pointers)
    CPPMnode *const *ppChild = (m_iNumChildSlots == 0 || m_iNumChildSlots == 1) ? &m_pChild : m_ppChildren;
    return ChildIterator(ppChild, ppChild - 1);
  }

  inline Dasher::CPPMLanguageModel::CPPMnode::CPPMnode(symbol _sym): sym(_sym) {
    vine = 0;
    m_iNumChildSlots = 0;
    m_ppChildren = NULL;
    count = 1;
  }

  inline CPPMLanguageModel::CPPMnode::CPPMnode() {
    vine = 0;
    m_iNumChildSlots = 0;
    m_ppChildren = NULL;
    count = 1;
  }
  
  inline CPPMLanguageModel::CPPMnode::~CPPMnode() {
    //single child = is direct pointer to node, not array...
    if (m_iNumChildSlots != 1)
      delete m_ppChildren;
  }

  inline CLanguageModel::Context CPPMLanguageModel::CreateEmptyContext() {
    CPPMContext *pCont = m_ContextAlloc.Alloc();
    *pCont = *m_pRootContext;

    m_setContexts.insert(pCont);

    return (Context) pCont;
  }

  inline CLanguageModel::Context CPPMLanguageModel::CloneContext(Context Copy) {
    CPPMContext *pCont = m_ContextAlloc.Alloc();
    CPPMContext *pCopy = (CPPMContext *) Copy;
    *pCont = *pCopy;

    m_setContexts.insert(pCont);

    return (Context) pCont;
  }

  inline void CPPMLanguageModel::ReleaseContext(Context release) {

    m_setContexts.erase(m_setContexts.find((CPPMContext *) release));

    m_ContextAlloc.Free((CPPMContext *) release);
  }
}                               // end namespace Dasher

#endif // __LanguageModelling__PPMLanguageModel_h__
