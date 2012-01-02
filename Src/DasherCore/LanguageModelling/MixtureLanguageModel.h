// MixtureLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_MixtureLanguageModel_h__
#define __LanguageModelling_MixtureLanguageModel_h__

#include "LanguageModel.h"
#include "PPMLanguageModel.h"
#include "DictLanguageModel.h"

//#include <iostream>
#include <vector>

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {
  class CMixtureLanguageModel;

  /// \ingroup LM
  /// \{
  class CMixtureLanguageModel:public CLanguageModel, protected CSettingsUser {
  public:

    /////////////////////////////////////////////////////////////////////////////

    CMixtureLanguageModel(CSettingsUser *pCreator, const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap)
    : CLanguageModel(pAlph->iEnd-1), CSettingsUser(pCreator) {

      //      std::cout << m_pAlphabet << std::endl;

      NextContext = 0;

      lma = new CPPMLanguageModel(this, m_iNumSyms);
      lmb = new CDictLanguageModel(this, pAlph, pAlphMap);

    };

      virtual ~ CMixtureLanguageModel() {
      delete lma;
      delete lmb;
    };

    /////////////////////////////////////////////////////////////////////////////
    // Context creation/destruction
    ////////////////////////////////////////////////////////////////////////////

    // FIXME - need to work out how to do this

    // Create a context (empty)
    virtual CLanguageModel::Context CreateEmptyContext();
    virtual CLanguageModel::Context CloneContext(CLanguageModel::Context Context);
    virtual void ReleaseContext(CLanguageModel::Context Context);

    /////////////////////////////////////////////////////////////////////////////
    // Context modifiers
    ////////////////////////////////////////////////////////////////////////////

    // Update context with a character - only modifies context
    virtual void EnterSymbol(CLanguageModel::Context context, int Symbol) {
      lma->EnterSymbol(ContextMap.find(context)->second->GetContextA(), Symbol);
      lmb->EnterSymbol(ContextMap.find(context)->second->GetContextB(), Symbol);
    };

    // Add character to the language model at the current context and update the context 
    // - modifies both the context and the LanguageModel
    virtual void LearnSymbol(CLanguageModel::Context context, int Symbol) {
      lma->LearnSymbol(ContextMap[context]->GetContextA(), Symbol);
      lmb->LearnSymbol(ContextMap[context]->GetContextB(), Symbol);
    };

    /////////////////////////////////////////////////////////////////////////////
    // Prediction
    /////////////////////////////////////////////////////////////////////////////

    // Get symbol probability distribution
    virtual void GetProbs(CLanguageModel::Context context, std::vector < unsigned int >&Probs, int iNorm, int iUniform) const {

      int iNumSymbols = GetSize();

        Probs.resize(iNumSymbols);

        std::vector < unsigned int >ProbsA(iNumSymbols);
        std::vector < unsigned int >ProbsB(iNumSymbols);

      int iNormA(iNorm * GetLongParameter(LP_LM_MIXTURE) / 100);
      int iNormB(iNorm - iNormA);
      
      // TODO: Fix uniform here
        lma->GetProbs(ContextMap.find(context)->second->GetContextA(), ProbsA, iNormA, 0);
        lmb->GetProbs(ContextMap.find(context)->second->GetContextB(), ProbsB, iNormB, 0);

      for(int i(1); i < iNumSymbols; i++) {
        Probs[i] = ProbsA[i] + ProbsB[i];
    }};

  private:
    CLanguageModel * lma;
    CLanguageModel *lmb;

    class CMixtureContext {
    public:
      CMixtureContext(CLanguageModel * _lma, CLanguageModel * _lmb):lma(_lma), lmb(_lmb) {
        ca = lma->CreateEmptyContext();
        cb = lmb->CreateEmptyContext();
      };

    CMixtureContext(CLanguageModel * _lma, CLanguageModel * _lmb, CLanguageModel::Context _ca, CLanguageModel::Context _cb):lma(_lma), lmb(_lmb), ca(_ca), cb(_cb) {
      };

      ~CMixtureContext() {
        lma->ReleaseContext(ca);
        lmb->ReleaseContext(cb);
      };

      CLanguageModel::Context GetContextA() {
        return ca;
      }

      CLanguageModel::Context GetContextB() {
        return cb;
      }

    private:
      CLanguageModel * lma;
      CLanguageModel *lmb;

      CLanguageModel::Context ca;
      CLanguageModel::Context cb;
    };

    int NextContext;

    std::map < int, CMixtureContext * >ContextMap;

  };
  /// \}

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CMixtureLanguageModel::CreateEmptyContext() {
    CMixtureContext *pCont = new CMixtureContext(lma, lmb);
    ContextMap[NextContext] = pCont;
    ++NextContext;
    return NextContext - 1;
  }

///////////////////////////////////////////////////////////////////

  inline CLanguageModel::Context CMixtureLanguageModel::CloneContext(CLanguageModel::Context Copy) {
    CMixtureContext *pCopy = ContextMap[Copy];
    CMixtureContext *pCont = new CMixtureContext(lma, lmb, lma->CloneContext(pCopy->GetContextA()), lmb->CloneContext(pCopy->GetContextB()));

    ContextMap[NextContext] = pCont;
    ++NextContext;
    return NextContext - 1;
  }

///////////////////////////////////////////////////////////////////

  inline void CMixtureLanguageModel::ReleaseContext(CLanguageModel::Context release) {
    // m_ContextAlloc.Free( (CMixtureContext*) release );
    delete ContextMap[release];
    ContextMap[release] = NULL;
  }
}

///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

#endif // ndef __LanguageModelling_MixtureLanguageModel_h__
