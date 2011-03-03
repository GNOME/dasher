
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

CTrainer::CTrainer(CLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pAlphabet)
  : CTrainingHelper(pAlphabet), m_pLanguageModel(pLanguageModel), m_pInfo(pInfo) {
    vector<symbol> syms;
    pAlphabet->GetSymbols(syms,pInfo->GetContextEscapeChar());
    if (syms.size()==1)
      m_iCtxEsc = syms[0];
    else {      
#ifdef DEBUG
      std::cout << "Warning: escape sequence " << pInfo->GetContextEscapeChar() << " must be a single unicode character; no context-switch commands will be executed." << std::endl;
#endif
      m_iCtxEsc = -1;
    }
}

void CTrainer::Train(CAlphabetMap::SymbolStream &syms) {
  CLanguageModel::Context sContext = m_pLanguageModel->CreateEmptyContext();

  for(symbol sym; (sym=syms.next(m_pAlphabet))!=-1;) {
    //check for context-switch commands.
    // (Will only ever be triggered if m_strEscape is a single unicode character, hence warning in c'tor)
    if (sym == m_iCtxEsc) {
      //that was a quick check, to avoid calling slow peekBack() in most cases. Now make sure...
      if (sym!=0 || syms.peekBack()==m_pInfo->GetContextEscapeChar()) {
        //Yes, found escape character....
        if (readEscape(sContext, syms)) continue;
        //returns false, if there was a _double_ escape character - i.e. an actual
        // occurrence of the character is wanted. In which case, fall through
        // (sym is already set to the correct AlphabetMap symbol# for the first escape character)
      }
      else DASHER_ASSERT (sym==0); //symbol, and escape char, both out-of-alphabet. Fall through...
      // (Or, TODO, should the Trainer be responsible for skipping unknown symbols, rather than the LM?)
    }
    //either a non-escapecharacter, or a double escapecharacter, was read
    m_pLanguageModel->LearnSymbol(sContext, sym);
  }
  m_pLanguageModel->ReleaseContext(sContext);
}

bool CTrainer::readEscape(CLanguageModel::Context &sContext, CAlphabetMap::SymbolStream &syms) {
  string delim=syms.peekAhead();
  //A double escape character means an actual occurrence of the character is wanted...
  if (delim == m_pInfo->GetContextEscapeChar()) return false;
  
  //ok, so switch context. release the old, start a new...
  m_pLanguageModel->ReleaseContext(sContext);
  sContext = m_pLanguageModel->CreateEmptyContext();
  //enter the alphabet default context first...
  vector<symbol> defCtx;
  m_pAlphabet->GetSymbols(defCtx, m_pInfo->GetDefaultContext());
  for (vector<symbol>::iterator it=defCtx.begin(); it!=defCtx.end(); it++) m_pLanguageModel->EnterSymbol(sContext, *it);
  //and read the first delimiter; everything until the second occurrence of this, is _context_ only.
  syms.next(m_pAlphabet); //skip it
  for (symbol sym; (sym=syms.next(m_pAlphabet))!=-1; ) {
    if (syms.peekBack()==delim) break;
    m_pLanguageModel->EnterSymbol(sContext, sym);
  }
  return true;  
}

CMandarinTrainer::CMandarinTrainer(CPPMPYLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pPYAlphabet, const CAlphabetMap *pCHAlphabet, const std::string &strDelim)
: CTrainer(pLanguageModel, pInfo, pCHAlphabet), m_pPYAlphabet(pPYAlphabet), m_strDelim(strDelim) {
}

void CMandarinTrainer::Train(CAlphabetMap::SymbolStream &syms) {
  unsigned numberofchar = 0;
  CLanguageModel::Context trainContext = m_pLanguageModel->CreateEmptyContext();
  
  for (string s; (s=syms.peekAhead()).length();) {
    syms.next(m_pPYAlphabet); //skip over character at which we just peeked (we don't need the symbol#)
    
    if (s == m_strDelim) { //found delimiter, so process next two characters
      symbol Sympy = syms.next(m_pPYAlphabet);
      if (Sympy==-1) break; //EOF
#ifdef DEBUG
      if (Sympy==0)
        std::cout << "Unknown pinyin character " << syms.peekBack() << std::endl;
#endif
      symbol Symchar = syms.next(m_pAlphabet);
      if (Symchar==-1) break; //EOF...ignore final Pinyin?
#ifdef DEBUG
      if (Symchar==0)
        std::cout << "Unknown chinese character " << syms.peekBack() << std::endl;
#endif
      static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->LearnPYSymbol(trainContext, Sympy);
      m_pLanguageModel->LearnSymbol(trainContext, Symchar);
      numberofchar++;    
    } else if (s == m_pInfo->GetContextEscapeChar()) {
      //we've already skipped over the (first) escape char
      readEscape(trainContext, syms);
      //a double escape-char will be ignored: it means "don't switch context,
      // here's an (escape-char)", but we are only looking for m_strDelim.
    } //else, keep looking for delimiter
  }
  m_pLanguageModel->ReleaseContext(trainContext);
}
