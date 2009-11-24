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
#include "SymbolAlphabet.h"
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

CPPMPYLanguageModel::CPPMPYLanguageModel(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CSymbolAlphabet &SymbolAlphabet, const CSymbolAlphabet &pySymbolAlphabet)
  :CLanguageModel(pEventHandler, pSettingsStore, SymbolAlphabet), m_iMaxOrder(2), NodesAllocated(0), m_NodeAlloc(8192), m_ContextAlloc(1024), m_pyAlphabet(pySymbolAlphabet){
  m_pRoot = m_NodeAlloc.Alloc();
  m_pRoot->symbol = -1;
  //  m_pRoot->child.resize(DIVISION, NULL);
  //  m_pRoot->pychild.resize(DIVISION, NULL);



  m_pRootContext = m_ContextAlloc.Alloc();
  m_pRootContext->head = m_pRoot;
  m_pRootContext->order = 0;

  m_iAlphSize = SymbolAlphabet.GetSize();
  //  std::cout<<"Alphaunit: "<<UNITALPH<<std::endl;
  m_iPYAlphSize = pySymbolAlphabet.GetSize();
  //  std::cout<<"PYunit: "<<UNITPY<<std::endl;
  
  // Cache the result of update exclusion - otherwise we have to look up a lot when training, which is slow

  // FIXME - this should be a boolean parameter

  bUpdateExclusion = ( GetLongParameter(LP_LM_UPDATE_EXCLUSION) !=0 );
}

/////////////////////////////////////////////////////////////////////

CPPMPYLanguageModel::~CPPMPYLanguageModel() {
}

/////////////////////////////////////////////////////////////////////
// Get the probability distribution at the context

void CPPMPYLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) const {
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
}

void CPPMPYLanguageModel::GetPartProbs(Context context, const std::vector<SCENode *> &vChildren, int norm, int iUniform){
  
  
  if(vChildren.size() == 1){
    vChildren[0]->NodeSize = norm;
    return;
  }
  //  std::cout<<"Norms is "<<norm<<std::endl;
  //  std::cout<<"iUniform is "<<iUniform<<std::endl;

  const CPPMPYContext *ppmcontext = (const CPPMPYContext *)(context);

  //  DASHER_ASSERT(m_setContexts.count(ppmcontext) > 0);

  //  std::cout<<"size of context set  "<<m_setContexts.size()<<std::endl;
  
  //  probs.resize(iNumSymbols);

  // Leave the vector treatment for now
  std::vector < bool > exclusions(vChildren.size());
  
  unsigned int iToSpend = norm;
  unsigned int iUniformLeft = iUniform;

  // TODO: Sort out zero symbol case
  exclusions[0] = false;
  
  //Reproduce iterative calculations with SCENode trie

  if(vChildren.size()){
    vChildren[0] -> NodeSize = 0;
    int i=1;
    for (std::vector<SCENode *>::const_iterator it = vChildren.begin()+1; it!=vChildren.end(); it++) {
      SCENode *pNode(*it);
      pNode->NodeSize = iUniformLeft / (vChildren.size() - i);
      //  std::cout<<"iUniformLeft: "<<iUniformLeft<<std::endl;
      iUniformLeft -= pNode->NodeSize;
      iToSpend -= pNode->NodeSize;
      exclusions[i] = false;
      i++;
    }
  }

  DASHER_ASSERT(iUniformLeft == 0);

  //  bool doExclusion = GetLongParameter( LP_LM_ALPHA );
  bool doExclusion = 0; //FIXME

  int alpha = GetLongParameter( LP_LM_ALPHA );
  int beta = GetLongParameter( LP_LM_BETA );

  CPPMPYnode *pTemp = ppmcontext->head;
  CPPMPYnode *pFound;
  std::vector<CPPMPYnode *> vNodeStore;

  //new code
  while(pTemp!=0){
    int iTotal =0;
    vNodeStore.clear();
    int i=0;
    for (std::vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
      SCENode *pNode(*it);

      pFound = pTemp->find_symbol(pNode->Symbol);
      //Mark: do we need to treat the exception of -1 separately?     
      if((!(exclusions[i] && doExclusion))&&(pFound)){
	  iTotal += pFound->count;
	  vNodeStore.push_back(pFound);
      }
      else
	vNodeStore.push_back(NULL);
	
      i++;
    }
    

    //    std::cout<<"after lan mod firs tloop"<<std::endl;
    if(iTotal){
      unsigned int size_of_slice = iToSpend;
      
      int i=0;
      for (vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
        SCENode *pNode(*it);
        if((!(exclusions[i] && doExclusion))&&(vNodeStore[i])) {
          exclusions[i] = 1;
          unsigned int p = static_cast < myint > (size_of_slice) * (100 * vNodeStore[i]->count - beta) / (100 * iTotal + alpha);
          if((pNode->Symbol>-1)&&(pNode->Symbol<=m_iAlphSize)){
            pNode->NodeSize += p;
            iToSpend -= p;
          }
        }
        i++;
      }
    }
    pTemp = pTemp->vine;
  }
  //code
  //std::cout<<"after lan mod second loop"<<std::endl;

  
  unsigned int size_of_slice = iToSpend;
  int symbolsleft = 0;

  for(unsigned int i = 1; i < vChildren.size(); i++)
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
  //std::cout<<"after lan mod third loop"<<std::endl;
  
  if(vChildren.size()) {
    int i=1;
    for (std::vector<SCENode *>::const_iterator it = vChildren.begin()+1; it!=vChildren.end(); it++) {
      if(!(exclusions[i] && doExclusion)) {
        unsigned int p = size_of_slice / symbolsleft;
        (*it)->NodeSize += p;
        iToSpend -= p;
      }
      i++;
    }
  }
  // std::cout<<"after lan mod fourth loop"<<std::endl;
  int iLeft = vChildren.size()-1;

  if(vChildren.size()) {
    //  std::cout<<"iNumsyjbols "<<vChildren.size()<<std::endl;

    for (std::vector<SCENode *>::const_iterator it = vChildren.begin()+1; it!=vChildren.end(); it++) {

      //     std::cout<<"iLeft "<<iLeft<<std::endl;
      //  std::cout<<"iToSpend "<<iToSpend<<std::endl;
      unsigned int p = iToSpend / iLeft;
      (*it)->NodeSize += p;
      --iLeft;
      iToSpend -= p;
    }
  }

  //std::cout<<"after lan mod fifth loop"<<std::endl;
  DASHER_ASSERT(iToSpend == 0);
  //std::cout<<"after lan mod assert?"<<std::endl;

}


