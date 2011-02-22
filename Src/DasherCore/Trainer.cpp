
#include "../Common/Common.h"

#include "Trainer.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMPYLanguageModel.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CTrainer::CTrainer(CLanguageModel *pLanguageModel, const CAlphabetMap *pAlphabet)
  : CTrainingHelper(pAlphabet), m_pLanguageModel(pLanguageModel) {
}

void CTrainer::Train(CAlphabetMap::SymbolStream &syms) {
  CLanguageModel::Context sContext = m_pLanguageModel->CreateEmptyContext();

  for(symbol sym; (sym=syms.next(m_pAlphabet))!=-1;) {
      m_pLanguageModel->LearnSymbol(sContext, sym);
  }
  m_pLanguageModel->ReleaseContext(sContext);
}

CMandarinTrainer::CMandarinTrainer(CPPMPYLanguageModel *pLanguageModel, const CAlphabetMap *pAlphabet, const CAlphabetMap *pCHAlphabet, const std::string &strDelim)
: CTrainer(pLanguageModel, pAlphabet), m_pCHAlphabet(pCHAlphabet), m_strDelim(strDelim) {
}

void CMandarinTrainer::Train(CAlphabetMap::SymbolStream &syms) {
  unsigned numberofchar = 0;
  CLanguageModel::Context trainContext = m_pLanguageModel->CreateEmptyContext();
  
  for (string s; (s=syms.peekAhead()).length();) {
    syms.next(m_pAlphabet); //skip over character at which we just peeked (we don't need the symbol#)
    
    if (s == m_strDelim) { //found delimiter, so process next two characters
      symbol Sympy = syms.next(m_pAlphabet);
      if (Sympy==-1) break; //EOF
#ifdef DEBUG
      if (Sympy==0)
        std::cout << "Unknown pinyin character " << syms.peekBack() << std::endl;
#endif
      symbol Symchar = syms.next(m_pCHAlphabet);
      if (Symchar==-1) break; //EOF...ignore final Pinyin?
#ifdef DEBUG
      if (Symchar==0)
        std::cout << "Unknown chinese character " << syms.peekBack() << std::endl;
#endif
      static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->LearnPYSymbol(trainContext, Sympy);
      m_pLanguageModel->LearnSymbol(trainContext, Symchar);
      numberofchar++;    
    } //else, keep looking for delimiter
  }
  m_pLanguageModel->ReleaseContext(trainContext);
}
