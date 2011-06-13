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
#include "Alphabet.h"

namespace Dasher {
  /// \ingroup LM
  /// \{
  class CJapaneseLanguageModel:public CLanguageModel, private NoClones {
  public:
    CJapaneseLanguageModel(CSettingsStore * pSettingsStore, const CAlphInfo *pAlph);

    virtual ~ CJapaneseLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    virtual void EnterSymbol(Context context, int Symbol);
    virtual void LearnSymbol(Context context, int Symbol);

    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int norm) const;

    void dump();

  private:
    CSettingsStore *m_pSettingsStore;

    class CJaPPMnode {
    public:
      CJaPPMnode * find_symbol(int sym) const;
      CJaPPMnode *child;
      CJaPPMnode *next;
      CJaPPMnode *vine;
      unsigned short int count;
      short int symbol;
      CJaPPMnode(int sym);
      CJaPPMnode();
    };

    class CJaPPMContext {
    public:
      CJaPPMContext(CJaPPMContext const &input) {
        head = input.head;
        order = input.order;
        history = input.history;
      } CJaPPMContext(CJaPPMnode * _head = 0, int _order = 0):head(_head), order(_order) {
      };
      ~CJaPPMContext() {
      };
      void dump();
      CJaPPMnode *head;
      int order;
      std::vector < symbol > history;
    };

    CJaPPMnode *AddSymbolToNode(CJaPPMnode * pNode, int sym, int *update);

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
    const void GetSymbols(std::vector < symbol > &Symbols, std::string &Input) const {
      SymbolAlphabet().GetAlphabetPointer()->GetSymbols(Symbols, Input);
    }
    //--End:Kanji Conversion Related
    virtual void AddSymbol(CJaPPMContext & context, int sym);
    void dumpSymbol(int sym);
    void dumpString(char *str, int pos, int len);
    void dumpTrie(CJaPPMnode * t, int d);

    CJaPPMContext *m_pRootContext;
    CJaPPMnode *m_pRoot;

    int m_iMaxOrder;
    double m_dBackOffConstat;

    int NodesAllocated;

    bool bUpdateExclusion;

    mutable CSimplePooledAlloc < CJaPPMnode > m_NodeAlloc;
    CPooledAlloc < CJaPPMContext > m_ContextAlloc;
  };
  /// \}

  ////////////////////////////////////////////////////////////////////////

  inline Dasher::CJapaneseLanguageModel::CJaPPMnode::CJaPPMnode(int sym):symbol(sym) {
    child = next = vine = 0;
    count = 1;
  }

  ////////////////////////////////////////////////////////////////////////

  inline CJapaneseLanguageModel::CJaPPMnode::CJaPPMnode() {
    child = next = vine = 0;
    count = 1;
  }

  ///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CJapaneseLanguageModel::CreateEmptyContext() {
    CJaPPMContext *pCont = m_ContextAlloc.Alloc();
    *pCont = *m_pRootContext;
    return (Context) pCont;
  }

  ///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CJapaneseLanguageModel::CloneContext(Context Copy) {
    CJaPPMContext *pCont = m_ContextAlloc.Alloc();
    CJaPPMContext *pCopy = (CJaPPMContext *) Copy;
    *pCont = *pCopy;
    return (Context) pCont;
  }

  ///////////////////////////////////////////////////////////////////

  inline void CJapaneseLanguageModel::ReleaseContext(Context release) {
    m_ContextAlloc.Free((CJaPPMContext *) release);
  }

  ///////////////////////////////////////////////////////////////////

}                               // end namespace Dasher

#endif // __JapaneseLanguageModell_h__
