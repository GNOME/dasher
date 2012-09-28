// JapaneseLanguageModel.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2005 David Ward
//                    2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "JapaneseLanguageModel.h"

#include "KanjiConversion.h"
#ifdef HAVE_LIBCANNA
#include "KanjiConversionCanna.h"
#elif _WIN32
#include "KanjiConversionIME.h"
#endif

#include <math.h>
#include <stack>
#include <sstream>
#include <iostream>

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////

CJapaneseLanguageModel::CJapaneseLanguageModel(CSettingsStore *pSettingsStore, const CAlphInfo *pAlph)
:CLanguageModel(pAlph->GetNumberTextSymbols()), m_pSettingsStore(pSettingsStore), m_iMaxOrder(5), NodesAllocated(0), m_NodeAlloc(8192), m_ContextAlloc(1024) {
  m_pRoot = m_NodeAlloc.Alloc();
  m_pRoot->symbol = -1;

  m_pRootContext = m_ContextAlloc.Alloc();
  m_pRootContext->head = m_pRoot;
  m_pRootContext->order = 0;

  // Cache the result of update exclusion - otherwise we have to look up a lot when training, which is slow

  bUpdateExclusion = ( m_pSettingsStore->GetLongParameter(LP_LM_UPDATE_EXCLUSION) !=0 );
  
}

/////////////////////////////////////////////////////////////////////

CJapaneseLanguageModel::~CJapaneseLanguageModel() {
}

/////////////////////////////////////////////////////////////////////
// Get the probability distribution at the context

void CJapaneseLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm) const {
  CJaPPMContext *ppmcontext = (CJaPPMContext *) (context);

  int iNumSymbols = GetSize();

    probs.resize(iNumSymbols);

  std::vector < bool > exclusions(iNumSymbols);

  int i;
  unsigned int ui;

  for(i = 0; i < iNumSymbols; i++) {
    probs[i] = 0;
    exclusions[i] = false;
  }

  bool doExclusion = 0; //FIXME

  int alpha = m_pSettingsStore->GetLongParameter( LP_LM_ALPHA );
  int beta = m_pSettingsStore->GetLongParameter( LP_LM_BETA );


  unsigned int iToSpend = norm;

  CJaPPMnode *pTemp = ppmcontext->head;

  bool has_convert_symbol = 0;  // Flag to show if a conversion symbol appears in the history
  std::vector < symbol > hiragana;      // Hiragana sequence to be converted
  std::vector < std::vector < symbol > >candidate;      // Temp list for candidates
  unsigned int kanji_pos = 0;

  // Initialize all probabilities to 0
  for(ui = 0; ui < probs.size(); ui++) {
    probs[ui] = 0;
  }

  // Look for a "Start Conversion" character in history
  for(ui = 0; ui < ppmcontext->history.size(); ui++) {
    if(ppmcontext->history[ui] == GetEndConversionSymbol()) {
      // If "End Conversion" character was found, clear the history
      ppmcontext->history.clear();
      break;
    }
    else if(ppmcontext->history[ui] == GetStartConversionSymbol()) {
      // We found the "Start Conversion" symbol!

      //TODO: Write a conversion code here
#ifdef HAVE_LIBCANNA
      CKanjiConversionCanna canna;
#elif _WIN32
      CKanjiConversionIME canna;
#else
      CKanjiConversion canna;
#endif
      //CKanjiConversion canna;
      std::vector < std::string > cand_list;

      // convert symbols to string
      string str;
      for(unsigned int j(0); j < hiragana.size(); j++) {
        str += GetText(hiragana[j]);
      }

      // do kanji conversion
      canna.ConvertKanji(str);

      // create candidate list
      for(unsigned int j(0); j < canna.phrase.size(); j++) {
        std::vector < std::string > tmp_cand_list;
        tmp_cand_list = cand_list;
        int nCandList = tmp_cand_list.size();
        cand_list.clear();
        for(unsigned int k(0); k < canna.phrase[j].candidate_list.size(); k++) {
          //CandidateString(canna.phrase[j].candidate_list[k]);
          //cout << "RAW:" << canna.phrase[j].candidate_list[k] << endl;
          if(k >= 30)
            break;

          if(nCandList) {
            for(int n = 0; n < nCandList; n++) {
              cand_list.push_back(tmp_cand_list[n] + canna.phrase[j].candidate_list[k]);
            }
          }
          else {
            cand_list.push_back(canna.phrase[j].candidate_list[k]);
          }
        }
      }

      //      std::cout << "Candidate list has " << cand_list.size() << " entries" << std::endl;

      // convert strings to symbols
      for(unsigned int j(0); j < cand_list.size(); j++) {
        //cout << "[" << j << "]" << cand_list[j] << endl;
        std::vector < symbol > new_cand;
        //SetCandidateString(cand_list[j]);
        GetSymbols(new_cand, cand_list[j]);
        /*for( int k=0; k<new_cand.size(); k++ )
           cout << GetText(new_cand[k]) << "[" << new_cand[k] << "] ";
           cout << endl; */
        candidate.push_back(new_cand);
      }
      candidate.push_back(hiragana);    // escape to hiragana
      has_convert_symbol = 1;

    }
    else if(has_convert_symbol && candidate.size()) {
      // disable the candidate if the symbol does not match the present symbol
      for(unsigned int j(0); j < candidate.size(); j++) {
        if(kanji_pos < candidate[j].size()) {
          if(ppmcontext->history[ui] != candidate[j][kanji_pos]) {
            for(unsigned int k(0); k < candidate[j].size(); k++) {
              candidate[j][k] = GetStartConversionSymbol();
            }
          }
        }
      }
      // Count up the kanji symbols we have seen so far
      kanji_pos++;
      //cout << "Kanji Pos:" << kanji_pos << endl;
    }
    else {
      hiragana.push_back(ppmcontext->history[ui]);
    }

  }

  //== Kanji Candidates
  if(has_convert_symbol && candidate.size()) {
    for(ui = 0; ui < probs.size(); ui++)
      exclusions[ui] = 0;

    // assign a large probability for the candidate
    int candidate_rank = 1;
    for(ui = 0; ui < candidate.size(); ui++) {
      /*cout << "Cand" << ui << ":";
         for( int j=0;  j<candidate[ui].size(); j++ ){
         cout << GetText( candidate[ui][j] );
         }
         cout << endl; */

      if(kanji_pos < candidate[ui].size()) {     // check if kanji_pos is valid in present candidate
        //cout << candidate_rank << ":" << GetText(candidate[ui][kanji_pos]) << "[" << kanji_pos << "]" << endl;
        if(candidate[ui][kanji_pos] != GetStartConversionSymbol() && !exclusions[candidate[ui][kanji_pos]]) {     // check if present candidate is enabled
          /*cout << "Selected:";
             for( int hoge = 0; hoge<candidate[ui].size(); hoge++ ){
             cout << GetText(candidate[ui][hoge]) << "(" << candidate[ui][hoge] << ")";
             }
             cout << endl; */

          uint32 p = (uint32) ((double)iToSpend / ((candidate_rank + 15) * (candidate_rank + 16)));       // a large probability
          probs[candidate[ui][kanji_pos]] += p;
          iToSpend -= p;
          exclusions[candidate[ui][kanji_pos]] = 1;
          candidate_rank++;
        }
      }
    }

    if(candidate_rank > 1) {
      unsigned int total = 0;
      for(ui = 0; ui < probs.size(); ui++) {
        total += probs[ui];
      }
      if(total) {
        iToSpend = norm;
        for(unsigned int ui(0); ui < probs.size(); ui++) {
          if(probs[ui]) {
            //cout << GetText(ui) << "       " << probs[ui] << " -> ";
            probs[ui] = (uint32) (((double)norm / (double)total) * (double)probs[ui]);
            iToSpend -= probs[ui];
            //cout << probs[ui] << endl;
          }
        }
      }
      pTemp = NULL;             // Skip normal PPM
    }
    else {
      probs[GetEndConversionSymbol()] += iToSpend;
      iToSpend = 0;
      candidate.clear();        // conversion is finished. clear the history
      ppmcontext->history.clear();
      pTemp = NULL;             // Skip normal PPM
    }
  }

  while(pTemp != 0) {
    int iTotal = 0;

    CJaPPMnode *pSymbol = pTemp->child;
    while(pSymbol) {
      int sym = pSymbol->symbol;
      if(!(exclusions[sym] && doExclusion))
        iTotal += pSymbol->count;
      pSymbol = pSymbol->next;
    }

    if(iTotal) {
      unsigned int size_of_slice = iToSpend;
      pSymbol = pTemp->child;
      while(pSymbol) {
        if(!(exclusions[pSymbol->symbol] && doExclusion)) {
          exclusions[pSymbol->symbol] = 1;

          unsigned int p = static_cast < myint > (size_of_slice) * (100 * pSymbol->count - beta) / (100 * iTotal + alpha);

          probs[pSymbol->symbol] += p;
          iToSpend -= p;
        }
        //                              Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);      
        //                              DebugOutput(debug);
        pSymbol = pSymbol->next;
      }
    }
    pTemp = pTemp->vine;
  }

  unsigned int size_of_slice = iToSpend;
  int symbolsleft = 0;

  for(i = 0; i < iNumSymbols; i++)
    if(!(exclusions[i] && doExclusion))
      symbolsleft++;

//      std::ostringstream str;
//      for (sym=0;sym<modelchars;sym++)
//              str << probs[sym] << " ";
//      str << std::endl;
//      DASHER_TRACEOUTPUT("probs %s",str.str().c_str());

//      std::ostringstream str2;
//      for (sym=0;sym<modelchars;sym++)
//              str2 << valid[sym] << " ";
//      str2 << std::endl;
//      DASHER_TRACEOUTPUT("valid %s",str2.str().c_str());

  for(i = 0; i < iNumSymbols; i++) {
    if(!(exclusions[i] && doExclusion)) {
      unsigned int p = size_of_slice / symbolsleft;
      probs[i] += p;
      iToSpend -= p;
    }
  }

  int iLeft = iNumSymbols;

  for(int j = 0; j < iNumSymbols; ++j) {
    unsigned int p = iToSpend / iLeft;
    probs[j] += p;
    --iLeft;
    iToSpend -= p;
  }

  DASHER_ASSERT(iToSpend == 0);
}

