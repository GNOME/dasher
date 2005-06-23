// WordLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"
#include "WordLanguageModel.h"

#include <math.h>
#include <stack>
#include <iostream>
using namespace Dasher;
using namespace std;

// static TCHAR debug[256];
typedef unsigned long ulong;

#ifdef DASHER_WIN32
#define snprintf _snprintf
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

CWordLanguageModel::CWordnode *CWordLanguageModel::CWordnode::find_symbol(int sym) const
{
  CWordnode *found=child;
  while (found) {
    if (found->sbl==sym)
      return found;
    found=found->next;
  }
  return 0;
}


CWordLanguageModel::CWordnode * CWordLanguageModel::AddSymbolToNode(CWordnode* pNode, symbol sym, int *update, bool bLearn )
{

  // FIXME - need to implement bLearn

	CWordnode *pReturn = pNode->find_symbol(sym);
	
	if (pReturn!=NULL)
	{
		if (( LanguageModelParams()->GetValue( std::string( "LMUpdateExclusion" ) ) == 0 ) ||*update) 
		{   // perform update exclusions
			 if( pReturn->count < USHRT_MAX ) // Truncate counts at storage limit
			   pReturn->count++;
			*update=0;
		}
		return pReturn;
	}
	pReturn = m_NodeAlloc.Alloc();  // count is initialized to 1
	pReturn->sbl = sym;  
	pReturn->next= pNode->child;
	pNode->child=pReturn;

	++NodesAllocated;

	return pReturn;		
	
}


/////////////////////////////////////////////////////////////////////
// CWordLanguageModel defs
/////////////////////////////////////////////////////////////////////

CWordLanguageModel::CWordLanguageModel(const CSymbolAlphabet &Alphabet, CLanguageModelParams *_params)
  : CLanguageModel(Alphabet, _params), max_order( 0 ), 
    m_NodeAlloc(8192), m_ContextAlloc(1024), NodesAllocated(0)
{
	m_pRoot= m_NodeAlloc.Alloc();
	m_pRoot->sbl = -1;
	m_rootcontext=new CWordContext(m_pRoot,0);

	nextid = 8192; // Start of indices for words - may need to increase this for *really* large alphabets
}


