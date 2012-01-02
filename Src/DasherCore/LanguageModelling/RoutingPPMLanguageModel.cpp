//
//  RoutingPPMLanguageModel.cpp
//  Dasher
//
//  Created by Alan Lawrence on 13/12/11.
//  Copyright 2011 Cambridge University. All rights reserved.
//

#include "RoutingPPMLanguageModel.h"

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

CRoutingPPMLanguageModel::CRoutingPPMLanguageModel(CSettingsUser *pCreator, const vector<symbol> *pBaseSyms, const vector<set<symbol> > *pRoutes, bool bRoutesContextSensitive)
:CAbstractPPM(pCreator, pRoutes->size()-1, new CRoutingPPMnode(-1), GetLongParameter(LP_LM_MAX_ORDER)), NodesAllocated(0), m_NodeAlloc(8192), m_pBaseSyms(pBaseSyms), m_pRoutes(pRoutes), m_bRoutesContextSensitive(bRoutesContextSensitive) {
  DASHER_ASSERT(pBaseSyms->size() >= pRoutes->size());
}

void CRoutingPPMLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) const {
  const CPPMContext *ppmcontext = (const CPPMContext *)(context);

  const int iNumSymbols(m_pBaseSyms->size()); //i.e., the #routes - so loop from i=1 to <iNumSymbols
  probs.resize(iNumSymbols);
  
  unsigned int iToSpend = norm;
  unsigned int iUniformLeft = iUniform;
  
  // TODO: Sort out zero symbol case
  probs[0] = 0;
  
  for(int i = 1; i < iNumSymbols; i++) {
    probs[i] = iUniformLeft / (iNumSymbols - i);
    iUniformLeft -= probs[i];
    iToSpend -= probs[i];
  }
  
  DASHER_ASSERT(iUniformLeft == 0);
  
  int alpha = GetLongParameter( LP_LM_ALPHA );
  int beta = GetLongParameter( LP_LM_BETA );

  //first, fill out the probabilities of the base symbols, as per ordinary PPM
  // (TODO, could move CPPMLanguageModel::GetProbs into CAbstractPPM, would do
  // this for us?)
  vector<unsigned int> baseProbs(GetSize()); //i.e. # base symbols
  for (CPPMnode *pTemp = ppmcontext->head; pTemp; pTemp = pTemp->vine) {
    int iTotal = 0;
    for (ChildIterator it=pTemp->children(); it!=pTemp->end(); it++)
      iTotal += (*it)->count;
    
    if(iTotal) {
      unsigned int size_of_slice = iToSpend;
      
      for (ChildIterator it=pTemp->children(); it!=pTemp->end(); it++) {
        unsigned int p = static_cast < myint > (size_of_slice) * (100 * (*it)->count - beta) / (100 * iTotal + alpha);
          
        baseProbs[(*it)->sym] += p;
        iToSpend -= p;
      
        //                              Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);      
        //                              DebugOutput( debug);
      }
    } 
  }
  
  //anything left over, distribute evenly...
  for (int i=1,iLeft=GetSize()-1; i<GetSize(); i++,iLeft--) {
    unsigned int p = iToSpend / iLeft;
    baseProbs[i] += p;
    iToSpend -= p;
  }
  DASHER_ASSERT(iToSpend == 0);
  
  //second, use those figures as the _total_ to divide up between the routes
  // _for_each_base_symbol_.
  for (CPPMnode *pTemp = ppmcontext->head; pTemp; pTemp=pTemp->vine) {
    if (pTemp!=m_pRoot && !m_bRoutesContextSensitive) continue;

    for (ChildIterator it = pTemp->children(); it!=pTemp->end(); it++) {
      const CRoutingPPMnode *pNode(static_cast<CRoutingPPMnode*>(*it));
      int iTotal=0; //total for base symbol corresponding to child (at this level of PPM tree)
      for (map<symbol,unsigned short int>::const_iterator it2=pNode->m_routes.begin(); it2!=pNode->m_routes.end(); it2++)
        iTotal += it2->second;
      if (iTotal) {
        //divvy up some of baseProbs according to the distribution
        // of pNode->m_routes
        unsigned int size_of_slice = baseProbs[pNode->sym];
        for (map<symbol,unsigned short int>::const_iterator it2=pNode->m_routes.begin(); it2!=pNode->m_routes.end(); it2++) {
          unsigned int p = size_of_slice * (100 * it2->second - beta) / (100*iTotal + alpha);
          probs[it2->first] += p;
          baseProbs[pNode->sym] -= p;
        }
      }
    }
  }
  
  //for each base, distribute any remaining probability mass
  // uniformly to all the routes to that base.
  for (int i=1; i<GetSize(); i++) {
    if (!baseProbs[i]) continue; //=already distributed
    
    //ok, so there's some probability mass assigned to the base symbol,
    // which we haven't assigned to any route
    const set<symbol> &routes((*m_pRoutes)[i]);
    //divide it up evenly
    int iLeft = routes.size();
    for (set<symbol>::iterator it = routes.begin(); it!=routes.end(); it++) {
      unsigned int p = baseProbs[i] / iLeft;
      probs[*it] += p;
      baseProbs[i] -= p;
      --iLeft;
    }
    DASHER_ASSERT(baseProbs[i]==0);
  }
}

