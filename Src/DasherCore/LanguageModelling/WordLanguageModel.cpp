// WordLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"
#include "WordLanguageModel.h"
#include "PPMLanguageModel.h"
#include "../Alphabet/AlphabetMap.h"


#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stack>

using namespace Dasher;
using namespace std;

// static TCHAR debug[256];
typedef unsigned long ulong;

#ifdef _WIN32
#define snprintf _snprintf
#endif

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

///////////////////////////////////////////////////////////////////

void CWordLanguageModel::CWordContext::dump()
        // diagnostic output
{
  // TODO uncomment this when headers sorted out
  //dchar debug[128];
  //Usprintf(debug,TEXT("head %x order %d\n"),head,order);
  //DebugOutput(debug);
}

////////////////////////////////////////////////////////////////////////
/// Wordnode definitions 
////////////////////////////////////////////////////////////////////////

/// Return the child of a node with a given symbol, or NULL if there is no child with that symbol yet

CWordLanguageModel::CWordnode* CWordLanguageModel::CWordnode::find_symbol(int sym) const {
  CWordnode *found = child;
  while(found) {
    if(found->sbl == sym)
      return found;
    found = found->next;
  }
  return 0;
}

void CWordLanguageModel::CWordnode::RecursiveDump(std::ofstream &file) {

  CWordnode *pCurrentChild(child);

  file << "\"" << this << "\" [label=\"" << this->sbl << "\\n" << this->count << "\"]" << std::endl;

  file << "\"" << this << "\" -> \"" << vine << "\" [style=dashed]" << std::endl;

  while(pCurrentChild) {
    file << "\"" << this << "\" -> \"" << pCurrentChild << "\"" << std::endl;
    pCurrentChild->RecursiveDump(file);
    pCurrentChild = pCurrentChild->next;
  }
}

CWordLanguageModel::CWordnode * CWordLanguageModel::AddSymbolToNode(CWordnode *pNode, symbol sym, int *update, bool bLearn) {

  // FIXME - need to implement bLearn

  CWordnode *pReturn = pNode->find_symbol(sym);

  if(pReturn != NULL) {
    if(*update) {

      //      std::cout << "USHRT_MAX: " << USHRT_MAX << " " << bLearn << std::endl;

      //      if( (pReturn->count < USHRT_MAX) && bLearn ) // Truncate counts at storage limit
      if(bLearn)                // Truncate counts at storage limit
        pReturn->count++;
      *update = 0;
    }
    return pReturn;
  }

  pReturn = m_NodeAlloc.Alloc();        // count is initialized to 1
  pReturn->sbl = sym;
  pReturn->next = pNode->child;
  pNode->child = pReturn;

  if(!bLearn) {
    --(pReturn->count);         // FIXME - in the long term, don't allocate
    // nodes if we're not learning, but should be
    // okay for now
  }

  //  std::cout << pReturn->count << std::endl;

  ++NodesAllocated;

  return pReturn;
}

/////////////////////////////////////////////////////////////////////
// CWordLanguageModel defs
/////////////////////////////////////////////////////////////////////

CWordLanguageModel::CWordLanguageModel(CSettingsUser *pCreator, 
				       const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap)
  :CLanguageModel(pAlph->iEnd-1), CSettingsUser(pCreator), m_iSpaceSymbol(pAlph->GetSpaceSymbol()), NodesAllocated(0),
   max_order(2), m_NodeAlloc(8192), m_ContextAlloc(1024) {
  
  // Construct a root node for the trie

  m_pRoot = m_NodeAlloc.Alloc();
  m_pRoot->sbl = -1;
  m_pRoot->count = 0;

  // Create a spelling model

  pSpellingModel = new CPPMLanguageModel(this, m_iNumSyms);

  // Construct a root context
  
  m_rootcontext = new CWordContext(m_pRoot, 0);
  
  m_rootcontext->m_pSpellingModel = pSpellingModel;
  m_rootcontext->oSpellingContext = pSpellingModel->CreateEmptyContext();

  iWordStart = 8192;

  nextid = iWordStart;          // Start of indices for words - may need to increase this for *really* large alphabets

  wordidx = 0;

}

