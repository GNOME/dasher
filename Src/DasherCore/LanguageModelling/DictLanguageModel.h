// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DictLanguageModel_h__
#define __DictLanguageModel_h__

#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"
#include "PPMLanguageModel.h"
#include "../Alphabet/AlphInfo.h"
#include "../Alphabet/AlphabetMap.h"
#include <vector>
#include <map>
#include <string>
#include <stdio.h>

//static char dumpTrieStr[40000];
//const int maxcont =200;

namespace Dasher {
  /// \ingroup LM
  /// \{
  class CDictLanguageModel:public CLanguageModel, protected CSettingsUser {
  public:
    CDictLanguageModel(CSettingsUser *pCreator, const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap);
    virtual ~CDictLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    virtual void GetProbs(Context Context, std::vector < unsigned int >&Probs, int iNorm, int iUniform) const;

    virtual void EnterSymbol(Context context, int Symbol);
    virtual void LearnSymbol(Context context, int Symbol) {
      EnterSymbol(context, Symbol);
    };                          // Never learn in this model

  private:

    void MyLearnSymbol(Context context, int Symbol);

    class CDictnode {
    public:
      CDictnode * find_symbol(int sym) const;
      CDictnode *child;
      CDictnode *next;
      CDictnode *vine;
      unsigned short int count;
      int sbl;

        CDictnode(int sym);
        CDictnode();
    };

    class CDictContext {
    public:
      CDictContext(CDictContext const &input) {
        head = input.head;
        word_head = input.word_head;
        current_word = input.current_word;
        order = input.order;
        word_order = input.word_order;
    } CDictContext(CDictnode * _head = 0, int _order = 0):head(_head), order(_order), word_head(_head), word_order(0) {
      };                        // FIXME - doesn't work if we're trying to create a non-empty context
      ~CDictContext() {
      };
      void dump();
      CDictnode *head;
      int order;

      std::string current_word;
      CDictnode *word_head;
      int word_order;

    };
    
    const CAlphabetMap *m_pAlphMap;
    const int m_iSpaceSymbol;

    CDictnode *AddSymbolToNode(CDictnode * pNode, symbol sym, int *update);

    void AddSymbol(CDictContext & context, symbol sym);

    void CollapseContext(CDictContext & context) const;

    int lookup_word(const std::string & w);
    int lookup_word_const(const std::string & w) const;

    CDictContext *m_rootcontext;
    CDictnode *m_pRoot;

    std::map < std::string, int >dict;  // Dictionary
    int nextid;

    int NodesAllocated;

    int max_order;

    mutable CSimplePooledAlloc < CDictnode > m_NodeAlloc;
    CPooledAlloc < CDictContext > m_ContextAlloc;
  };
  /// \}

////////////////////////////////////////////////////////////////////////
// Inline functions 
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

  inline Dasher::CDictLanguageModel::CDictnode::CDictnode(symbol sym):sbl(sym) {
    child = next = vine = 0;
    count = 1;
  }

////////////////////////////////////////////////////////////////////////

  inline CDictLanguageModel::CDictnode::CDictnode() {
    child = next = vine = 0;
    count = 1;
  }

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CDictLanguageModel::CreateEmptyContext() {
    CDictContext *pCont = m_ContextAlloc.Alloc();
    *pCont = *m_rootcontext;
    return (Context) pCont;
  }

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CDictLanguageModel::CloneContext(Context Copy) {
    CDictContext *pCont = m_ContextAlloc.Alloc();
    CDictContext *pCopy = (CDictContext *) Copy;
    *pCont = *pCopy;
    return (Context) pCont;
  }

///////////////////////////////////////////////////////////////////

  inline void CDictLanguageModel::ReleaseContext(Context release) {
    m_ContextAlloc.Free((CDictContext *) release);
  }

///////////////////////////////////////////////////////////////////

}                               // end namespace Dasher

#endif /* #ifndef __DictLanguageModel_H__ */
