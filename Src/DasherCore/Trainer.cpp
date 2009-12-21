
#include "../Common/Common.h"

#include "Trainer.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include <cstring>
#include <sstream>

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
  : m_pAlphabet(pAlphabet), m_pLanguageModel(pLanguageModel), m_pInfo(pInfo) {
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

class ProgressStream : public CAlphabetMap::SymbolStream {
public:
  ProgressStream(std::istream &_in, CTrainer::ProgressIndicator *pProg, off_t iStart=0) : SymbolStream(_in), m_iLastPos(iStart), m_pProg(pProg) {
  }
  void bytesRead(off_t num) {
    if (m_pProg) m_pProg->bytesRead(m_iLastPos += num);
  }
  off_t m_iLastPos;
private:
  CTrainer::ProgressIndicator *m_pProg;
};

void 
Dasher::CTrainer::LoadFile(const std::string &strFileName, ProgressIndicator *pProg) {
  if(strFileName == "")
    return;
  
  FILE *pInputFile;
  if((pInputFile = fopen(strFileName.c_str(), "r")) == (FILE *) 0)
    return;
  
  char szTestBuffer[6];
  
  int iNumberRead = fread(szTestBuffer, 1, 5, pInputFile);
  szTestBuffer[iNumberRead] = '\0';
  
  fclose(pInputFile);
  
  if(!strcmp(szTestBuffer, "<?xml")) {
    //Invoke AbstractXMLParser method
    m_bInSegment = false;
    m_iLastBytes=0;
    ParseFile(strFileName);
  } else {
    std::ifstream in(strFileName.c_str(), std::ios::binary);
    if (in.fail()) {
      std::cerr << "Unable to open file \"" << strFileName << "\" for reading" << std::endl;
      return;
    }
    ProgressStream syms(in,pProg);
    Train(syms);
  
    in.close();
  }
}

void CTrainer::XmlStartHandler(const XML_Char *szName, const XML_Char **pAtts) {
  if(!strcmp(szName, "segment")) {
    m_strCurrentText = "";
    m_bInSegment = true;
  }
}

void CTrainer::XmlEndHandler(const XML_Char *szName) {
  if(!strcmp(szName, "segment")) {
    std::istringstream in(m_strCurrentText);
    ProgressStream syms(in, m_pProg, m_iLastBytes);
    Train(syms);
    m_iLastBytes = syms.m_iLastPos; //count that segment, ready for next
    m_bInSegment = false;
  }
}

void CTrainer::XmlCData(const XML_Char *szS, int iLen) {
  if(m_bInSegment)
    m_strCurrentText += std::string(szS, iLen);
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
