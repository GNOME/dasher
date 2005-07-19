// JapaneseLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2005 David Ward
//                    2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __JapaneseLanguageModel_h__
#define __JapaneseLanguageModel_h__

#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"

#include "LanguageModel.h"

namespace Dasher {

  class CJapaneseLanguageModel:public CLanguageModel, private NoClones {
  public:
    CJapaneseLanguageModel(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, const CSymbolAlphabet & alph, CLanguageModelParams * _params);

    virtual ~ CJapaneseLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    virtual void EnterSymbol(Context context, int Symbol);
    virtual void LearnSymbol(Context context, int Symbol);

    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm) const;

    void dump();

    virtual int GetMemory() {
      return NodesAllocated;
    }

  private:

    class CPPMnode {
    public:
      CPPMnode * find_symbol(int sym) const;
      CPPMnode *child;
      CPPMnode *next;
      CPPMnode *vine;
      unsigned short int count;
      short int symbol;
      CPPMnode(int sym);
      CPPMnode();
    };

    class CPPMContext {
    public:
      CPPMContext(CPPMContext const &input) {
        head = input.head;
        order = input.order;
        history = input.history;
      } CPPMContext(CPPMnode * _head = 0, int _order = 0):head(_head), order(_order) {
      };
      ~CPPMContext() {
      };
      void dump();
      CPPMnode *head;
      int order;
      std::vector < symbol > history;
    };

    CPPMnode *AddSymbolToNode(CPPMnode * pNode, int sym, int *update);

    //--Start:Kanji Conversion Related Addition
    symbol GetStartConversionSymbol() const {
      return SymbolAlphabet().GetAlphabetPointer()->GetStartConversionSymbol();
    } //Added 23 June 2005
    symbol GetEndConversionSymbol() const {
      return SymbolAlphabet().GetAlphabetPointer()->GetEndConversionSymbol();
    } //Added 7 July 2005
    const std::string & GetText(symbol i) const {
      return SymbolAlphabet().GetAlphabetPointer()->GetText(i);
    } // return string for i'th symbol
    const std::string & GetDisplayText(symbol i) const {
      return SymbolAlphabet().GetAlphabetPointer()->GetDisplayText(i);
    } // return string for i'th symbol 
    const void GetSymbols(std::vector < symbol > *Symbols, std::string * Input, bool IsMore) const {
      SymbolAlphabet().GetAlphabetPointer()->GetSymbols(Symbols, Input, IsMore);
    }
    //--End:Kanji Conversion Related
    virtual void AddSymbol(CPPMContext & context, int sym);
    void dumpSymbol(int sym);
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
  };

  ////////////////////////////////////////////////////////////////////////

  inline Dasher::CJapaneseLanguageModel::CPPMnode::CPPMnode(int sym):symbol(sym) {
    child = next = vine = 0;
    count = 1;
  }

  ////////////////////////////////////////////////////////////////////////

  inline CJapaneseLanguageModel::CPPMnode::CPPMnode() {
    child = next = vine = 0;
    count = 1;
  }

  ///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CJapaneseLanguageModel::CreateEmptyContext() {
    CPPMContext *pCont = m_ContextAlloc.Alloc();
    *pCont = *m_pRootContext;
    return (Context) pCont;
  }

  ///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CJapaneseLanguageModel::CloneContext(Context Copy) {
    CPPMContext *pCont = m_ContextAlloc.Alloc();
    CPPMContext *pCopy = (CPPMContext *) Copy;
    *pCont = *pCopy;
    return (Context) pCont;
  }

  ///////////////////////////////////////////////////////////////////

  inline void CJapaneseLanguageModel::ReleaseContext(Context release) {
    m_ContextAlloc.Free((CPPMContext *) release);
  }

  ///////////////////////////////////////////////////////////////////

}                               // end namespace Dasher

#endif // __JapaneseLanguageModell_h__