CWordLanguageModel::~CWordLanguageModel()
{

	delete m_rootcontext;

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

int CWordLanguageModel::lookup_word( const std::string &w ) {

  if( dict[ w ] == 0 ) {
    dict[ w ] = nextid;
    ++nextid;
  }

  return dict[ w ];
  
}

int CWordLanguageModel::lookup_word_const( const std::string &w ) const {
 
  return dict.find( w )->second;
  
}

/////////////////////////////////////////////////////////////////////
// get the probability distribution at the context


void CWordLanguageModel::GetProbs( Context context,vector<unsigned int> &probs, int norm) const
{

  const CWordLanguageModel::CWordContext *wordcontext=(const CWordContext *)(context);

  // Make sure that the probability vector has the right length
		
  int iNumSymbols = GetSize();
  probs.resize( iNumSymbols );

  // For the prototype work with double precision to make things easier to normalise

  std::vector< double > dProbs;

  for( std::vector< double >::iterator it( dProbs.begin() ); it != dProbs.end(); ++it )
    *it = 0.0;

  double alpha = LanguageModelParams()->GetValue( std::string( "LMAlpha" ) )/100.0;
  double beta = LanguageModelParams()->GetValue( std::string( "LMBeta" ) )/100.0;

  double dToSpend(1.0);
  double dRemaining;

  CWordnode* pTmp=wordcontext->head;
  CWordnode* pTmpWord = wordcontext->word_head;

  // We'll assume that these stay in sync for now - maybe do something more robust later.

  while(pTmp)
    {
      // Get the total count from the word node

      int iTotal( pTmpWord->count );
      
      if (iTotal) 
	{
	  dRemaining = dToSpend;

	  CWordnode *pTmpChild( pTmp->child );

	  while (pTmpChild) 
	    {
	      double dP = dRemaining * (pTmpChild->count - beta)/static_cast<double>(iTotal + alpha);
	      dProbs[pTmpChild->sbl]+=dP;
	      dToSpend -= dP;		
	  
	      pTmpChild = pTmpChild->next;
	    }
	}
      pTmp = pTmp->vine;
      pTmpWord = pTmpWord->vine;
    }


  // Escape to a uniform model. In actuality, this should be a letter
  // only model for spelling unseen words, but this hasn't yet been
  // implemented

  dRemaining = dToSpend;

  for(int i(0); i < iNumSymbols; ++i) {
    double dP( dRemaining / static_cast<double>( iNumSymbols ) );
    dProbs[i] += dP;
    dToSpend -= dP;
  }

  // Convert floating point to integer probabilities

  int iToSpend( norm );

  for(int i(0); i < iNumSymbols; ++i) {
    probs[i] = norm * dProbs[i];

    iToSpend -= probs[i];
  }

  // Check that we haven't got anything left over due to rounding errors:

  int iLeft = iNumSymbols;
  
  for (int j=0; j< iNumSymbols; ++j) 
    {
      unsigned int p= iToSpend/iLeft;
      probs[j] +=p;
      --iLeft;
      iToSpend -=p;
    }
  
  DASHER_ASSERT(iToSpend == 0);
}


/// Collapse the context. This also has the effect of entering a count
/// for the word into the word part of the model

void CWordLanguageModel::CollapseContext( CWordLanguageModel::CWordContext &context, bool bLearn ) {

  // Letters appear at the end of the trie:
  // 


  if( max_order == 0 ) {
    // If max_order = 0 then we are not keeping track of previous
    // words, so we just collapse the letter part of the context and
    // return

    context.head = m_pRoot;
    context.order = 0;
 
  }
  else {

    if( bLearn ) { // Only do this if we are learning
      // We need to increment all substrings - start at the current context striped back to the word level
      
      bool bUpdateExclusion( false ); // Whether to keep going or not
      
      CWordnode *pCurrent( context.word_head );
      
      // Keep track of pointers to all child nodes
      
      std::vector< std::vector< CWordnode* >* > oNodeCache;
      
      // FIXME - remember to delete member vectors when we're done
      
      std::vector<symbol> oSymbols;
      
      // FIXME broken SymbolAlphabet().GetAlphabetPointer()->GetSymbols( &oSymbols, &(context.current_word), false );

      // We're not storing the actual string - just a list of symbol IDs
      
      while( (pCurrent != NULL) && !bUpdateExclusion ) {
	
	std::vector< CWordnode* > *pCurrentCache( new std::vector< CWordnode* > );
	
	CWordnode *pTmp( pCurrent );
	
	for( std::vector<symbol>::iterator it( oSymbols.begin() ); it != oSymbols.end(); ++it ) {
	  int iSymbol( *it );
	  
	  CWordnode *pTmpChild( pTmp->find_symbol( iSymbol ) );
	  
	  if( pTmpChild == NULL ) {
	    // We don't already have this child, so add a new node
	    
	    pTmpChild = m_NodeAlloc.Alloc();
	    pTmpChild->sbl = iSymbol;
	    pTmpChild->next = pTmp->child;
	    pTmp->child = pTmpChild;
	    
	    // Newly allocated child already has a count of one, so no need to increment it explicitly
	    
	  }
	  else {
	    bUpdateExclusion = true;
	    ++pTmpChild->count;
	  }
	  
	  pCurrentCache->push_back( pTmpChild );
	  
	  pTmp = pTmpChild;
	  
	}
	
	pCurrent = pCurrent->vine;
	
	oNodeCache.push_back( pCurrentCache );
      }
      
      // Now we need to go through and fix up the vine pointers
      
      for( std::vector< std::vector< CWordnode* >* >::iterator it( oNodeCache.begin() ); it != oNodeCache.end(); ++it ) {
	
	CWordnode *pPreviousNode( NULL ); // Start with a NULL pointer
	
	for( std::vector< CWordnode * >::reverse_iterator it2( (*it)->rbegin() ); it2 != (*it)->rend(); ++it2 ) {
	  (*it2)->vine = pPreviousNode;
	  pPreviousNode = (*it2);
	}
	
      }
    }

    // Collapse down word part regardless of whether we're learning or not

    int iNewSymbol( lookup_word_const( context.current_word ) );

    CWordnode *pTmp( context.word_head );
    CWordnode *pTmpChild;

    while( pTmp != NULL ) {

      int foo(1);

      AddSymbolToNode( pTmp, iNewSymbol, &foo, true ); // FIXME - might have added a new node here, so fix up vine pointers.
      pTmp = pTmp->vine;

    }

    // FIXME - add symbol if it doesn't already exist


 // Finally get rid of the letter part of the context
    
    context.head = context.word_head;
    context.order = context.word_order;    
  }
}

void CWordLanguageModel::LearnSymbol(Context c, int Symbol)
{
  CWordContext& context = * (CWordContext *) (c);
  AddSymbol(context, Symbol, true);
}

/// add symbol to the context creates new nodes, updates counts and
/// leaves 'context' at the new context

void CWordLanguageModel::AddSymbol(CWordLanguageModel::CWordContext &context,symbol sym, bool bLearn)
{
  DASHER_ASSERT(sym>=0 && sym< GetSize());

  // Add the symbol to the letter part of the context. Note that we don't do any learning at this stage

  CWordnode *pTmp( context.head ); // Current node
  CWordnode *pTmpVine; // Child created last time around (for vine pointers)

  // Context head is a special case so that we can increment order etc.

  int foo2(1);

  pTmpVine =  AddSymbolToNode( pTmp, sym, &foo2, false ); // Last parameter is whether to learn or not

  context.head = pTmpVine;
  ++context.order;

  pTmp = pTmp->vine;

  while( pTmp != NULL ) {
    CWordnode *pTmpNew; // Child created this time around
    
    int foo(1);

    pTmpNew = AddSymbolToNode( pTmp, sym, &foo, false );

    // Connect up vine pointers if necessary

    if( pTmpVine ) {
      pTmpVine->vine = pTmpNew;
    }

    pTmpVine = pTmpNew;

    // Follow vine pointers

    pTmp = pTmp->vine;

  }

  pTmpVine->vine = NULL; // (not sure if this is needed)

  // Add the new symbol to the string representation too - note that
  // string is actually a series of integers, not the actual symbols -
  // doesn't matter as long as we're consistent and unique.

  char sbuffer[5];
  snprintf( sbuffer, 5, "%04d", sym );
  context.current_word.append( sbuffer );

  // Collapse the context (with learning) if we've just entered a space
  // FIXME - we need to generalise this for more languages.

  if( sym == SymbolAlphabet().GetSpaceSymbol() ) {
    CollapseContext( context, bLearn );
  }
}

/////////////////////////////////////////////////////////////////////
// update context with symbol 'Symbol'

void CWordLanguageModel::EnterSymbol(Context c, int Symbol)
{	
  // Same as AddSymbol but without learning in CollapseContext 

  CWordContext& context = * (CWordContext *) (c);
  AddSymbol(context, Symbol, false);
}

