// LanguageModel.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "LanguageModel.h"

using namespace Dasher;
using namespace std;

// I have removed the following as it doesn't seem to compile in gcc:

// using CLanguageModel::CNodeContext;

///////////////////////////////////////////////////////////////////

CLanguageModel::CLanguageModel(CAlphabet* Alphabet, int Normalization)
	: m_Alphabet(Alphabet), m_iNorm(Normalization)
{
	m_iModelChars = m_Alphabet->GetNumberSymbols();
}


///////////////////////////////////////////////////////////////////

void CLanguageModel::EnterText(CNodeContext* NodeContext, string TheText)
{
	vector<symbol> Symbols;
	m_Alphabet->GetSymbols(&Symbols, &TheText, false);
	for (unsigned int i=0; i<Symbols.size(); i++)
		EnterSymbol((CContext*) NodeContext, (modelchar) Symbols[i]);
}

///////////////////////////////////////////////////////////////////

void CLanguageModel::LearnText(CNodeContext* NodeContext, string* TheText, bool IsMore)
{
	vector<symbol> Symbols;
	
	m_Alphabet->GetSymbols(&Symbols, TheText, IsMore);
	
	for (unsigned int i=0; i<Symbols.size(); i++)
		LearnSymbol((CContext*) NodeContext, (modelchar) Symbols[i]);
}

///////////////////////////////////////////////////////////////////

bool CLanguageModel::GetNodeProbs(CNodeContext* Context, vector<symbol> &NewSymbols,
		vector<unsigned int> &Groups, vector<unsigned int> &Probs, double AddProb)
{
	// make sure it is a valid context
	if (Context) {
		int s = m_Alphabet->GetNumberSymbols();
		NewSymbols.resize(s);
		Groups.resize(s);
		for (int i=0;i<s;i++) {
			NewSymbols[i]=i; // This will be replaced by something that works out valid nodes for this context
			Groups[i]=m_Alphabet->get_group(i);
		}
		GetProbs((CContext*) Context,Probs,AddProb);
		return true;
	}
	return false;
}

int CLanguageModel::GetColour(int character)
{
  std::string colour=m_Alphabet->GetColour(character);
  if (colour!="") {
    return atoi(colour.c_str());
  } else {
    return 0;
  }
}
