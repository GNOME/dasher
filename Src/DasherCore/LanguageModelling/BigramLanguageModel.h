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

namespace Dasher
{

class CBigramLanguageModel : public CLanguageModel, private NoClones
{
public:
	CBigramLanguageModel(const CAlphabet* pcAlphabet);
	virtual ~CBigramLanguageModel();

	Context CreateEmptyContext();
	void ReleaseContext(Context context);
	Context CloneContext(Context context);
	
	void EnterSymbol(Context context, int Symbol);
	void LearnSymbol(Context context, int Symbol);
	
	//inline bool GetProbs(CContext*,std::vector<symbol> &newchars,std::vector<unsigned int> &groups,std::vector<unsigned int> &probs,double addprob);
	virtual bool GetProbs(Context context, std::vector<unsigned int> &Probs, int norm) const;

private:

	class CContext
	{

	};


	CPooledAlloc<CContext> m_ContextAlloc;

};

////////////////////////////////////////////////////////////////////////
// Inline functions 
////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CBigramLanguageModel::CreateEmptyContext()
{
	CContext* pCont = m_ContextAlloc.Alloc();
//	*pCont = *m_Rootontext;
	return (Context)pCont;
}

///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CBigramLanguageModel::CloneContext(Context Copy)
{
	CContext* pCont = m_ContextAlloc.Alloc();
	CContext* pCopy = (CContext*)Copy;
	*pCont = *pCopy;
	return (Context)pCont;	
}

///////////////////////////////////////////////////////////////////

inline void CBigramLanguageModel::ReleaseContext(Context release)
{
	m_ContextAlloc.Free( (CContext*) release );
}


} // end namespace Dasher

#endif // ndef 
