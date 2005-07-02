// LanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_LanguageModel_h__
#define __LanguageModelling_LanguageModel_h__

#include "LanguageModelParams.h" 

#include "SymbolAlphabet.h"
#include "../DasherComponent.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {class CLanguageModel;}

class Dasher::CLanguageModel : public Dasher::CDasherComponent
{
public:
	
	/////////////////////////////////////////////////////////////////////////////

	CLanguageModel( Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CSymbolAlphabet& Alphabet, CLanguageModelParams* pParams);

	virtual ~CLanguageModel() {}

	virtual	void HandleEvent( Dasher::CEvent *pEvent );	

	// Handle for a language model context
	// 0 is reserved
	typedef size_t Context;

	/////////////////////////////////////////////////////////////////////////////
	// Context creation/destruction
	////////////////////////////////////////////////////////////////////////////

	// Create a context (empty)
	virtual Context CreateEmptyContext() =0;
	virtual Context CloneContext(Context Context) =0;
	virtual void ReleaseContext(Context Context) =0;

	/////////////////////////////////////////////////////////////////////////////
	// Context modifiers
	////////////////////////////////////////////////////////////////////////////

	// Update context with a character - only modifies context
	virtual void EnterSymbol(Context context, int Symbol)=0;
	
	// Add character to the language model at the current context and update the context 
	// - modifies both the context and the LanguageModel
	virtual void LearnSymbol(Context context, int Symbol) =0;
	
	/////////////////////////////////////////////////////////////////////////////
	// Prediction
	/////////////////////////////////////////////////////////////////////////////

	// Get symbol probability distribution
	virtual void GetProbs(Context Context, std::vector<unsigned int> &Probs, int iNorm) const =0;

	// Get some measure of the memory usage for diagnostic
	// purposes. No need to implement this if you're not comparing
	// language models. The exact meaning of the result will
	// depend on the implementation (for example, could be the
	// number of nodes in a trie, or the physical memory usage).

	virtual int GetMemory() =0;

protected:
	int GetSize() const {return m_Alphabet.GetSize();}

	const CSymbolAlphabet& SymbolAlphabet() const
	{
		return m_Alphabet;
	}

	// const?
	CLanguageModelParams* LanguageModelParams() const
	{
		return m_pParams;
	}

	
	
 private:
	
	const CSymbolAlphabet m_Alphabet;

	// perhaps this should be const - or should we just take a copy?
	CLanguageModelParams* m_pParams;

};


/////////////////////////////////////////////////////////////////////////////

#endif // ndef __LanguageModelling_LanguageModel_h__
