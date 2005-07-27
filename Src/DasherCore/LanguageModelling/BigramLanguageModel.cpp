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

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

CBigramLanguageModel::CBigramLanguageModel(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CSymbolAlphabet &SymbolAlphabet)
:CLanguageModel(pEventHandler, pSettingsStore, SymbolAlphabet), m_ContextAlloc(1024) {

}

///////////////////////////////////////////////////////////////////

void CBigramLanguageModel::EnterSymbol(Context context, int Symbol) const {
}

///////////////////////////////////////////////////////////////////

void CBigramLanguageModel::LearnSymbol(Context context, int Symbol) {
}

///////////////////////////////////////////////////////////////////

void CBigramLanguageModel::GetProbs(Context context, std::vector <unsigned int >&Probs, int norm) const {
}

///////////////////////////////////////////////////////////////////