void CJapaneseLanguageModel::AddSymbol(CJapaneseLanguageModel::CJaPPMContext &context, int sym)
        // add symbol to the context
        // creates new nodes, updates counts
        // and leaves 'context' at the new context
{
  DASHER_ASSERT(sym >= 0 && sym <= GetSize());

  CJaPPMnode *vineptr, *temp;
  int updatecnt = 1;

  temp = context.head->vine;
  context.head = AddSymbolToNode(context.head, sym, &updatecnt);
  vineptr = context.head;
  context.order++;

  while(temp != 0) {
    vineptr->vine = AddSymbolToNode(temp, sym, &updatecnt);
    vineptr = vineptr->vine;
    temp = temp->vine;
  }
  vineptr->vine = m_pRoot;

  //  m_iMaxOrder = LanguageModelParams()->GetValue(std::string("LMMaxOrder"));
  m_iMaxOrder = m_pSettingsStore->GetLongParameter( LP_LM_MAX_ORDER );

  while(context.order > m_iMaxOrder) {
    context.head = context.head->vine;
    context.order--;
  }
}

/////////////////////////////////////////////////////////////////////
// Update context with symbol 'Symbol'

void CJapaneseLanguageModel::EnterSymbol(Context c, int Symbol) {
  DASHER_ASSERT(Symbol >= 0 && Symbol <= GetSize());

  CJapaneseLanguageModel::CJaPPMContext & context = *(CJaPPMContext *) (c);

  CJaPPMnode *find;

  context.history.push_back(Symbol);
  if(context.history.size() > 100) {
    context.history.erase(context.history.begin());
  }

  while(context.head) {

    if(context.order < m_iMaxOrder) {   // Only try to extend the context if it's not going to make it too long
      find = context.head->find_symbol(Symbol);
      if(find) {
        context.order++;
        context.head = find;
        //      Usprintf(debug,TEXT("found context %x order %d\n"),head,order);
        //      DebugOutput(debug);

        //                      std::cout << context.order << std::endl;
        return;
      }
    }

    // If we can't extend the current context, follow vine pointer to shorten it and try again

    context.order--;
    context.head = context.head->vine;
  }

  if(context.head == 0) {
    context.head = m_pRoot;
    context.order = 0;
  }

  //      std::cout << context.order << std::endl;

}