CWordLanguageModel::~CWordLanguageModel() {

  delete m_rootcontext;
  delete pSpellingModel;

  // A non-recursive node deletion algorithm using a stack
/*	std::stack<CWordnode*> deletenodes;
	deletenodes.push(m_pRoot);
	while (!deletenodes.empty())
	{
		CWordnode* temp = deletenodes.top();
		deletenodes.pop();
		CWordnode* next;
		do	
		{
			next = temp->next;

			// push the child
			if (temp->child)
				deletenodes.push(temp->child);
			
			delete temp;

			temp=next;
			
		} while (temp !=0); 

	}*/

}

int CWordLanguageModel::lookup_word(const std::string &w) {
  if(dict[w] == 0) {
    dict[w] = nextid;
    ++nextid;
  }

  return dict[w];
}

int CWordLanguageModel::lookup_word_const(const std::string &w) const {
  std::cout << "Looking up (const) " << w << std::endl;

  return dict.find(w)->second;
}

/////////////////////////////////////////////////////////////////////
// get the probability distribution at the context

void CWordLanguageModel::GetProbs(Context context, std::vector<unsigned int> &probs, int norm, int iUniform) const {
  // Got rid of const below

  CWordLanguageModel::CWordContext * wordcontext = (CWordContext *) (context);

  // Make sure that the probability vector has the right length

  int iNumSymbols = GetSize();
  probs.resize(iNumSymbols);

  // For the prototype work with double precision to make things easier to normalise

  std::vector < double >dProbs(iNumSymbols);

  for(std::vector < double >::iterator it(dProbs.begin()); it != dProbs.end(); ++it)
    *it = 0.0;

  double alpha = GetLongParameter(LP_LM_WORD_ALPHA) / 100.0;
  //  double beta = LanguageModelParams()->GetValue( std::string( "LMBeta" ) )/100.0;

  // Ignore beta for now - we'll need to know how many different words have been seen, not just the total count.

  double dToSpend(1.0);

  CWordnode *pTmp = wordcontext->head;
  CWordnode *pTmpWord = wordcontext->word_head;

  // We'll assume that these stay in sync for now - maybe do something more robust later.

  while(pTmp) {

    // Get the total count from the word node

    int iTotal(pTmpWord->count);

    if(iTotal) {

      CWordnode *pTmpChild(pTmp->child);

      while(pTmpChild) {
        // make sure we only get child nodes which correspond
        // to symbols (not words).

        if(pTmpChild->sbl < iWordStart) {

          double dP;

          if(pTmpChild->count > 0)
            dP = dToSpend * (pTmpChild->count) / static_cast < double >(iTotal + alpha);
          else
          dP = 0.0;

          dProbs[pTmpChild->sbl] += dP;
        }

        pTmpChild = pTmpChild->next;
      }

    }

    dToSpend *= alpha / static_cast < double >(iTotal + alpha);

    pTmp = pTmp->vine;
    pTmpWord = pTmpWord->vine;

  }

  // Get probabilities from the spelling model (note we cache these for later)

  wordcontext->m_iSpellingNorm = norm;

  int iSpellingNorm(wordcontext->m_iSpellingNorm);

  wordcontext->m_pSpellingModel->GetProbs(wordcontext->oSpellingContext, wordcontext->oSpellingProbs, iSpellingNorm, 0);

  double dNorm(0.0);

  for(int i(0); i < iNumSymbols; ++i) {
    dProbs[i] += wordcontext->m_dSpellingFactor * wordcontext->oSpellingProbs[i] / static_cast < double >(wordcontext->m_iSpellingNorm);
    dNorm += dProbs[i];
  }

  // Convert back to integer representation

  int iToSpend(norm);

  for(int i(0); i < iNumSymbols; ++i) {
    probs[i] = (unsigned int) (norm * dProbs[i] / dNorm);
    iToSpend -= probs[i];
  }

  // Check that we haven't got anything left over due to rounding errors:

  int iLeft = iNumSymbols;

  for(int j = 0; j < iNumSymbols; ++j) {
    unsigned int p = iToSpend / iLeft;
    probs[j] += p;
    --iLeft;
    iToSpend -= p;
  }

  DASHER_ASSERT(iToSpend == 0);
}

/// Collapse the context. This also has the effect of entering a count
/// for the word into the word part of the model

