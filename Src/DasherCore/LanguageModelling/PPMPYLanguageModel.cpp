// PPMPYLanguageModel.cpp
//
// Mandarin character - py prediction by a extension in PPM (subtrees attached to Symbol nodes)
//
// Started from a replicate of PPMLanguageModel
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"
#include "PPMPYLanguageModel.h"
#include "LanguageModel.h"
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

CPPMPYLanguageModel::CPPMPYLanguageModel(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CAlphInfo *pAlph, const CAlphInfo *pPyAlphabet)
  :CLanguageModel(pEventHandler, pSettingsStore, pAlph), m_iMaxOrder(2), NodesAllocated(0), m_NodeAlloc(8192), m_ContextAlloc(1024), m_pPyAlphabet(pPyAlphabet){
  m_pRoot = m_NodeAlloc.Alloc();
  m_pRoot->sym = -1;
  //  m_pRoot->child.resize(DIVISION, NULL);
  //  m_pRoot->pychild.resize(DIVISION, NULL);



  m_pRootContext = m_ContextAlloc.Alloc();
  m_pRootContext->head = m_pRoot;
  m_pRootContext->order = 0;

  m_iAlphSize = GetSize();
  //  std::cout<<"Alphaunit: "<<UNITALPH<<std::endl;
  
  // Cache the result of update exclusion - otherwise we have to look up a lot when training, which is slow

  // FIXME - this should be a boolean parameter

  bUpdateExclusion = ( GetLongParameter(LP_LM_UPDATE_EXCLUSION) !=0 );
}

/////////////////////////////////////////////////////////////////////

CPPMPYLanguageModel::~CPPMPYLanguageModel() {
}

/////////////////////////////////////////////////////////////////////
// Get the probability distribution at the context

//ACL This is Will's original GetProbs() method - AFAICT unused, as the only
// potential call site tested for MandarinDasher and if so explicitly called
// the old GetPYProbs instead (!)...so have renamed latter to Get Probs instead...
/*void CPPMPYLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) const {
  const CPPMPYContext *ppmcontext = (const CPPMPYContext *)(context);

  //  DASHER_ASSERT(m_setContexts.count(ppmcontext) > 0);

  //  std::cout<<"size of context set  "<<m_setContexts.size()<<std::endl;

  int iNumSymbols = GetSize();
  
  probs.resize(iNumSymbols);

  std::vector < bool > exclusions(iNumSymbols);
  
  unsigned int iToSpend = norm;
  unsigned int iUniformLeft = iUniform;

  // TODO: Sort out zero symbol case
  probs[0] = 0;
  exclusions[0] = false;

  int i;
  for(i = 1; i < iNumSymbols; i++) {
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

  CPPMPYnode *pTemp = ppmcontext->head;

  while(pTemp != 0) {
    int iTotal = 0;

    CPPMPYnode *pSymbol;
    for(i =0; i<DIVISION; i++){
      // std::cout<<"I "<<i<<std::endl;
      pSymbol = pTemp->child[i];
      //std::cout<<"Symbols "<<pSymbol->symbol<<std::endl;
      while(pSymbol) {
	int sym = pSymbol->symbol;

	if(!(exclusions[sym] && doExclusion))
	  iTotal += pSymbol->count;
	pSymbol = pSymbol->next;
      }
    }

    if(iTotal) {
      unsigned int size_of_slice = iToSpend;

      for(i=0; i<DIVISION; i++){
	pSymbol = pTemp->child[i];
	while(pSymbol) {
	  if(!(exclusions[pSymbol->symbol] && doExclusion)) {
	    exclusions[pSymbol->symbol] = 1;
	    
	    unsigned int p = static_cast < myint > (size_of_slice) * (100 * pSymbol->count - beta) / (100 * iTotal + alpha);
	    
	    if((pSymbol->symbol>-1)&&(pSymbol->symbol<GetSize())){
	      probs[pSymbol->symbol] += p;
	      iToSpend -= p;
	    }
	  }
	  //                              Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);      
	  //                              DebugOutput(debug);
	  pSymbol = pSymbol->next;
	}
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
}*/

