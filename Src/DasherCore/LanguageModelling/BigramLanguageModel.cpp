// BigramLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"
#include "BigramLanguageModel.h"

using namespace Dasher;
///////////////////////////////////////////////////////////////////

void CBigramLanguageModel::EnterSymbol(Context context, int Symbol) const
{
}

void CBigramLanguageModel::LearnSymbol(Context context, int Symbol)
{
}

bool CBigramLanguageModel::GetProbs(Context context, std::vector<unsigned int> &Probs, int norm) const
{
	return true;
}

///////////////////////////////////////////////////////////////////

CBigramLanguageModel blm(NULL);