void CWordLanguageModel::CollapseContext(CWordLanguageModel::CWordContext &context, bool bLearn) {

  // Letters appear at the end of the trie:
  // 

  if(max_order == 0) {
    // If max_order = 0 then we are not keeping track of previous
    // words, so we just collapse the letter part of the context and
    // return

    // FIXME - not sure this will work any more - don't use this
    // branch without checking that it's doing the right thing

    context.head = m_pRoot;
    context.order = 0;

  }
  else {

    std::vector < symbol > oSymbols;

    for(std::string::iterator it(context.current_word.begin()); it != context.current_word.end(); it += 4) {
      std::string fragment(it, it + 4);
      oSymbols.push_back(atoi(fragment.c_str()));
    }

    if(bLearn) {                // Only do this if we are learning
      // We need to increment all substrings - start at the current context striped back to the word level

      bool bUpdateExclusion(false);     // Whether to keep going or not

      CWordnode *pCurrent(context.word_head);

      // Keep track of pointers to all child nodes

      //      std::vector< std::vector< CWordnode* >* > oNodeCache;

      std::vector < CWordnode * >**apNodeCache;

      apNodeCache = new std::vector < CWordnode * >*[oSymbols.size()];

      for(unsigned int i(0); i < oSymbols.size(); ++i)
        apNodeCache[i] = new std::vector < CWordnode * >;

      // FIXME - remember to delete member vectors when we're done

      // FIXME broken m_pAlphabet->GetSymbols( &oSymbols, &(context.current_word), false );

      // We're not storing the actual string - just a list of symbol IDs

      while((pCurrent != NULL) && !bUpdateExclusion) {

        //      std::cout << "Incrementing" << std::endl;

        ++(pCurrent->count);

        int i(0);

        //      std::vector< CWordnode* > *pCurrentCache( new std::vector< CWordnode* > );

        CWordnode *pTmp(pCurrent);

        bUpdateExclusion = true;

        for(std::vector < symbol >::iterator it(oSymbols.begin()); it != oSymbols.end(); ++it) {
          int iSymbol(*it);

          //      std::cout << "Symbol " << iSymbol << std::endl;

          CWordnode *pTmpChild(pTmp->find_symbol(iSymbol));

          //      std::cout << "pTmpChild: " << pTmpChild << std::endl;

          if(pTmpChild == NULL) {
            // We don't already have this child, so add a new node

            pTmpChild = m_NodeAlloc.Alloc();
            pTmpChild->sbl = iSymbol;
            pTmpChild->next = pTmp->child;
            pTmp->child = pTmpChild;

            bUpdateExclusion = false;

            // Newly allocated child already has a count of one, so no need to increment it explicitly

          }
          else {
            if(pTmpChild->count == 0)
              bUpdateExclusion = false;
            ++(pTmpChild->count);
          }

          apNodeCache[i]->push_back(pTmpChild);
          ++i;
          pTmp = pTmpChild;

        }

        pCurrent = pCurrent->vine;

        //      std::cout << "foo: " << pCurrent << " " << bUpdateExclusion << std::endl;

      }

      // Now we need to go through and fix up the vine pointers

      //      for( std::vector< std::vector< CWordnode* >* >::iterator it( oNodeCache.begin() ); it != oNodeCache.end(); ++it ) {
      for(unsigned int i(0); i < oSymbols.size(); ++i) {

        CWordnode *pPreviousNode(NULL); // Start with a NULL pointer

        for(std::vector < CWordnode * >::reverse_iterator it2(apNodeCache[i]->rbegin()); it2 != apNodeCache[i]->rend(); ++it2) {
          (*it2)->vine = pPreviousNode;
          pPreviousNode = (*it2);
        }

        delete apNodeCache[i];

      }
      delete[] apNodeCache;

    }

    // Collapse down word part regardless of whether we're learning or not

    int iNewSymbol(lookup_word(context.current_word));

    // Insert into the spelling model if this is a new word

    context.m_pSpellingModel->ReleaseContext(context.oSpellingContext);
    context.oSpellingContext = context.m_pSpellingModel->CreateEmptyContext();

    for (std::vector < int >::iterator it(oSymbols.begin()); it != oSymbols.end(); ++it) {
      context.m_pSpellingModel->LearnSymbol(context.oSpellingContext, *it);
    }

    CWordnode *pTmp(context.word_head);
    CWordnode *pTmpChild;
    CWordnode *pTmpVine(NULL);

    //    std::cout << "pTmp is " << pTmp << std::endl;

    int iUpdateExclusion(1);

    {

      pTmpChild = AddSymbolToNode(pTmp, iNewSymbol, &iUpdateExclusion, false);  // FIXME - might have added a new node here, so fix up vine pointers.

      //      std::cout << "New node: " << pTmpChild << std::endl;

      context.word_head = pTmpChild;
      ++context.word_order;
      pTmpVine = pTmpChild;
      pTmp = pTmp->vine;
    }

    while(pTmp != NULL) {

      //      std::cout << "pTmp is " << pTmp << std::endl;

      pTmpChild = AddSymbolToNode(pTmp, iNewSymbol, &iUpdateExclusion, false);  // FIXME - might have added a new node here, so fix up vine pointers.

      //      std::cout << "New node: " << pTmpChild << std::endl;

      if(pTmpVine)
        pTmpVine->vine = pTmpChild;

      pTmpVine = pTmpChild;
      pTmp = pTmp->vine;
    }

    pTmpVine->vine = m_pRoot;

    // Finally get rid of the letter part of the context

    //    std::cout << "Changed head to " << context.word_head << std::endl;

    while(context.word_order > 2) {
      context.word_head = context.word_head->vine;
      //      std::cout << " * Followed vine to head to " << context.word_head << std::endl;
      --(context.word_order);
    }

    context.head = context.word_head;
    context.order = context.word_order;
    context.current_word = "";

    context.m_pSpellingModel->ReleaseContext(context.oSpellingContext);
    context.oSpellingContext = context.m_pSpellingModel->CreateEmptyContext();

  }

//   if( wordidx == 1 ) {
//      ofstream ofile( "graph.dot" );

//      ofile << "digraph G {" << std::endl;

//      m_pRoot->RecursiveDump( ofile );

//      ofile << "}" << std::endl;

//      ofile.close();

//      exit(0);
//    }

  ++wordidx;

}

