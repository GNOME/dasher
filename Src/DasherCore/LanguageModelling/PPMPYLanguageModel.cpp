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

CPPMPYLanguageModel::CPPMPYLanguageModel(CSettingsUser *pCreator, int iNumCHsyms, int iNumPYsyms)
  :CAbstractPPM(pCreator, iNumCHsyms, new CPPMPYnode(-1), 2), NodesAllocated(0), m_NodeAlloc(8192), m_iNumPYsyms(iNumPYsyms) {
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

  const CPPMContext *ppmcontext = (const CPPMContext *)(context);

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
    DASHER_ASSERT(it->first > 0 && it->first < GetSize()); //i.e., is valid CH symbol
    it->second = iUniformLeft / (vChildren.size() - i);
      //  std::cout<<"iUniformLeft: "<<iUniformLeft<<std::endl;
    iUniformLeft -= it->second;
    iToSpend -= it->second;
    i++;
  }

  DASHER_ASSERT(iUniformLeft == 0);

  int alpha = GetLongParameter( LP_LM_ALPHA );
  int beta = GetLongParameter( LP_LM_BETA );

  int *vCounts=new int[vChildren.size()]; //num occurrences of symbol at same index in vChildren

  //new code
  for (CPPMnode *pTemp = ppmcontext->head; pTemp; pTemp=pTemp->vine) {
    int iTotal=0, i=0;
    for (std::vector<pair<symbol, unsigned int> >::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++,i++) {
      if (CPPMnode *pFound = pTemp->find_symbol(it->first)) {
        iTotal += vCounts[i] = pFound->count; //double assignment
      } else
        vCounts[i] = 0;
    }
    

    //    std::cout<<"after lan mod firs tloop"<<std::endl;
    if(iTotal){
      unsigned int size_of_slice = iToSpend;
      
      int i=0;
      for (vector<pair<symbol, unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++,i++) {
        if(vCounts[i]) {
          unsigned int p = static_cast < myint > (size_of_slice) * (100 * vCounts[i] - beta) / (100 * iTotal + alpha);
          it->second += p;
          iToSpend -= p;
        }
      }
    }
  }
  delete[] vCounts;
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
  const CPPMContext *ppmcontext = (const CPPMContext *)(context);

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

  int iNumSymbols = m_iNumPYsyms+1;
  
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

  for (CPPMnode *pTemp = ppmcontext->head; pTemp; pTemp = pTemp->vine) {
    int iTotal = 0;
    const map<symbol, unsigned short int> &pychild( static_cast<CPPMPYnode *>(pTemp)->pychild);

    for (map<symbol, unsigned short int>::const_iterator it=pychild.begin(); it!=pychild.end(); it++) {
      if(!(exclusions[it->first] && doExclusion))
        iTotal += it->second;
    }

    if(iTotal) {
      unsigned int size_of_slice = iToSpend;
      
      for (map<symbol, unsigned short int>::const_iterator it = pychild.begin(); it!=pychild.end(); it++) {
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

//Do _not_ move on the context...
void CPPMPYLanguageModel::LearnPYSymbol(Context c, int pysym) {
  // Ignore attempts to add the root symbol
  if(pysym==0)
    return;

  DASHER_ASSERT(pysym > 0 && pysym <= m_iNumPYsyms);
  CPPMPYLanguageModel::CPPMContext & context = *(CPPMContext *) (c);
 
  //  std::cout<<"py learn context : "<<context.head->symbol<<std::endl;
  /*   CPPMPYnode * pNode = m_pRoot->child;

     while(pNode){
    std::cout<<"Next Symbol: "<<pNode->symbol<<"   ";
    pNode = pNode->next;
    }
     std::cout<<" "<<std::endl;
  */

  for (CPPMnode *pNode = context.head; pNode; pNode=pNode->vine) {
    if (static_cast<CPPMPYnode *>(pNode)->pychild[pysym]++) {
      //count non-zero before increment, i.e. sym already present
      if (bUpdateExclusion) break;
    }
  }
  
  //no context order increase
  //context.order++;
}

CPPMPYLanguageModel::CPPMPYnode *CPPMPYLanguageModel::makeNode(int sym) {
  CPPMPYnode *res = m_NodeAlloc.Alloc();
  res->sym=sym;
  ++NodesAllocated;
  return res;
}

//Mandarin - PY not enabled for these read-write functions
bool CPPMPYLanguageModel::WriteToFile(std::string strFilename) {
  return false;
}

//Mandarin - PY not enabled for these read-write functions
bool CPPMPYLanguageModel::ReadFromFile(std::string strFilename) {
  return false;
}
