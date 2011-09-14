
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
  
  ProgressStream syms(in,m_pProg,m_pMsgs);
  Train(syms);
  
  return true;
}

CMandarinTrainer::CMandarinTrainer(CMessageDisplay *pMsgs, CPPMPYLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pPYAlphabet, const CAlphabetMap *pCHAlphabet, const std::string &strDelim)
: CTrainer(pMsgs, pLanguageModel, pInfo, pCHAlphabet), m_pPYAlphabet(pPYAlphabet), m_strDelim(strDelim) {
}

void CMandarinTrainer::Train(CAlphabetMap::SymbolStream &syms) {
  unsigned numberofchar = 0;
  CLanguageModel::Context trainContext = m_pLanguageModel->CreateEmptyContext();
  
  for (string s; (s=syms.peekAhead()).length();) {
    syms.next(m_pPYAlphabet); //skip over character at which we just peeked (we don't need the symbol#)
    
    if (s == m_strDelim) { //found delimiter, so process next two characters
      symbol Sympy = syms.next(m_pPYAlphabet);
      if (Sympy==-1) break; //EOF
      if (Sympy==0) {
        m_pMsgs->FormatMessageWithString(_("Training file contains unknown source alphabet character %s"), syms.peekBack().c_str());
      }
      symbol Symchar = syms.next(m_pAlphabet);
      if (Symchar==-1) break; //EOF...ignore final Pinyin?
      if (Symchar==0) {
        m_pMsgs->FormatMessageWithString(_("Training file contains unknown target alphabet character %s"), syms.peekBack().c_str());
      }
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