void CPPMPYLanguageModel::GetPYProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) {
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

  int iNumSymbols = m_pyAlphabet.GetSize();
  
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
    for(i=0; i<DIVISION; i++){
      pSymbol  = pTemp->pychild[i];
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
	pSymbol = pTemp->pychild[i];
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


void CPPMPYLanguageModel::AddSymbol(CPPMPYLanguageModel::CPPMPYContext &context, int sym)
        // add symbol to the context
        // creates new nodes, updates counts
        // and leaves 'context' at the new context
{
  // Ignore attempts to add the root symbol

  if(sym==0)
    return;

  DASHER_ASSERT(sym >= 0 && sym < GetSize());

  CPPMPYnode *vineptr, *temp;
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
  m_iMaxOrder = 2;//GetLongParameter( LP_LM_MAX_ORDER );
  //std::cout<<"Max Order: "<<m_iMaxOrder<<std::endl;
  while(context.order > m_iMaxOrder) {
    context.head = context.head->vine;
    context.order--;
  }
}

void CPPMPYLanguageModel::AddPYSymbol(CPPMPYLanguageModel::CPPMPYContext &context, int pysym)
        // add symbol to the context
        // creates new nodes, updates counts
        // and leaves 'context' at the new context
{
  // Ignore attempts to add the root symbol

  if(pysym==0)
    return;

  DASHER_ASSERT(pysym >= 0 && pysym < m_pyAlphabet.GetSize());

  CPPMPYnode *vineptr, *temp, *pytail;
  int updatecnt = 1;


  //update of vine pointers similar to old PPMPYnodes
  temp = context.head->vine;
  pytail = AddPYSymbolToNode(context.head, pysym, &updatecnt);
  vineptr = pytail;

  //no context order increase
  //context.order++;

  while(temp != 0) {
    vineptr->vine = AddPYSymbolToNode(temp, pysym, &updatecnt);
    vineptr = vineptr->vine;
    temp = temp->vine;
  }

  //the py tree attached to root should have vine pointers NULL
  vineptr->vine = NULL;

  //m_iMaxOrder = LanguageModelParams()->GetValue(std::string("LMMaxOrder"));
  /*  m_iMaxOrder = GetLongParameter( LP_LM_MAX_ORDER );

  while(context.order > m_iMaxOrder) {
    context.head = context.head->vine;
    context.order--;*/
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
  AddSymbol(context, Symbol);
}

void CPPMPYLanguageModel::LearnPYSymbol(Context c, int Symbol) {
  if(Symbol==0)
    return;
  

  DASHER_ASSERT(Symbol >= 0 && Symbol < m_pyAlphabet.GetSize());
  CPPMPYLanguageModel::CPPMPYContext & context = *(CPPMPYContext *) (c);
 
  //  std::cout<<"py learn context : "<<context.head->symbol<<std::endl;
  /*   CPPMPYnode * pNode = m_pRoot->child;

     while(pNode){
    std::cout<<"Next Symbol: "<<pNode->symbol<<"   ";
    pNode = pNode->next;
    }
     std::cout<<" "<<std::endl;
  */
     AddPYSymbol(context, Symbol);
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
  CPPMPYnode * found = NULL;
  bool bFound = 0;

  for (int i=0; i<DIVISION-1; i++){
    if(sym<(i+1)*UNITALPH){
      //      std::cout<<"before"<<std::endl;
      found = child[i];
      //  std::cout<<"after child: "<<i<<std::endl;
      //      std::cout<<"i "<<i<<std::endl;
      //   std::cout<<"sym "<<sym<<std::endl;
      bFound =1;
      break;
    }
  }
  
  if(!bFound){
    found = child[DIVISION-1];

    //    std::cout<<"in last group "<<std::endl;
  }

  //  std::cout<<"here?"<<std::endl;
  while(found) {
    if(found->symbol == sym){
      //  std::cout<<"Found!"<<std::endl;
      return found;
    }
    found = found->next;
    //  std::cout<<"next successful"<<std::endl;
    // if((found)){

    //   std::cout<<"next "<<found->symbol<<std::endl;
    //  if(found->next)
    //	std::cout<<"next not empty"<<std::endl;
    // }
    // else
    //std::cout<<"found is NULL"<<std::endl;
  }

  //  std::cout<<"end"<<std::endl;
  return 0;
}

// New find pysymbol function, to find the py symbol in nodes attached to character node
CPPMPYLanguageModel::CPPMPYnode * CPPMPYLanguageModel::CPPMPYnode::find_pysymbol(int pysym) const
// see if pysymbol is a child of node
{

  CPPMPYnode * found = NULL;
  bool bFound = 0;

  for (int i=0; i<DIVISION-1; i++){
    if(pysym<(i+1)*UNITPY){
      found = pychild[i];
      bFound = 1;
      break;
    }
  }
  
  if(!bFound)
    found = pychild[DIVISION-1];

  while(found) {
    if(found->symbol == pysym){
      return found;
    }
    found = found->next;
  }
  return 0;
}

CPPMPYLanguageModel::CPPMPYnode * CPPMPYLanguageModel::AddSymbolToNode(CPPMPYnode *pNode, int sym, int *update) {
  //  std::cout<<"Addnode sym "<<sym<<std::endl;
  CPPMPYnode *pReturn = pNode->find_symbol(sym);

  //  std::cout << sym << ",";

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
  //  pReturn->child.resize(DIVISION, NULL);
  //  pReturn->pychild.resize(DIVISION, NULL);

  bool bFound =0;
  for (int i=0; i<DIVISION-1; i++){
    if(sym<(i+1)*UNITALPH){
      pReturn->next = pNode->child[i];
      pNode->child[i]=pReturn;
      bFound =1;
      break;
    }
  }

  if(!bFound){
    pReturn->next = pNode->child[DIVISION-1];
    pNode->child[DIVISION-1]=pReturn;  
  }

  ++NodesAllocated;

  return pReturn;

}

CPPMPYLanguageModel::CPPMPYnode * CPPMPYLanguageModel::AddPYSymbolToNode(CPPMPYnode *pNode, int pysym, int *update) {
  CPPMPYnode *pReturn = pNode->find_pysymbol(pysym);

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
  pReturn->symbol = pysym;
  //  pReturn->child.resize(DIVISION, NULL);
  //  pReturn->pychild.resize(DIVISION, NULL);

  bool bFound =0;
  
  for (int i=0; i<DIVISION-1; i++){
    if(pysym<(i+1)*UNITPY){
      pReturn->next = pNode->pychild[i];
      pNode->pychild[i]=pReturn;
      bFound = 1;
      break;
    }
  }

  if(!bFound){
    pReturn->next = pNode->pychild[DIVISION-1];
    pNode->pychild[DIVISION-1]=pReturn;  
  }

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

bool CPPMPYLanguageModel::WriteToFile(std::string strFilename) {

  std::cout<<"WRITE TO FILE USED?"<<std::endl;

  std::map<CPPMPYnode *, int> mapIdx;
  int iNextIdx(1); // Index of 0 means NULL;

  std::ofstream oOutputFile(strFilename.c_str());

  RecursiveWrite(m_pRoot, &mapIdx, &iNextIdx, &oOutputFile);

  oOutputFile.close();

  return false;
};

//Mandarin - PY not enabled for these read-write functions
bool CPPMPYLanguageModel::RecursiveWrite(CPPMPYnode *pNode, std::map<CPPMPYnode *, int> *pmapIdx, int *pNextIdx, std::ofstream *pOutputFile) {

  // Dump node here

  BinaryRecord sBR;

  sBR.m_iIndex = GetIndex(pNode, pmapIdx, pNextIdx); 
  //Note future changes here:
  sBR.m_iChild = GetIndex(pNode->child[0], pmapIdx, pNextIdx); 
  sBR.m_iNext = GetIndex(pNode->next, pmapIdx, pNextIdx); 
  sBR.m_iVine = GetIndex(pNode->vine, pmapIdx, pNextIdx);
  sBR.m_iCount = pNode->count;
  sBR.m_iSymbol = pNode->symbol;

  pOutputFile->write(reinterpret_cast<char*>(&sBR), sizeof(BinaryRecord));

  CPPMPYnode *pCurrentChild(pNode->child[0]);
  
  while(pCurrentChild != NULL) {
    RecursiveWrite(pCurrentChild, pmapIdx, pNextIdx, pOutputFile);
    pCurrentChild = pCurrentChild->next;
  }

  return true;
};

int CPPMPYLanguageModel::GetIndex(CPPMPYnode *pAddr, std::map<CPPMPYnode *, int> *pmapIdx, int *pNextIdx) {
  std::cout<<"GetIndex gets called?"<<std::endl;
  int iIndex;
  if(pAddr == NULL)
    iIndex = 0;
  else {
    std::map<CPPMPYnode *, int>::iterator it(pmapIdx->find(pAddr));
    
    if(it == pmapIdx->end()) {
      iIndex = *pNextIdx;
      pmapIdx->insert(std::pair<CPPMPYnode *, int>(pAddr, iIndex));
      ++(*pNextIdx);
    }
    else {
      iIndex = it->second;
    }
  }
  return iIndex;
};


//Mandarin - PY not enabled for these read-write functions
bool CPPMPYLanguageModel::ReadFromFile(std::string strFilename) {
  
  std::ifstream oInputFile(strFilename.c_str());
  std::map<int, CPPMPYnode*> oMap;
  BinaryRecord sBR;
  bool bStarted(false);

  while(!oInputFile.eof()) {
    oInputFile.read(reinterpret_cast<char *>(&sBR), sizeof(BinaryRecord));

    CPPMPYnode *pCurrent(GetAddress(sBR.m_iIndex, &oMap));
    //Note future changes here:
    pCurrent->child[0] = GetAddress(sBR.m_iChild, &oMap);
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

CPPMPYLanguageModel::CPPMPYnode *CPPMPYLanguageModel::GetAddress(int iIndex, std::map<int, CPPMPYnode*> *pMap) {

  std::cout<<"Get Address gets called?"<<std::endl;
  std::map<int, CPPMPYnode*>::iterator it(pMap->find(iIndex));

  if(it == pMap->end()) {
    CPPMPYnode *pNewNode;
    pNewNode = m_NodeAlloc.Alloc();
    pMap->insert(std::pair<int, CPPMPYnode*>(iIndex, pNewNode));
    return pNewNode;
  }
  else {
    return it->second;
  }
}
