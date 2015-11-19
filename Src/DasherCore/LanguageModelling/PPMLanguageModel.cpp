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
#include <string.h>
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

CAbstractPPM::CAbstractPPM(CSettingsUser *pCreator, int iNumSyms, CPPMnode *pRoot, int iMaxOrder)
: CLanguageModel(iNumSyms), CSettingsUser(pCreator), m_pRoot(pRoot), m_iMaxOrder(iMaxOrder<0 ? GetLongParameter(LP_LM_MAX_ORDER) : iMaxOrder), bUpdateExclusion( GetLongParameter(LP_LM_UPDATE_EXCLUSION)!=0 ), m_ContextAlloc(1024) {
  m_pRootContext = m_ContextAlloc.Alloc();
  m_pRootContext->head = m_pRoot;
  m_pRootContext->order = 0;
}

bool CAbstractPPM::isValidContext(const Context context) const {
  return m_setContexts.count((const CPPMContext *)context) > 0;
}

/////////////////////////////////////////////////////////////////////
// Get the probability distribution at the context

void CPPMLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) const {
  const CPPMContext *ppmcontext = (const CPPMContext *)(context);

  DASHER_ASSERT(isValidContext(context));

  int iNumSymbols = GetSize();
  
  probs.resize(iNumSymbols);

  std::vector < bool > exclusions(iNumSymbols);
  
  unsigned int iToSpend = norm;
  unsigned int iUniformLeft = iUniform;

  // TODO: Sort out zero symbol case
  probs[0] = 0;
  exclusions[0] = false;

  for (int i = 1; i < iNumSymbols; i++) {
    probs[i] = iUniformLeft / (iNumSymbols - i);
    iUniformLeft -= probs[i];
    iToSpend -= probs[i];
    exclusions[i] = false;
  }

  DASHER_ASSERT(iUniformLeft == 0);

  //  bool doExclusion = GetLongParameter( LP_LM_ALPHA );
  bool doExclusion = 0; //FIXME

  int alpha = GetLongParameter( LP_LM_ALPHA );
  int beta = GetLongParameter( LP_LM_BETA );

  for (CPPMnode *pTemp = ppmcontext->head; pTemp; pTemp=pTemp->vine) {
    int iTotal = 0;

    for (ChildIterator pSymbol = pTemp->children(); pSymbol != pTemp->end(); pSymbol++) {
      symbol sym = (*pSymbol)->sym;
      if(!(exclusions[sym] && doExclusion))
        iTotal += (*pSymbol)->count;
    }

    if(iTotal) {
      unsigned int size_of_slice = iToSpend;
      for (ChildIterator pSymbol = pTemp->children(); pSymbol != pTemp->end(); pSymbol++) {
        if(!(exclusions[(*pSymbol)->sym] && doExclusion)) {
          exclusions[(*pSymbol)->sym] = 1;

          unsigned int p = static_cast < myint > (size_of_slice) * (100 * (*pSymbol)->count - beta) / (100 * iTotal + alpha);

          probs[(*pSymbol)->sym] += p;
          iToSpend -= p;
        }
        //                              Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);      
        //                              DebugOutput(debug);
      }
    }
  }

  unsigned int size_of_slice = iToSpend;
  int symbolsleft = 0;

  for(int i = 1; i < iNumSymbols; i++)
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

  for(int i = 1; i < iNumSymbols; i++) {
    if(!(exclusions[i] && doExclusion)) {
      unsigned int p = size_of_slice / symbolsleft;
      probs[i] += p;
      iToSpend -= p;
    }
  }

  int iLeft = iNumSymbols-1;

  for(int i = 1; i < iNumSymbols; i++) {
    unsigned int p = iToSpend / iLeft;
    probs[i] += p;
    --iLeft;
    iToSpend -= p;
  }

  DASHER_ASSERT(iToSpend == 0);
}

/////////////////////////////////////////////////////////////////////
// Update context with symbol 'Symbol'

