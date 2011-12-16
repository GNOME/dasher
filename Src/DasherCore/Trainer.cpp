
#include "../Common/Common.h"

#include "Trainer.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include <vector>
#include <cstring>
#include <sstream>
#include <string>

#include <iostream>

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CTrainer::CTrainer(CMessageDisplay *pMsgs, CLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pAlphabet)
  : AbstractParser(pMsgs), m_pAlphabet(pAlphabet), m_pLanguageModel(pLanguageModel), m_pInfo(pInfo), m_pProg(NULL) {
    vector<symbol> syms;
    pAlphabet->GetSymbols(syms,pInfo->GetContextEscapeChar());
    if (syms.size()==1)
      m_iCtxEsc = syms[0];
    else {      
      //no context switch commands will be executed!
      pMsgs->FormatMessageWithString(_("Warning: faulty alphabet definition, escape sequence %s must be a single unicode character. This may worsen Dasher's text prediction."),
                                     pInfo->GetContextEscapeChar().c_str());
      m_iCtxEsc = -1;
    }
}

CTrainer::~CTrainer() {
}

void CTrainer::Train(CAlphabetMap::SymbolStream &syms) {
  CLanguageModel::Context sContext = m_pLanguageModel->CreateEmptyContext();

  for(symbol sym; (sym=syms.next(m_pAlphabet))!=-1;) {
    //check for context-switch commands.
    // (Will only ever be triggered if m_strEscape is a single unicode character, hence warning in c'tor)
    if (readEscape(sContext, sym, syms)) continue;
    //either a non-escapecharacter, or a double escapecharacter, was read;
    //either way, sym identifies the symbol.
    m_pLanguageModel->LearnSymbol(sContext, sym);
  }
  m_pLanguageModel->ReleaseContext(sContext);
}

bool CTrainer::readEscape(CLanguageModel::Context &sContext, symbol sym, CAlphabetMap::SymbolStream &syms) {
  if (sym != m_iCtxEsc) return false;
  
  //that was a quick check, to avoid calling slow peekBack() in most cases. Now make sure...
  if (sym==0 && syms.peekBack()!=m_pInfo->GetContextEscapeChar()) return false;
  
  //Yes, found escape character....
  
  string delim=syms.peekAhead(); syms.next(m_pAlphabet); //peekAhead doesn't read

  //A double escape character means an actual occurrence of the character is wanted...
  if (delim == m_pInfo->GetContextEscapeChar()) {
    return false;
  }
  //ok, so switch context. release the old, start a new...
  m_pLanguageModel->ReleaseContext(sContext);
  sContext = m_pLanguageModel->CreateEmptyContext();
  //enter the alphabet default context first...
  vector<symbol> defCtx;
  m_pAlphabet->GetSymbols(defCtx, m_pInfo->GetDefaultContext());
  for (vector<symbol>::iterator it=defCtx.begin(); it!=defCtx.end(); it++) m_pLanguageModel->EnterSymbol(sContext, *it);
  //and read the first delimiter; everything until the second occurrence of this, is _context_ only.
  for (symbol sym; (sym=syms.next(m_pAlphabet))!=-1; ) {
    if (syms.peekBack()==delim) break;
    m_pLanguageModel->EnterSymbol(sContext, sym);
  }
  return true;  
}

class ProgressStream : public CAlphabetMap::SymbolStream {
public:
  ProgressStream(std::istream &_in, CTrainer::ProgressIndicator *pProg, CMessageDisplay *pMsgs, off_t iStart=0) : SymbolStream(_in,pMsgs), m_iLastPos(iStart), m_pProg(pProg) {
  }
  void bytesRead(off_t num) {
    if (m_pProg) m_pProg->bytesRead(m_iLastPos += num);
  }
  off_t m_iLastPos;
private:
  CTrainer::ProgressIndicator *m_pProg;
};

bool 
Dasher::CTrainer::Parse(const string &strDesc, istream &in, bool bUser) {
  if (in.fail()) {
    m_pMsgs->FormatMessageWithString(_("Unable to open file \"%s\" for reading"),strDesc.c_str());
    return false;
  }
  ///easy enough to be re-entrant, so might as well
  string oldDesc=m_strDesc;
  m_strDesc = strDesc;
  ProgressStream syms(in,m_pProg,m_pMsgs);
  Train(syms);
  m_strDesc=oldDesc;
  return true;
}