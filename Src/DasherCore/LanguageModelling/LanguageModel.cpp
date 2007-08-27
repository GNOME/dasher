// LanguageModel.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "LanguageModel.h"
#include "../Alphabet/Alphabet.h"

/////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////

CLanguageModel::CLanguageModel(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CSymbolAlphabet &Alphabet)
  :CDasherComponent(pEventHandler, pSettingsStore), m_Alphabet(Alphabet) {
}

void CLanguageModel::HandleEvent(Dasher::CEvent *pEvent) {
}

///////////////////////////////////////////////////////////////////
