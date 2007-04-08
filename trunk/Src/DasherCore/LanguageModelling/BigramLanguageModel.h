// BigramLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_BigramLanguageModel_h__
#define __LanguageModelling_BigramLanguageModel_h__

/////////////////////////////////////////////////////////////////////////////
// Bigram chararacter Language Model
/////////////////////////////////////////////////////////////////////////////

#include "LanguageModel.h"
#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"

namespace Dasher {

  class CLanguageModelParams;

  /// \ingroup LM
  /// \{
  class CBigramLanguageModel:public CLanguageModel, private NoClones {
  public:
    CBigramLanguageModel(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, const CSymbolAlphabet & alph);
      virtual ~ CBigramLanguageModel();

    Context CreateEmptyContext();
    void ReleaseContext(Context context);
    Context CloneContext(Context context);

    void EnterSymbol(Context context, int Symbol) const;
    void LearnSymbol(Context context, int Symbol);

    virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int iSumProbs) const;

  private:

      class CContext {
      int m_last;
    };

      CPooledAlloc < CContext > m_ContextAlloc;

  };
  /// \}

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CBigramLanguageModel::CreateEmptyContext() {
    CContext *pCont = m_ContextAlloc.Alloc();
//      *pCont = *m_Rootontext;
    return (Context) pCont;
  }

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CBigramLanguageModel::CloneContext(Context Copy) {
    CContext *pCont = m_ContextAlloc.Alloc();
    CContext *pCopy = (CContext *) Copy;
    *pCont = *pCopy;
    return (Context) pCont;
  }

///////////////////////////////////////////////////////////////////

  inline void CBigramLanguageModel::ReleaseContext(Context release) {
    m_ContextAlloc.Free((CContext *) release);
  }

}                               // end namespace Dasher

#endif // ndef
