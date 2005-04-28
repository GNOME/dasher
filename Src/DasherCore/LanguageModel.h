// LanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

// Abstract language model class
// See PPMModel for an example implementation

/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModel_h__
#define __LanguageModel_h__

#include <vector>
#include <string>

#include "Alphabet.h"
#include "Context.h"
#include "DasherTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {class CLanguageModel;}
class Dasher::CLanguageModel
{
public:
	CLanguageModel(CAlphabet* Alphabet);

	virtual ~CLanguageModel() {}

	virtual CContext* GetEmptyContext() const =0;
	virtual CContext* CloneContext(const CContext* pContext)=0;
	virtual void ReleaseContext(CContext* pContext)=0;
	
	// update context with a character:
	virtual void EnterSymbol(CContext* pContext, symbol Symbol)=0;
	
	// add character to the language model:
	virtual void LearnSymbol(CContext* pContext, symbol Symbol)=0;
	
	// get the probability distrubution at the given context:
	virtual bool GetProbs(const CContext* pContext, std::vector<unsigned int> &Probs, int norm) const=0;
	
	void EnterText(CContext* Context, std::string TheText);
	void LearnText(CContext* Context, std::string* TheText, bool IsMore);

	bool GetProbs(CContext* Context, std::vector<symbol> &NewSymbols,
		std::vector<unsigned int> &Groups, std::vector<unsigned int> &Probs, int iNorm) const;
	
	// Alphabet pass-through functions for widely needed information
	symbol GetSpaceSymbol() const {return m_Alphabet->GetSpaceSymbol();}
	symbol GetControlSymbol() const {return m_Alphabet->GetControlSymbol();}
	
	bool isRealSymbol( symbol _s ) const;

	int GetColour(int character) const;
	int GetGroupColour(int group) const;
	std::string GetGroupLabel(int group) const;

	void SetUniform( int _uniform ) { m_uniform = _uniform; };

protected:
	int GetNumberModelChars() const {return m_Alphabet->GetNumberSymbols();}
	
	// diagnostic info:
	virtual void dump()=0;
	
private:
	CAlphabet *m_Alphabet;
	int m_uniform;     // Fraction to allocate to uniform dist. (*1000)
};

using namespace Dasher;

/////////////////////////////////////////////////////////////////////////////



#endif /* #ifndef __LanguageModel_h__ */
