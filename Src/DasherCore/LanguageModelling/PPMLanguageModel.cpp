// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"
#include "PPMLanguageModel.h"

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

CPPMLanguageModel::CPPMLanguageModel(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CSymbolAlphabet &SymbolAlphabet)
:CLanguageModel(pEventHandler, pSettingsStore, SymbolAlphabet), m_iMaxOrder(4), NodesAllocated(0), m_NodeAlloc(8192), m_ContextAlloc(1024) {
  m_pRoot = m_NodeAlloc.Alloc();
  m_pRoot->symbol = -1;

  m_pRootContext = m_ContextAlloc.Alloc();
  m_pRootContext->head = m_pRoot;
  m_pRootContext->order = 0;

  // Cache the result of update exclusion - otherwise we have to look up a lot when training, which is slow

  // FIXME - this should be a boolean parameter

  bUpdateExclusion = ( GetLongParameter(LP_LM_UPDATE_EXCLUSION) !=0 );

}

/////////////////////////////////////////////////////////////////////

CPPMLanguageModel::~CPPMLanguageModel() {
}

/////////////////////////////////////////////////////////////////////
// Get the probability distribution at the context

void CPPMLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm) const {
  const CPPMContext *ppmcontext = (const CPPMContext *)(context);

  int iNumSymbols = GetSize();
  
  probs.resize(iNumSymbols);

  std::vector < bool > exclusions(iNumSymbols);

  int i;
  for(i = 0; i < iNumSymbols; i++) {
    probs[i] = 0;
    exclusions[i] = false;
  }

  //  bool doExclusion = GetLongParameter( LP_LM_ALPHA );
  bool doExclusion = 0; //FIXME

  int alpha = GetLongParameter( LP_LM_ALPHA );
  int beta = GetLongParameter( LP_LM_BETA );

  unsigned int iToSpend = norm;

  CPPMnode *pTemp = ppmcontext->head;

  while(pTemp != 0) {
    int iTotal = 0;

    CPPMnode *pSymbol = pTemp->child;
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

  for(i = 1; i < iNumSymbols; i++)
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

  for(i = 1; i < iNumSymbols; i++) {
    if(!(exclusions[i] && doExclusion)) {
      unsigned int p = size_of_slice / symbolsleft;
      probs[i] += p;
      iToSpend -= p;
    }
  }

  int iLeft = iNumSymbols-1;

  for(int j = 1; j < iNumSymbols; ++j) {
    unsigned int p = iToSpend / iLeft;
    probs[j] += p;
    --iLeft;
    iToSpend -= p;
  }

  DASHER_ASSERT(iToSpend == 0);
}

void CPPMLanguageModel::AddSymbol(CPPMLanguageModel::CPPMContext &context, int sym)
        // add symbol to the context
        // creates new nodes, updates counts
        // and leaves 'context' at the new context
{
  // Ignore attempts to add the root symbol

  if(sym==0)
    return;

  DASHER_ASSERT(sym >= 0 && sym < GetSize());

  CPPMnode *vineptr, *temp;
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

  //m_iMaxOrder = LanguageModelParams()->GetValue(std::string("LMMaxOrder"));
  m_iMaxOrder = GetLongParameter( LP_LM_MAX_ORDER );

  while(context.order > m_iMaxOrder) {
    context.head = context.head->vine;
    context.order--;
  }
}

/////////////////////////////////////////////////////////////////////
// Update context with symbol 'Symbol'

void CPPMLanguageModel::EnterSymbol(Context c, int Symbol) {
  if(Symbol==0)
    return;

  DASHER_ASSERT(Symbol >= 0 && Symbol < GetSize());

  CPPMLanguageModel::CPPMContext & context = *(CPPMContext *) (c);

  CPPMnode *find;

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

void CPPMLanguageModel::LearnSymbol(Context c, int Symbol) {
  if(Symbol==0)
    return;
  

  DASHER_ASSERT(Symbol >= 0 && Symbol < GetSize());
  CPPMLanguageModel::CPPMContext & context = *(CPPMContext *) (c);
  AddSymbol(context, Symbol);
}

void CPPMLanguageModel::dumpSymbol(int sym) {
  if((sym <= 32) || (sym >= 127))
    printf("<%d>", sym);
  else
    printf("%c", sym);
}

void CPPMLanguageModel::dumpString(char *str, int pos, int len)
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

void CPPMLanguageModel::dumpTrie(CPPMLanguageModel::CPPMnode *t, int d)
        // diagnostic display of the PPM trie from node t and deeper
{
//TODO
/*
	dchar debug[256];
	int sym;
	CPPMnode *s;
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

void CPPMLanguageModel::dump()
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

CPPMLanguageModel::CPPMnode * CPPMLanguageModel::CPPMnode::find_symbol(int sym) const
// see if symbol is a child of node
{
  //  printf("finding symbol %d at node %d\n",sym,node->id);
  CPPMnode *found = child;

  while(found) {
    if(found->symbol == sym) {
      return found;
    }
    found = found->next;
  }
  return 0;
}

CPPMLanguageModel::CPPMnode * CPPMLanguageModel::AddSymbolToNode(CPPMnode *pNode, int sym, int *update) {
  CPPMnode *pReturn = pNode->find_symbol(sym);

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

struct BinaryRecord {
  int m_iIndex;
  int m_iChild;
  int m_iNext;
  int m_iVine;
  unsigned short int m_iCount;
  short int m_iSymbol;
};

bool CPPMLanguageModel::WriteToFile(std::string strFilename) {

  std::map<CPPMnode *, int> mapIdx;
  int iNextIdx(1); // Index of 0 means NULL;

  std::ofstream oOutputFile(strFilename.c_str());

  RecursiveWrite(m_pRoot, &mapIdx, &iNextIdx, &oOutputFile);

  oOutputFile.close();

  return false;
};

bool CPPMLanguageModel::RecursiveWrite(CPPMnode *pNode, std::map<CPPMnode *, int> *pmapIdx, int *pNextIdx, std::ofstream *pOutputFile) {

  // Dump node here

  BinaryRecord sBR;

  sBR.m_iIndex = GetIndex(pNode, pmapIdx, pNextIdx); 
  sBR.m_iChild = GetIndex(pNode->child, pmapIdx, pNextIdx); 
  sBR.m_iNext = GetIndex(pNode->next, pmapIdx, pNextIdx); 
  sBR.m_iVine = GetIndex(pNode->vine, pmapIdx, pNextIdx);
  sBR.m_iCount = pNode->count;
  sBR.m_iSymbol = pNode->symbol;

  pOutputFile->write(reinterpret_cast<char*>(&sBR), sizeof(BinaryRecord));

  CPPMnode *pCurrentChild(pNode->child);
  
  while(pCurrentChild != NULL) {
    RecursiveWrite(pCurrentChild, pmapIdx, pNextIdx, pOutputFile);
    pCurrentChild = pCurrentChild->next;
  }

  return true;
};

int CPPMLanguageModel::GetIndex(CPPMnode *pAddr, std::map<CPPMnode *, int> *pmapIdx, int *pNextIdx) {

  int iIndex;
  if(pAddr == NULL)
    iIndex = 0;
  else {
    std::map<CPPMnode *, int>::iterator it(pmapIdx->find(pAddr));
    
    if(it == pmapIdx->end()) {
      iIndex = *pNextIdx;
      pmapIdx->insert(std::pair<CPPMnode *, int>(pAddr, iIndex));
      ++(*pNextIdx);
    }
    else {
      iIndex = it->second;
    }
  }
  return iIndex;
};

bool CPPMLanguageModel::ReadFromFile(std::string strFilename) {
  
  std::ifstream oInputFile(strFilename.c_str());
  std::map<int, CPPMnode*> oMap;
  BinaryRecord sBR;
  bool bStarted(false);

  while(!oInputFile.eof()) {
    oInputFile.read(reinterpret_cast<char *>(&sBR), sizeof(BinaryRecord));

    CPPMnode *pCurrent(GetAddress(sBR.m_iIndex, &oMap));

    pCurrent->child = GetAddress(sBR.m_iChild, &oMap);
    pCurrent->next = GetAddress(sBR.m_iNext, &oMap);
    pCurrent->vine = GetAddress(sBR.m_iVine, &oMap);
    pCurrent->count = sBR.m_iCount;
    pCurrent->symbol = sBR.m_iSymbol;

    if(!bStarted) {
      m_pRoot = pCurrent;
      bStarted = true;
    }
  }

  oInputFile.close();

  return false;
};

CPPMLanguageModel::CPPMnode *CPPMLanguageModel::GetAddress(int iIndex, std::map<int, CPPMnode*> *pMap) {
  std::map<int, CPPMnode*>::iterator it(pMap->find(iIndex));

  if(it == pMap->end()) {
    CPPMnode *pNewNode;
    pNewNode = m_NodeAlloc.Alloc();
    pMap->insert(std::pair<int, CPPMnode*>(iIndex, pNewNode));
    return pNewNode;
  }
  else {
    return it->second;
  }
}
