// LanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_LanguageModel_h__
#define __LanguageModelling_LanguageModel_h__

#include <vector>
#include <string>

#include "../Alphabet.h"
#include "../DasherTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {class CLanguageModel;}

class Dasher::CLanguageModel
{
public:

	CLanguageModel(const CAlphabet* pcAlphabet);
	virtual ~CLanguageModel() {}

	// Handle for a language model context
	// NULL is reserved
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

	// update context with a character:
	virtual void EnterSymbol(Context context, symbol Symbol) const =0;
	
	// add character to the language model:
	virtual void LearnSymbol(Context context, symbol Symbol) =0;
	
	// get the probability distrubution at the given context:
	virtual bool GetProbs(Context Context, std::vector<unsigned int> &Probs, int norm) const =0;
	
	void EnterText(Context Context, std::string TheText);
	void LearnText(Context Context, std::string* TheText, bool IsMore);

	bool GetProbs(Context Context, std::vector<symbol> &NewSymbols,
		std::vector<unsigned int> &Groups, std::vector<unsigned int> &Probs, int iNorm) const;
	
	// Alphabet pass-through functions for widely needed information
	symbol GetSpaceSymbol() const {return m_pcAlphabet->GetSpaceSymbol();}
	symbol GetControlSymbol() const {return m_pcAlphabet->GetControlSymbol();}
	
	bool isRealSymbol( symbol _s ) const;

	int GetColour(int character) const;
	int GetGroupColour(int group) const;
	std::string GetGroupLabel(int group) const;

	void SetUniform( int _uniform ) { m_uniform = _uniform; };

protected:
	int GetNumberModelChars() const {return m_pcAlphabet->GetNumberSymbols();}
	
	// diagnostic info:
	virtual void dump() {}
	
private:
	const CAlphabet* m_pcAlphabet;
	int m_uniform;     // Fraction to allocate to uniform dist. (*1000)
};


/////////////////////////////////////////////////////////////////////////////



#endif /* #ifndef __LanguageModel_h__ */