void CAbstractPPM::EnterSymbol(Context c, int Symbol) {
  if(Symbol==0)
    return;

  DASHER_ASSERT(Symbol >= 0 && Symbol < GetSize());

  CPPMContext & context = *(CPPMContext *) (c);

  while(context.head) {

    if(context.order < m_iMaxOrder) {   // Only try to extend the context if it's not going to make it too long
      if (CPPMnode *find = context.head->find_symbol(Symbol)) {
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
// add symbol to the context
// creates new nodes, updates counts
// and leaves 'context' at the new context
void CAbstractPPM::LearnSymbol(Context c, int Symbol) {
  
  if(Symbol==0)
    return;
  

  DASHER_ASSERT(Symbol >= 0 && Symbol < GetSize());
  CPPMContext & context = *(CPPMContext *) (c);
  
  CPPMnode* n = AddSymbolToNode(context.head, Symbol);
  DASHER_ASSERT ( n == context.head->find_symbol(Symbol));
  context.head=n;
  context.order++;
  
  while(context.order > m_iMaxOrder) {
    context.head = context.head->vine;
    context.order--;
  }
  
}

void CAbstractPPM::dumpSymbol(symbol sym) {
  if((sym <= 32) || (sym >= 127))
    printf("<%d>", sym);
  else
    printf("%c", sym);
}

void CAbstractPPM::dumpString(char *str, int pos, int len)
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

void CAbstractPPM::dumpTrie(CAbstractPPM::CPPMnode *t, int d)
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
		Usprintf(debug,TEXT( " %3d %5d %7x  %7x  %7x    <"), t->sym,t->count, t->vine, t->child, t->next );
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
			sym =s->sym;
			
			dumpTrieStr [d] = sym;
			dumpTrie( s, d+1 );
			s = s->next;
		}
	}
*/
}

void CAbstractPPM::dump()
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

bool CAbstractPPM::eq(CAbstractPPM *other) {
  std::map<CPPMnode *,CPPMnode *> equivs;
  if (!m_pRoot->eq(other->m_pRoot,equivs)) return false;
  //have first & second being equivalent, for all entries in map, except vine ptrs not checked.
  for (std::map<CPPMnode *,CPPMnode *>::iterator it=equivs.begin(); it!=equivs.end(); it++) {
    CPPMnode *myVine = it->first->vine;
    CPPMnode *oVine = it->second->vine;
    if (myVine==NULL) {
      if (oVine==NULL) continue;
      return false;
    } else if (oVine==NULL) return false;
    std::map<CPPMnode *,CPPMnode *>::iterator found = equivs.find(myVine);
    if (found->second != oVine) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////
/// PPMnode definitions 
////////////////////////////////////////////////////////////////////////

bool CAbstractPPM::CPPMnode::eq(CAbstractPPM::CPPMnode *other, std::map<CPPMnode *,CPPMnode *> &equivs) {
  if (sym != other->sym)
    return false;
  if (count != other->count)
    return false;
  //check children....but allow for different orders by sorting into symbol order
  std::map<symbol, CPPMnode *> thisCh, otherCh;
  for (ChildIterator it = children(); it != end(); it++) thisCh[(*it)->sym] = *it;
  for (ChildIterator it = other->children(); it != other->end(); it++) otherCh[(*it)->sym] = *it;
  if (thisCh.size() != otherCh.size())
    return false;
  for (std::map<symbol, CPPMnode *>::iterator it1 = thisCh.begin(), it2=otherCh.begin(); it1 != thisCh.end() ; it1++, it2++)
    if (!it1->second->eq(it2->second, equivs))
      return false; //different - note eq checks symbol
  equivs.insert(std::pair<CPPMnode *,CPPMnode *>(this,other));
  return true;
}

#define MAX_RUN 4

CAbstractPPM::CPPMnode * CAbstractPPM::CPPMnode::find_symbol(symbol sym) const
// see if symbol is a child of node
{
  if (m_iNumChildSlots < 0) //negative to mean "full alphabet", use direct indexing
    return m_ppChildren[sym];
  if (m_iNumChildSlots == 1) {
    if (m_pChild->sym == sym)
      return m_pChild;
    return 0;
  }
  if (m_iNumChildSlots <= MAX_RUN) {
    for (int i = 0; i < m_iNumChildSlots && m_ppChildren[i]; i++)
      if (m_ppChildren[i]->sym == sym) return m_ppChildren[i];
    return 0;
  }
  //  printf("finding symbol %d at node %d\n",sym,node->id);

  for (int i = sym; ; i++) { //search through elements which have overflowed into subsequent slots
    CPPMnode *found = this->m_ppChildren[i % m_iNumChildSlots]; //wrap round
    if (!found) return 0; //null element
    if(found->sym == sym) {
      return found;
    }
  }
  return 0;
}

void CAbstractPPM::CPPMnode::AddChild(CPPMnode *pNewChild, int numSymbols) {
  if (m_iNumChildSlots < 0) {
    m_ppChildren[pNewChild->sym] = pNewChild;
  }
  else 
  {
    if (m_iNumChildSlots == 0) {
      m_iNumChildSlots = 1;
      m_pChild = pNewChild;
      return;
    } else if (m_iNumChildSlots == 1) {
      //no room, have to resize...
    } else if (m_iNumChildSlots<=MAX_RUN) {
      for (int i = 0; i < m_iNumChildSlots; i++)
        if (!m_ppChildren[i]) {
          m_ppChildren[i] = pNewChild;
          return;
        }
    } else {
      

      int start = pNewChild->sym;
      //find length of run (including to-be-inserted element)....
      while (m_ppChildren[start = (start + m_iNumChildSlots - 1) % m_iNumChildSlots]);

      int idx = pNewChild->sym;
      while (m_ppChildren[idx %= m_iNumChildSlots]) ++idx;
      //found NULL
      int stop = idx;
      while (m_ppChildren[stop = (stop + 1) % m_iNumChildSlots]);
      //start and idx point to NULLs (with inserted element somewhere inbetween)
      
      int runLen = (m_iNumChildSlots + stop - (start+1)) % m_iNumChildSlots;
      if (runLen <= MAX_RUN) {
        //ok, maintain size
        m_ppChildren[idx] = pNewChild;
        return;
      }
    }
    //resize!
    CPPMnode **oldChildren = m_ppChildren;
    int oldSlots = m_iNumChildSlots;
    int newNumElems;
    if (m_iNumChildSlots >= numSymbols/4) {
      m_iNumChildSlots = -numSymbols; // negative = "use direct indexing"
      newNumElems = numSymbols;
    } else {
      m_iNumChildSlots+=m_iNumChildSlots+1;
      newNumElems = m_iNumChildSlots;
    }
    m_ppChildren = new CPPMnode *[newNumElems]; //null terminator
    memset (m_ppChildren, 0, sizeof(CPPMnode *)*newNumElems);
    if (oldSlots == 1)
      AddChild((CPPMnode *)oldChildren, numSymbols);
    else {
      while (oldSlots-- > 0) if (oldChildren[oldSlots]) AddChild(oldChildren[oldSlots], numSymbols);
      delete[] oldChildren;
    }
    AddChild(pNewChild, numSymbols);
  }
}

CAbstractPPM::CPPMnode * CAbstractPPM::AddSymbolToNode(CPPMnode *pNode, symbol sym) {

  CPPMnode *pReturn = pNode->find_symbol(sym);

  //      std::cout << sym << ",";

  if(pReturn != NULL) {
    pReturn->count++;
    if (!bUpdateExclusion) {
      //update vine contexts too. Guaranteed to exist if child does!
      for (CPPMnode *v = pReturn->vine; v; v=v->vine) {
        DASHER_ASSERT(v == m_pRoot || v->sym == sym);
        v->count++;
      }
    }
  } else {
    //symbol does not exist at this level
    pReturn = makeNode(sym); //count initialized to 1 but no vine pointer
    pNode->AddChild(pReturn, GetSize());
    pReturn->vine = (pNode==m_pRoot) ? m_pRoot : AddSymbolToNode(pNode->vine,sym);
  }
  
  return pReturn;
}

CPPMLanguageModel::CPPMLanguageModel(CSettingsUser *pCreator, int iNumSyms)
: CAbstractPPM(pCreator, iNumSyms, new CPPMnode(-1)), NodesAllocated(0), m_NodeAlloc(8192) {
}

CAbstractPPM::CPPMnode *CPPMLanguageModel::makeNode(int sym) {
  CPPMnode *res = m_NodeAlloc.Alloc();
  res->sym = sym;
  ++NodesAllocated;
  return res;
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

  RecursiveWrite(m_pRoot, NULL, &mapIdx, &iNextIdx, &oOutputFile);

  oOutputFile.close();

  return false;
}

bool CPPMLanguageModel::RecursiveWrite(CPPMnode *pNode, CPPMnode *pNextSibling, std::map<CPPMnode *, int> *pmapIdx, int *pNextIdx, std::ofstream *pOutputFile) {

  // Dump node here

  BinaryRecord sBR;

  sBR.m_iIndex = GetIndex(pNode, pmapIdx, pNextIdx); 
  sBR.m_iNext = GetIndex(pNextSibling, pmapIdx, pNextIdx); 
  sBR.m_iVine = GetIndex(pNode->vine, pmapIdx, pNextIdx);
  sBR.m_iCount = pNode->count;
  sBR.m_iSymbol = pNode->sym;

  ChildIterator it =pNode->children();
  CPPMnode *pCurrentChild = (it == pNode->end()) ? NULL : *it++;
  sBR.m_iChild = GetIndex(pCurrentChild, pmapIdx, pNextIdx); 
  
  pOutputFile->write(reinterpret_cast<char*>(&sBR), sizeof(BinaryRecord));

  if (pCurrentChild) {
    for (CPPMnode *pNextChild; it != pNode->end(); pCurrentChild = pNextChild) {
      pNextChild = *it++;
      RecursiveWrite(pCurrentChild, pNextChild, pmapIdx, pNextIdx, pOutputFile);
    }
    RecursiveWrite(pCurrentChild, NULL, pmapIdx, pNextIdx, pOutputFile);
  }

  return true;
}

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
}

bool CPPMLanguageModel::ReadFromFile(std::string strFilename) {
  
  std::ifstream oInputFile(strFilename.c_str());
  //map from file index, to address of node object with that index
  std::map<int, CPPMnode*> oMap;
  //map from file index, to address of *parent* for that node
  // - only stored for the child that will *next* be read.
  std::map<int, CPPMnode *> parentMap;
  BinaryRecord sBR;
  bool bStarted(false);

  while(!oInputFile.eof()) {
    oInputFile.read(reinterpret_cast<char *>(&sBR), sizeof(BinaryRecord));

    CPPMnode *pCurrent(GetAddress(sBR.m_iIndex, &oMap));

    pCurrent->vine = GetAddress(sBR.m_iVine, &oMap);
    pCurrent->count = sBR.m_iCount;
    pCurrent->sym = sBR.m_iSymbol;

    //if this node has a parent...
    std::map<int,CPPMnode *>::iterator it(parentMap.find(sBR.m_iIndex));
    if (it != parentMap.end()) {
      CPPMnode *parent = it->second;
      parent->AddChild(pCurrent,GetSize());
      //erase the record of parent hood, now we've realized it
      parentMap.erase(it);
      //add mapping for the _next_ sibling; since siblings will be read in the order
      // they were written out, when the next sibling is read it will find the mapping.
      if (sBR.m_iNext) parentMap.insert(pair<int,CPPMnode *>(sBR.m_iNext,parent));
    }
    
    //if the node has children, record for the benefit of the first child
    // this node's address...(said child will be the first one read)
    if (sBR.m_iChild) parentMap.insert(pair<int,CPPMnode *>(sBR.m_iChild, pCurrent));
    
    if(!bStarted) {
      m_pRoot = pCurrent;
      bStarted = true;
    }
  }

  oInputFile.close();

  return false;
}

CPPMLanguageModel::CPPMnode *CPPMLanguageModel::GetAddress(int iIndex, std::map<int, CPPMnode*> *pMap) {
  if (iIndex==0) return NULL;
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