void CWordLanguageModel::LearnSymbol(Context c, int Symbol) {
  CWordContext & context = *(CWordContext *) (c);
  AddSymbol(context, Symbol, true);
}

/// add symbol to the context creates new nodes, updates counts and
/// leaves 'context' at the new context

void CWordLanguageModel::AddSymbol(CWordLanguageModel::CWordContext &context, symbol sym, bool bLearn) {
  DASHER_ASSERT(sym >= 0 && sym < GetSize());

  if( context.oSpellingProbs.size() != 0 )
    context.m_dSpellingFactor *= context.oSpellingProbs[sym] / static_cast < double >(context.m_iSpellingNorm);

  // Update the context for the spelling model;

  context.m_pSpellingModel->EnterSymbol(context.oSpellingContext, sym);

  // Add the symbol to the letter part of the context. Note that we don't do any learning at this stage

  CWordnode *pTmp(context.head);        // Current node
  CWordnode *pTmpVine;          // Child created last time around (for vine pointers)

  // Context head is a special case so that we can increment order etc.

  int foo2(1);

  //  std::cout << "aa: " << pTmp << " " << m_pRoot << std::endl;

  pTmpVine = AddSymbolToNode(pTmp, sym, &foo2, false);  // Last parameter is whether to learn or not

  context.head = pTmpVine;
  ++context.order;

  pTmp = pTmp->vine;
  CWordnode *pTmpNew;           // Child created this time around

  while(pTmp != NULL) {

    int foo(1);

    pTmpNew = AddSymbolToNode(pTmp, sym, &foo, false);

    // Connect up vine pointers if necessary

    if(pTmpVine) {
      pTmpVine->vine = pTmpNew;
    }

    pTmpVine = pTmpNew;

    // Follow vine pointers

    pTmp = pTmp->vine;

  }

  pTmpVine->vine = NULL;        // (not sure if this is needed)

  // Add the new symbol to the string representation too - note that
  // string is actually a series of integers, not the actual symbols -
  // doesn't matter as long as we're consistent and unique.

  char sbuffer[5];
  snprintf(sbuffer, 5, "%04d", sym);
  context.current_word.append(sbuffer);

  // Collapse the context (with learning) if we've just entered a space
  // FIXME - we need to generalise this for more languages.

  if(sym == m_iSpaceSymbol) {
    CollapseContext(context, bLearn);
    context.m_dSpellingFactor = 1.0;
  }

}

/////////////////////////////////////////////////////////////////////
// update context with symbol 'Symbol'

void CWordLanguageModel::EnterSymbol(Context c, int Symbol) {
  // Same as AddSymbol but without learning in CollapseContext 

  CWordContext & context = *(CWordContext *) (c);
  AddSymbol(context, Symbol, false);
}
