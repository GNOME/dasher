// LanguageModel.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "LanguageModel.h"

/////////////////////////////////////////////////////////////////////////////

using namespace Dasher;
using namespace std;

///////////////////////////////////////////////////////////////////

CLanguageModel::CLanguageModel(const CAlphabet* pcAlphabet)
	: m_pcAlphabet(pcAlphabet)
{
	m_uniform = 50;
}


///////////////////////////////////////////////////////////////////

void CLanguageModel::EnterText(Context context, string TheText)
{
	vector<symbol> Symbols;
	m_pcAlphabet->GetSymbols(&Symbols, &TheText, false);
	for (unsigned int i=0; i<Symbols.size(); i++)
		EnterSymbol(context, Symbols[i]);
}

///////////////////////////////////////////////////////////////////

void CLanguageModel::LearnText(Context context, string* TheText, bool IsMore)
{
	vector<symbol> Symbols;
	
	m_pcAlphabet->GetSymbols(&Symbols, TheText, IsMore);
	
	for (unsigned int i=0; i<Symbols.size(); i++)
		LearnSymbol( context,  Symbols[i]);
}

///////////////////////////////////////////////////////////////////

bool CLanguageModel::GetProbs(Context context, vector<symbol> &NewSymbols,
		vector<unsigned int> &Groups, vector<unsigned int> &Probs, int iNorm) const
{
	// make sure it is a valid context
	if (context) {
		int s = m_pcAlphabet->GetNumberSymbols();

		// Subtract some number of nodes to actually get the number of real symbols

		int real_s;

//  		if( GetControlSymbol() == -1 )
//  		  real_s = s - 1;
//  		else
//  		  real_s = s - 2;

		real_s = s - 1;
		
		// Panic if we have no real symbols

		if( real_s <= 0 )
		  return false;

		//		int control( norm * 0.1 ); // FIXME - control node fration is decided here

		//		if( GetControlSymbol() != -1 )
		//  norm -= control;

		int uniform_add = ((iNorm / 1000 ) / real_s ) * m_uniform;
		int nonuniform_norm = iNorm - real_s * uniform_add;

		NewSymbols.resize(s);
		Groups.resize(s);
		for (int i=0;i<s;i++) {
			NewSymbols[i]=i; // This will be replaced by something that works out valid nodes for this context
			Groups[i]=m_pcAlphabet->get_group(i);
		}
		GetProbs(context,Probs,nonuniform_norm);

		for( int k=0; k < Probs.size(); ++k )
		  if( isRealSymbol(NewSymbols[k] ) )
		    Probs[k] += uniform_add;

		    int tot(0);
		    
		    for (int l=0;l<s;l++) {
		      tot += Probs[l];
		    }
		    
		    //		    std::cout << tot << " " << nonuniform_norm << " " << norm << " " << uniform_add << " " << real_s << std::endl;
		    //		if( GetControlSymbol() != -1 )
		    //	  Probs[ GetControlSymbol() ] += control;

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////

int CLanguageModel::GetColour(int character) const
{
  return m_pcAlphabet->GetColour(character);
}

///////////////////////////////////////////////////////////////////

int CLanguageModel::GetGroupColour(int group) const
{
	return m_pcAlphabet->GetGroupColour(group);
}

///////////////////////////////////////////////////////////////////

std::string CLanguageModel::GetGroupLabel(int group) const
{
    return m_pcAlphabet->GetGroupLabel(group);
}

///////////////////////////////////////////////////////////////////

bool CLanguageModel::isRealSymbol( symbol _s ) const
{
  if( _s  == 0 )
    return false;
  else if( _s == GetControlSymbol() )
    return true;
  else
    return true;
}

///////////////////////////////////////////////////////////////////