/////////////////////////////////////////////////////////////////////

symbol CRoutingPPMLanguageModel::GetBestRoute(Context ctx) {
  const CPPMContext *context = (const CPPMContext *)ctx;
  DASHER_ASSERT(context->head && context->head != m_pRoot);
  
  map<symbol,unsigned int> probs; //of the routes leading to this base sym
  int iToSpend = 1<<16; //arbitrary, could be anything
  int alpha = GetLongParameter(LP_LM_ALPHA), beta=GetLongParameter(LP_LM_BETA);
  
  for (CPPMnode *pTemp = context->head; pTemp!=m_pRoot; pTemp=pTemp->vine) {
    if (pTemp->vine!=m_pRoot && !m_bRoutesContextSensitive) continue;

    const CRoutingPPMnode *node(static_cast<CRoutingPPMnode*>(pTemp));
    unsigned long iTotal=0;
    for (map<symbol,unsigned short int>::const_iterator it=node->m_routes.begin(); it!=node->m_routes.end(); it++)
      iTotal += it->second;
    if (!iTotal) continue;
    const int size_of_slice(iToSpend);
    for (map<symbol,unsigned short int>::const_iterator it=node->m_routes.begin(); it!=node->m_routes.end(); it++) {
      unsigned int p = size_of_slice * (100*it->second - beta) / (100*iTotal+ alpha);
      iToSpend-=p;
      probs[it->first]+=p;
    }
  }
  //Could divvy up rest uniformly...but there's no point, this won't affect
  // which is most likely! (Except by rounding error, i.e. if iToSpend can't
  // be divided evenly between the routes. But let's not worry about that,
  // the worst that can happen is the user ends up in a different, very-nearly-equally-sized,
  // box)
  
  pair<symbol,unsigned int> best;//initially (0,0)
  for (map<symbol, unsigned int>::iterator it=probs.begin(); it!=probs.end(); it++) {
    DASHER_ASSERT((*m_pRoutes)[context->head->sym].count(it->first));
    if (it->second>best.second) best=*it;
  }
  
  if (best.second) return best.first;
  //no data. pick one at random
  const set<symbol> &options((*m_pRoutes)[context->head->sym]);
  //in fact, (very pseudo)-random:
  return *(options.begin());
}

void CRoutingPPMLanguageModel::LearnBaseSymbol(Context c, int baseSym) {
  CAbstractPPM::LearnSymbol(c, baseSym);
}

void CRoutingPPMLanguageModel::LearnSymbol(Context ctx, int sym) {
  int base = m_pBaseSyms->at(sym);
  LearnBaseSymbol(ctx, base);
  //ctx now updated, points to node for learnt base sym
  DASHER_ASSERT((*m_pRoutes)[base].size());
  if ((*m_pRoutes)[base].size()==1) return; //no need to store, saves computation if we don't
  for (CPPMnode *node=((CPPMContext*)ctx)->head; node!=m_pRoot; node=node->vine) {
    if (node->vine!=m_pRoot && !m_bRoutesContextSensitive) continue;
    else if (static_cast<CRoutingPPMnode*>(node)->m_routes[sym]++) //returns old value, i.e. 0 if not present
      if (bUpdateExclusion) break;
  }
}

CRoutingPPMLanguageModel::CRoutingPPMnode *CRoutingPPMLanguageModel::makeNode(int sym) {
  CRoutingPPMnode *res = m_NodeAlloc.Alloc();
  res->sym=sym;
  ++NodesAllocated;
  return res;
}

//Mandarin - PY not enabled for these read-write functions
bool CRoutingPPMLanguageModel::WriteToFile(std::string strFilename) {
  return false;
}

//Mandarin - PY not enabled for these read-write functions
bool CRoutingPPMLanguageModel::ReadFromFile(std::string strFilename) {
  return false;
}
