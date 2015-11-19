// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WordLanguageModel_h__
#define __WordLanguageModel_h__

#include <cstdlib>

#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"
#include "PPMLanguageModel.h"
#include "../SettingsStore.h"
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

  ///
  /// Language model using words
  ///
  class CWordLanguageModel:public CLanguageModel, protected CSettingsUser {
  public:
    CWordLanguageModel(CSettingsUser *pCreator, const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap);
      virtual ~ CWordLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    virtual void GetProbs(Context Context, std::vector < unsigned int >&Probs, int iNorm, int iUniform) const;

    virtual void EnterSymbol(Context context, int Symbol);
    virtual void LearnSymbol(Context context, int Symbol);

  private:
    
      class CWordnode {
    public:
      CWordnode * find_symbol(int sym)const;
      CWordnode *child;
      CWordnode *next;
      CWordnode *vine;
      unsigned int count;
      int sbl;

        CWordnode(int sym);
        CWordnode();

      void RecursiveDump(std::ofstream & file);
    };



    class CWordContext {
    public:
      CWordContext(CWordContext const &input) {
        head = input.head;
        word_head = input.word_head;
        current_word = input.current_word;
        order = input.order;
        word_order = input.word_order;
      } 
      
      CWordContext(CWordnode * _head = 0, int _order = 0): head(_head), order(_order), word_head(_head), word_order(0)
	{};                        // FIXME - doesn't work if we're trying to create a non-empty context
      ~CWordContext() {
      };
      void dump();
      CWordnode *head;
      int order;

      std::string current_word;
      CWordnode *word_head;
      int word_order;

      std::vector < unsigned int >oSpellingProbs;
      int m_iSpellingNorm;
      double m_dSpellingFactor;

      /// Pointer to the letter based model - note that we don't
      /// actually own this, so don't delete it

      CPPMLanguageModel *m_pSpellingModel;

      ///
      /// The corresponding context in the spelling model

      CPPMLanguageModel::Context oSpellingContext;

    };

    CWordnode *AddSymbolToNode(CWordnode * pNode, symbol sym, int *update, bool bLearn);

    void AddSymbol(CWordContext & context, symbol sym, bool bLearn);

    void CollapseContext(CWordContext & context, bool bLearn);

    int lookup_word(const std::string & w);
    int lookup_word_const(const std::string & w) const;

    const int m_iSpaceSymbol;
    
    CWordContext *m_rootcontext;
    CWordnode *m_pRoot;

    std::map < std::string, int >dict;  // Dictionary
    int nextid;
    int iWordStart;

    int wordidx;

    int NodesAllocated;

    int max_order;

    CPPMLanguageModel *pSpellingModel;  // Use this to predict the spellings of new words


    mutable CSimplePooledAlloc < CWordnode > m_NodeAlloc;
    CPooledAlloc < CWordContext > m_ContextAlloc;
  };
  /// \}

////////////////////////////////////////////////////////////////////////
// Inline functions 
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

  inline Dasher::CWordLanguageModel::CWordnode::CWordnode(symbol sym):sbl(sym) {
    child = next = vine = 0;
    count = 1;
  }

////////////////////////////////////////////////////////////////////////

  inline CWordLanguageModel::CWordnode::CWordnode() {
    child = next = vine = 0;
    count = 1;
  }

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CWordLanguageModel::CreateEmptyContext() {
    return CloneContext((Context)m_rootcontext);
  }

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CWordLanguageModel::CloneContext(Context Copy) {
    CWordContext *pCont = m_ContextAlloc.Alloc();
    CWordContext *pCopy = (CWordContext *) Copy;
    *pCont = *pCopy;

    // Create a clone of the spelling context

    pCont->oSpellingContext = pCont->m_pSpellingModel->CloneContext(pCopy->oSpellingContext);

    return (Context) pCont;
  }

///////////////////////////////////////////////////////////////////

  inline void CWordLanguageModel::ReleaseContext(Context release) {
    // Urgh!
    CWordContext *pCont(reinterpret_cast<CWordContext *>(release));
    
    pCont->m_pSpellingModel->ReleaseContext(pCont->oSpellingContext);

    m_ContextAlloc.Free(pCont);
  }

///////////////////////////////////////////////////////////////////

}                               // end namespace Dasher

#endif /* #ifndef __WordLanguageModel_H__ */