void CPPMPYLanguageModel::GetPartProbs(Context context, std::vector<pair<symbol, unsigned int> > &vChildren, int norm, int iUniform){
  DASHER_ASSERT(!vChildren.empty());
  
  if(vChildren.size() == 1){
    vChildren[0].second = norm;
    return;
  }
  //  std::cout<<"Norms is "<<norm<<std::endl;
  //  std::cout<<"iUniform is "<<iUniform<<std::endl;

  const CPPMPYContext *ppmcontext = (const CPPMPYContext *)(context);

  //  DASHER_ASSERT(m_setContexts.count(ppmcontext) > 0);

  //  std::cout<<"size of context set  "<<m_setContexts.size()<<std::endl;
  
  //  probs.resize(iNumSymbols);
  
  unsigned int iToSpend = norm;
  unsigned int iUniformLeft = iUniform;

  // TODO: Sort out zero symbol case
  
  //Reproduce iterative calculations with SCENode trie

  //ACL following loop distributes the part of the probability mass assigned the uniform distribution.
  // In Will's code, it assigned 0 to the first entry, then split evenly among the rest...seems wrong?!
  int i=0;
  for (std::vector<pair<symbol, unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    DASHER_ASSERT(it->first > -1 && it->first <= m_iAlphSize);
    it->second = iUniformLeft / (vChildren.size() - i);
      //  std::cout<<"iUniformLeft: "<<iUniformLeft<<std::endl;
    iUniformLeft -= it->second;
    iToSpend -= it->second;
    i++;
  }

  DASHER_ASSERT(iUniformLeft == 0);

  int alpha = GetLongParameter( LP_LM_ALPHA );
  int beta = GetLongParameter( LP_LM_BETA );

  std::vector<CPPMPYnode *> vNodeStore;

  //new code
  for (CPPMPYnode *pTemp = ppmcontext->head; pTemp; pTemp=pTemp->vine) {
    int iTotal =0;
    vNodeStore.clear();
    for (std::vector<pair<symbol, unsigned int> >::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {

      if (CPPMPYnode *pFound = pTemp->find_symbol(it->first)) {
        iTotal += pFound->count;
        vNodeStore.push_back(pFound);
      } else
        vNodeStore.push_back(NULL);
    }
    

    //    std::cout<<"after lan mod firs tloop"<<std::endl;
    if(iTotal){
      unsigned int size_of_slice = iToSpend;
      
      int i=0;
      for (vector<pair<symbol, unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
        if(vNodeStore[i]) {
          unsigned int p = static_cast < myint > (size_of_slice) * (100 * vNodeStore[i]->count - beta) / (100 * iTotal + alpha);
          it->second += p;
          iToSpend -= p;
        }
        i++;
      }
    }
  }
  //code
  //std::cout<<"after lan mod second loop"<<std::endl;

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
  //std::cout<<"after lan mod third loop"<<std::endl;

  //allow for rounding error by distributing the leftovers evenly amongst all elements...
  // (ACL: previous code assigned nothing to element. Why? - I'm guessing due to confusion
  //  with other LM code where the first element of the probability array was a dummy,
  // storing 0 probability mass assigned to the 'root symbol' - not the case here!)
  unsigned int p = iToSpend / vChildren.size();
  for (std::vector<pair<symbol, unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    it->second += p;
    iToSpend -= p;
  }
  // std::cout<<"after lan mod fourth loop"<<std::endl;
  int iLeft = vChildren.size()-1;

  //  std::cout<<"iNumsyjbols "<<vChildren.size()<<std::endl;

  for (std::vector<pair<symbol, unsigned int> >::iterator it = vChildren.begin()+1; it!=vChildren.end(); it++) {

    //     std::cout<<"iLeft "<<iLeft<<std::endl;
    //  std::cout<<"iToSpend "<<iToSpend<<std::endl;
    unsigned int p = iToSpend / iLeft;
    it->second += p;
    --iLeft;
    iToSpend -= p;
  }

  //std::cout<<"after lan mod fifth loop"<<std::endl;
  DASHER_ASSERT(iToSpend == 0);
  //std::cout<<"after lan mod assert?"<<std::endl;

}

//ACL this was Will's original "GetPYProbs" method - explicitly called instead of GetProbs
// by an explicit cast to PPMPYLanguageModel whenever MandarinDasher was activated. Renaming
// to GetProbs causes the normal (virtual) call to come straight here without any special-casing...
void CPPMPYLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) const {
  const CPPMPYContext *ppmcontext = (const CPPMPYContext *)(context);


  //  std::cout<<"PPMCONTEXT symbol: "<<ppmcontext->head->symbol<<std::endl;
  /*
   CPPMPYnode * pNode = m_pRoot->child;

    while(pNode){
    std::cout<<"Next Symbol: "<<pNode->symbol<<"   ";
    pNode = pNode->next;
  }
    std::cout<<" "<<std::endl;
  */
  //  DASHER_ASSERT(m_setContexts.count(ppmcontext) > 0);

  int iNumSymbols = m_pPyAlphabet->GetNumberTextSymbols()+1;
  
  probs.resize(iNumSymbols);

  std::vector < bool > exclusions(iNumSymbols);
  
  unsigned int iToSpend = norm;
  unsigned int iUniformLeft = iUniform;

  // TODO: Sort out zero symbol case
  probs[0] = 0;
  exclusions[0] = false;

  int i;
  for(i = 1; i < iNumSymbols; i++) {
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

  CPPMPYnode *pTemp = ppmcontext->head;

  while(pTemp != 0) {
    int iTotal = 0;

    for (map<symbol, unsigned short int>::iterator it=pTemp->pychild.begin(); it!=pTemp->pychild.end(); it++) {
      if(!(exclusions[it->first] && doExclusion))
        iTotal += it->second;
    }

    if(iTotal) {
      unsigned int size_of_slice = iToSpend;
      
      for (map<symbol, unsigned short int>::iterator it = pTemp->pychild.begin(); it!=pTemp->pychild.end(); it++) {
        if(!(exclusions[it->first] && doExclusion)) {
          exclusions[it->first] = 1;
	    
          unsigned int p = static_cast < myint > (size_of_slice) * (100 * it->second - beta) / (100 * iTotal + alpha);
	    
          probs[it->first] += p;
          iToSpend -= p;
        }
	  //                              Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);      
	  //                              DebugOutput( debug);
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

/////////////////////////////////////////////////////////////////////
// Update context with symbol 'Symbol'

void CPPMPYLanguageModel::EnterSymbol(Context c, int Symbol) {
  if(Symbol<0)
    return;

  DASHER_ASSERT(Symbol >= 0 && Symbol < GetSize());

  CPPMPYLanguageModel::CPPMPYContext & context = *(CPPMPYContext *) (c);

  CPPMPYnode *find;

  while(context.head) {

    //std::cout<<"Max Order: "<<m_iMaxOrder<<std::endl;
    if(context.order < m_iMaxOrder) {   // Only try to extend the context if it's not going to make it too long
      find = context.head->find_symbol(Symbol);
      if(find) {
	//	std::cout<<"FOund PPM Node for update!"<<std::endl;
        context.order++;
        context.head = find;
        //      Usprintf(debug,TEXT("found context %x order %d\n"),head,order);
        //      DebugOutput(debug);

        //      std::cout << context.order << std::endl;
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

void CPPMPYLanguageModel::LearnSymbol(Context c, int Symbol) {
  if(Symbol==0)
    return;
  
  DASHER_ASSERT(Symbol >= 0 && Symbol < GetSize());
  CPPMPYLanguageModel::CPPMPYContext & context = *(CPPMPYContext *) (c);
  CPPMPYnode *n = AddSymbolToNode(context.head, Symbol);
  DASHER_ASSERT(n == context.head->find_symbol(Symbol));
  context.head = n;
  context.order++;
  //m_iMaxOrder = LanguageModelParams()->GetValue(std::string("LMMaxOrder"));
  m_iMaxOrder = 2;//GetLongParameter( LP_LM_MAX_ORDER );
  //std::cout<<"Max Order: "<<m_iMaxOrder<<std::endl;
  while(context.order > m_iMaxOrder) {
    context.head = context.head->vine;
    context.order--;
  }
}

//Do _not_ move on the context...
void CPPMPYLanguageModel::LearnPYSymbol(Context c, int pysym) {
  // Ignore attempts to add the root symbol
  if(pysym==0)
    return;

  DASHER_ASSERT(pysym > 0 && pysym <= m_pPyAlphabet->GetNumberTextSymbols());
  CPPMPYLanguageModel::CPPMPYContext & context = *(CPPMPYContext *) (c);
 
  //  std::cout<<"py learn context : "<<context.head->symbol<<std::endl;
  /*   CPPMPYnode * pNode = m_pRoot->child;

     while(pNode){
    std::cout<<"Next Symbol: "<<pNode->symbol<<"   ";
    pNode = pNode->next;
    }
     std::cout<<" "<<std::endl;
  */

  for (CPPMPYnode *pNode = context.head; pNode; pNode=pNode->vine) {
    if (++pNode->pychild[pysym]>1) {
      //sym was already present
      if (bUpdateExclusion) break;
    }
  }
  
  //no context order increase
  //context.order++;
}

void CPPMPYLanguageModel::dumpSymbol(int sym) {
  if((sym <= 32) || (sym >= 127))
    printf("<%d>", sym);
  else
    printf("%c", sym);
}

void CPPMPYLanguageModel::dumpString(char *str, int pos, int len)
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

void CPPMPYLanguageModel::dumpTrie(CPPMPYLanguageModel::CPPMPYnode *t, int d)
        // diagnostic display of the PPM trie from node t and deeper
{
//TODO
/*
	dchar debug[256];
	int sym;
	CPPMPYnode *s;
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

void CPPMPYLanguageModel::dump()
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
/// PPMPYnode definitions 
////////////////////////////////////////////////////////////////////////

CPPMPYLanguageModel::CPPMPYnode * CPPMPYLanguageModel::CPPMPYnode::find_symbol(int sym) const
// see if symbol is a child of node
{
  //  printf("finding symbol %d at node %d\n",sym,node->id);

  //Potentially replace with large scale find algorithm, necessary?
  for (CPPMPYnode * found = child[ min(DIVISION-1, sym/UNITALPH) ]; found; found=found->next) {
    if(found->sym == sym) {
      return found;
    }
  }

  return 0;
}

CPPMPYLanguageModel::CPPMPYnode * CPPMPYLanguageModel::AddSymbolToNode(CPPMPYnode *pNode, int sym) {
  //  std::cout<<"Addnode sym "<<sym<<std::endl;
  CPPMPYnode *pReturn = pNode->find_symbol(sym);

  //  std::cout << sym << ",";

  if(pReturn != NULL) {
    //      std::cout << "Using existing node" << std::endl;
    pReturn->count++;
    if (!bUpdateExclusion) {
      //update vine contexts too. Must exist if higher-order context does!
      for (CPPMPYnode *v = pReturn->vine; v; v=v->vine) {
        DASHER_ASSERT(v==m_pRoot || v->sym == sym);
        v->count++;
      }
    }
  } else {
    //symbol does not exist at this level
    pReturn = m_NodeAlloc.Alloc();        // count is initialized to 1 but no symbol or vine ptr
    ++NodesAllocated;
    pReturn->sym = sym;
    const int childIdx( min(DIVISION-1, sym/UNITALPH) );
    pReturn->next = pNode->child[childIdx];
    pNode->child[childIdx] = pReturn;
    pReturn->vine = (pNode == m_pRoot) ? m_pRoot : AddSymbolToNode(pNode->vine, sym);
  }

  return pReturn;
}

//Mandarin - PY not enabled for these read-write functions
bool CPPMPYLanguageModel::WriteToFile(std::string strFilename) {
  return false;
}

//Mandarin - PY not enabled for these read-write functions
bool CPPMPYLanguageModel::ReadFromFile(std::string strFilename) {
  return false;
}
