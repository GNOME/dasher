#include "WordGeneratorBase.h"

using namespace Dasher;

CWordGeneratorBase::CWordGeneratorBase(const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap) : m_pAlph(pAlph), m_pAlphMap(pAlphMap) {
}

void CWordGeneratorBase::GetSymbols(std::vector<symbol> &into) {
  for (;;) {
    string s(GetLine());
    if (s.empty()) break; //no more lines, so no more symbols...
    stringstream line(s);
    CAlphabetMap::SymbolStream ss(line);
    for (int sym; (sym=ss.next(m_pAlphMap))!=-1; ) {
      if (!into.empty()) {
        const symbol lastSym(into.back());
        if (sym==0 && lastSym == m_pAlph->GetSpaceSymbol()) continue; //skip unknown after space
        if (lastSym==0) {
          //convert last sym to a space, or combine with next sym
          into.pop_back();
          if (sym!=0 && sym!=m_pAlph->GetSpaceSymbol()) 
            into.push_back(m_pAlph->GetSpaceSymbol());
        }
      }
      into.push_back(sym);
    }
    if (!into.empty()) {
      if (into.back()==0) into.pop_back();
      if (!into.empty()) break;
    }
    //didn't find any usable symbols in line! repeat...
  }
}