/////////////////////////////////////////////////////////////////////

void CJapaneseLanguageModel::LearnSymbol(Context c, int Symbol) {
  DASHER_ASSERT(Symbol >= 0 && Symbol <= GetSize());

  CJapaneseLanguageModel::CJaPPMContext & context = *(CJaPPMContext *) (c);
  AddSymbol(context, Symbol);
}

void CJapaneseLanguageModel::dumpSymbol(int sym) {
  if((sym <= 32) || (sym >= 127))
    printf("<%d>", sym);
  else
    printf("%c", sym);
}

void CJapaneseLanguageModel::dumpString(char *str, int pos, int len)
        // Dump the string STR starting at position POS
{
  char cc;
  int p;
  for(p = pos; p < pos + len; p++) {
    cc = str[p];
    if((cc <= 31) || (cc >= 127))
      printf("<%d>", cc);
    else
      printf("%c", cc);
  }
}

void CJapaneseLanguageModel::dumpTrie(CJapaneseLanguageModel::CJaPPMnode *t, int d)
        // diagnostic display of the PPM trie from node t and deeper
{
//TODO
/*
	dchar debug[256];
	int sym;
	CJaPPMnode *s;
	Usprintf( debug,TEXT("%5d %7x "), d, t );
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
	if (t < 0) // pointer to input
		printf( "                     <" );
	else {
		Usprintf(debug,TEXT( " %3d %5d %7x  %7x  %7x    <"), t->symbol,t->count, t->vine, t->child, t->next );
		//TODO: Uncomment this when headers sort out
		//DebugOutput(debug);
	}
	
	dumpString( dumpTrieStr, 0, d );
	Usprintf( debug,TEXT(">\n") );
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
	if (t != 0) {
		s = t->child;
		while (s != 0) {
			sym =s->symbol;
			
			dumpTrieStr [d] = sym;
			dumpTrie( s, d+1 );
			s = s->next;
		}
	}
*/
}

void CJapaneseLanguageModel::dump()
        // diagnostic display of the whole PPM trie
{
// TODO:
/*
	dchar debug[256];
	Usprintf(debug,TEXT(  "Dump of Trie : \n" ));
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
	Usprintf(debug,TEXT(   "---------------\n" ));
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
	Usprintf( debug,TEXT(  "depth node     symbol count  vine   child      next   context\n") );
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
	dumpTrie( root, 0 );
	Usprintf( debug,TEXT(  "---------------\n" ));
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
	Usprintf(debug,TEXT( "\n" ));
	//TODO: Uncomment this when headers sort out
	//DebugOutput(debug);
*/
}

////////////////////////////////////////////////////////////////////////
/// PPMnode definitions 
////////////////////////////////////////////////////////////////////////

CJapaneseLanguageModel::CJaPPMnode * CJapaneseLanguageModel::CJaPPMnode::find_symbol(int sym) const
// see if symbol is a child of node
{
  //  printf("finding symbol %d at node %d\n",sym,node->id);
  CJaPPMnode *found = child;

  while(found) {
    if(found->symbol == sym) {
      return found;
    }
    found = found->next;
  }
  return 0;
}

CJapaneseLanguageModel::CJaPPMnode * CJapaneseLanguageModel::AddSymbolToNode(CJaPPMnode *pNode, int sym, int *update) {
  CJaPPMnode *pReturn = pNode->find_symbol(sym);

  //      std::cout << sym << ",";

  if(pReturn != NULL) {
    //      std::cout << "Using existing node" << std::endl;

    //            if (*update || (LanguageModelParams()->GetValue("LMUpdateExclusion") == 0) ) 
    if(*update || !bUpdateExclusion) {  // perform update exclusions
      pReturn->count++;
      *update = 0;
    }
    return pReturn;
  }

  //       std::cout << "Creating new node" << std::endl;

  pReturn = m_NodeAlloc.Alloc();        // count is initialized to 1
  pReturn->symbol = sym;
  pReturn->next = pNode->child;
  pNode->child = pReturn;

  ++NodesAllocated;

  return pReturn;

}
