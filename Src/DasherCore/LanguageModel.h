// LanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

// Abstract language model class
// See PPMModel for an example implementation

// Contexts are indentified by a unique unsigned integer

#ifndef __LanguageModel_h__
#define __LanguageModel_h__

#include "../Common/MSVC_Unannoy.h"
#include <vector>
#include <string>

#include "Alphabet.h"
#include "Context.h"
#include "DasherTypes.h"

namespace Dasher {class CLanguageModel;}
class Dasher::CLanguageModel
{
public:
	CLanguageModel(CAlphabet* Alphabet, int Normalization);

	// Interface for the Dasher code
	// --------------------------------------------------------------------------
	class CNodeContext {
	public:
		CNodeContext() {};
		virtual ~CNodeContext() {};
	};
	
	// return the model's normalization - what the probabilities sum to
	const int normalization() const { return m_iNorm;}
	
	CNodeContext* GetRootNodeContext();
	CNodeContext* CloneNodeContext(CNodeContext* NodeContext);
	void ReleaseNodeContext(CNodeContext* NodeContext);
	void EnterNodeSymbol(CNodeContext* NodeContext, symbol Symbol);
	void LearnNodeSymbol(CNodeContext* NodeContext, symbol Symbol);
	void EnterText(CNodeContext* NodeContext, std::string TheText);
	void LearnText(CNodeContext* NodeContext, std::string* TheText, bool IsMore);
	bool GetNodeProbs(CNodeContext* Context, std::vector<symbol> &NewSymbols,
		std::vector<unsigned int> &Groups, std::vector<unsigned int> &Probs, double AddProb);
	
	// Alphabet pass-through functions for widely needed information
	symbol GetSpaceSymbol() {return m_Alphabet->GetSpaceSymbol();}
	symbol GetControlSymbol() {return m_Alphabet->GetControlSymbol();}
	
	int GetColour(int character);

	void NewControlTree(ControlTree *tree) { controltree=tree; }

	ControlTree* GetControlTree() { return controltree; }

protected:
	int GetNumberModelChars() {return m_Alphabet->GetNumberSymbols();}
	
	// Generic language model functions to be implemented 
	// --------------------------------------------------------------------------
	typedef unsigned int modelchar;
	
	// return the id for the root context:
	virtual CContext* GetRootContext()=0;
	// clone a context and return the new id:
	virtual CContext* CloneContext(CContext*)=0;
	// delete a context:
	virtual void ReleaseContext(CContext*)=0;
	// diagnostic info:
	virtual void dump()=0;
	// add character to the language model:
	virtual void LearnSymbol(CContext* Context, modelchar Symbol)=0;
	// update context with a character:
	virtual void EnterSymbol(CContext* context, modelchar Symbol)=0;
	// get the probability distrubution at the given context:
	virtual bool GetProbs(CContext* Context, std::vector<unsigned int> &Probs, double AddProb)=0;
	
private:
	CAlphabet *m_Alphabet;
	int m_iModelChars; // number of charater in the model 1...ModelChars
	int m_iNorm;       // normalization of probabilities
	ControlTree *controltree;
};

using namespace Dasher;

///////////////////////////////////////////////////////////////////

inline CLanguageModel::CNodeContext* CLanguageModel::GetRootNodeContext()
{
	return (CNodeContext*) GetRootContext();
}

///////////////////////////////////////////////////////////////////

inline CLanguageModel::CNodeContext* CLanguageModel::CloneNodeContext(CNodeContext* NodeContext)
{
	return (CNodeContext*) CloneContext((CContext*) NodeContext);
}

///////////////////////////////////////////////////////////////////

inline void CLanguageModel::ReleaseNodeContext(CNodeContext* NodeContext)
{
	ReleaseContext((CContext*) NodeContext);
}

///////////////////////////////////////////////////////////////////

inline void CLanguageModel::EnterNodeSymbol(CNodeContext* NodeContext, symbol Symbol)
{
	EnterSymbol((CContext*) NodeContext, (modelchar) Symbol);
}

///////////////////////////////////////////////////////////////////

inline void CLanguageModel::LearnNodeSymbol(CNodeContext* NodeContext, symbol Symbol)
{
	LearnSymbol((CContext*) NodeContext, (modelchar) Symbol);
}



#endif /* #ifndef __LanguageModel_h__ */
