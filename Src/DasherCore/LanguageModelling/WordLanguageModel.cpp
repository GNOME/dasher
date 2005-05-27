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

CWordLanguageModel::CWordnode *CWordLanguageModel::CWordnode::find_symbol(int sym) const
// see if symbol is a child of node
{
	//  printf("finding symbol %d at node %d\n",sym,node->id);
	CWordnode *found=child;
	while (found) {
		if (found->sbl==sym)
			return found;
		found=found->next;
	}
	return 0;
}


CWordLanguageModel::CWordnode * CWordLanguageModel::AddSymbolToNode(CWordnode* pNode, symbol sym,int *update)
{
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

	const CWordLanguageModel::CWordContext *wordcontext= (const CWordContext *)(context);
		
	int iNumSymbols = GetSize();

	probs.resize( iNumSymbols );
	
	std::vector<bool> exclusions(iNumSymbols);

	int i;
	for( i=1 ; i < iNumSymbols; i++)
	{
		probs[i] = 0;
		exclusions[i] = false;
	}

	bool doExclusion = (LanguageModelParams()->GetValue( std::string( "LMExclusion" ) ) == 1 );

	int alpha = LanguageModelParams()->GetValue( std::string( "LMAlpha" ) );
	int beta = LanguageModelParams()->GetValue( std::string( "LMBeta" ) );

	unsigned int iToSpend = norm;

	CWordnode* pTemp=wordcontext->head;

	while (pTemp != 0) 
	{
		int iTotal=0;
		
		CWordnode* pSymbol = pTemp->child;
		while (pSymbol)
		{
			int sym = pSymbol->sbl; 
			if (!(doExclusion && exclusions[sym]))
				iTotal += pSymbol->count;
			pSymbol = pSymbol->next;
		}

		if (iTotal) 
		{
			unsigned int size_of_slice = iToSpend;
			pSymbol = pTemp->child;
			while (pSymbol) 
			{
				if (!(doExclusion && exclusions[pSymbol->sbl])) 
				{
					exclusions[pSymbol->sbl]=1;
					
					unsigned int p = static_cast<unsigned long long int>(size_of_slice)*(100*pSymbol->count - beta)/(100*iTotal + alpha);
					probs[pSymbol->sbl]+=p;
					iToSpend-=p;		
				}
				//				Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);	 
				//				DebugOutput(debug);
				pSymbol = pSymbol->next;
			}
		}
		pTemp = pTemp->vine;
	}
	
	unsigned int size_of_slice= iToSpend;
	int symbolsleft=0;
	
	for (i=0; i < iNumSymbols ; i++)
	  if ( !(doExclusion && exclusions[i]) )
	    symbolsleft++;
	
//	std::ostringstream str;
//	for (sym=0;sym<modelchars;sym++)
//		str << probs[sym] << " ";
//	str << std::endl;
//	DASHER_TRACEOUTPUT("probs %s",str.str().c_str());

//	std::ostringstream str2;
//	for (sym=0;sym<modelchars;sym++)
//		str2 << valid[sym] << " ";
//	str2 << std::endl;
//	DASHER_TRACEOUTPUT("valid %s",str2.str().c_str());

	for (i=0;  i < iNumSymbols ; i++) 
	{
		if (!(doExclusion && exclusions[i]) ) 
		{
			unsigned int p=size_of_slice/symbolsleft;
			probs[i]+=p;
			iToSpend -= p;
		}
	}

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




// void CWordLanguageModel::GetProbs( Context context,vector<unsigned int> &probs, int norm) const
// {
// // 	const CWordContext *ppmcontext= (const CWordContext *)(context);
	
	
// // 	//	int modelchars=GetNumberModelChars();

// // 	int modelchars=Alphabet.GetSize();

// // 	//	int norm=CLanguageModel::normalization();
// // 	probs.resize( modelchars );
// // 	for( vector<unsigned int>::iterator it( probs.begin() ); it != probs.end(); ++it )
// // 	  *it = 0;

// // 	vector<bool> exclusions( probs.size() );
// // 	for( vector<bool>::iterator it2( exclusions.begin() ); it2 != exclusions.end(); ++it2 )
// // 	  *it2 = false;

// // 	vector<bool> valid( probs.size() );
// // 	for( unsigned int i(0); i < valid.size(); ++i )
// // 	  valid[i] = isRealSymbol( i );
	
// // 	CWordnode *temp,*s; 
// // 	//	int loop,total;
// // 	int sym; 
// // 	ulong size_of_slice;
// // 	ulong tospend=norm;
// // 	temp=ppmcontext->head;

// // 	int total;

// // 	while (temp!=0) {

// // 	  int controlcount=0;

// // 		total=0;
// // 		s=temp->child;
// // 		while (s) {
// // 		  sym=s->sbl; 
// // 		  if (!exclusions[sym] && valid[sym]) {
// // 		    if( sym == GetControlSymbol() ) {
// // 		      // Do nothing
// // 		    } 
// // 		    else if( sym == GetSpaceSymbol() ) {
// // 		      total=total+s->count;
		      
// // 		      controlcount = int(0.4 * s->count); // FIXME - and here
		      
// // 		      if( controlcount < 1 )
// // 			controlcount = 1;

// // 		      if( GetControlSymbol() != -1 )
// // 			total = total + controlcount;

// // 		    }
// // 		    else {
// // 		      total=total+s->count;
// // 		    }
// // 		  }
// // 		  s=s->next;
// // 		}
// // 		if (total) {
// // 		  size_of_slice=tospend;
// // 		  s=temp->child;
// // 		  while (s) {
// // 		    if (!exclusions[s->sbl] && valid[s->sbl]) {
// // 		      //		      exclusions[s->symbol]=1; 
// // 		      if( s->sbl == GetControlSymbol() ) {
// // 			// Do nothing
// // 		      } 
// // 		      else if( s->sbl == GetSpaceSymbol() ) {
// // 			ulong p=((size_of_slice/2)/ulong(total))*(2*s->count-1);
// // 			probs[s->sbl]+=p;
// // 			tospend-=p;
// // 			exclusions[s->sbl]=1;
// // 			if( GetControlSymbol() != -1 )
// // 			  if( !exclusions[GetControlSymbol()] ) {
// // 			    ulong p=((size_of_slice/2)/ulong(total))*(2*controlcount-1);
// // 			    probs[GetControlSymbol()]+=p;
// // 			    tospend-=p;
// // 			    exclusions[GetControlSymbol()]=1;
// // 			  }
// // 		      }
// // 		      else {

// // 	ulong p=(((size_of_slice/2)*(2*s->count-1))/ulong(total)); // Changed to multiply before divide to avoid underflow problems when counts are really high - however, there's a chance this will cause overflow problems

// // 	//			ulong p=((size_of_slice/2)/ulong(total))*(2*s->count-1);
// // 			probs[s->sbl]+=p;
// // 			tospend-=p;	
// // 			exclusions[s->sbl]=1;
// // 		      }
// // 		    }
// // 		    s=s->next;
// // 		  }
// // 		}
// // 		temp = temp->vine;
// // 	}
// // 	//	Usprintf(debug,TEXT("Norm %u tospend %u\n"),Norm,tospend);
// // 	//	DebugOutput(debug);
	
// // 	size_of_slice=tospend;
// // 	int symbolsleft=0;
// // 	for (sym=0;sym<modelchars;sym++)
// // 	  if (!probs[sym] && valid[sym])
// // 	    symbolsleft++;
// // 	for (sym=0;sym<modelchars;sym++) 
// // 	  if (!probs[sym] && valid[sym]) {
// // 	    ulong p=size_of_slice/symbolsleft;
// // 	    probs[sym]+=p;
// // 	    tospend-=p;
// // 	  }
	
// // 			// distribute what's left evenly	
// // 		//tospend+=uniform;

// // //  	int current_symbol(0);
// // //  	while( tospend > 0 )
// // //  	  {
// // //  	    if( valid[current_symbol] ) {
// // //  		probs[current_symbol] += 1;
// // //  		tospend -= 1;
// // //  	    }

// // //  	    ++current_symbol;
// // //  	    if( current_symbol == modelchars )
// // //  	      current_symbol = 0;
// // //  	  }

// // 	int valid_char_count(0);

// // 	for( int k(0); k < modelchars; ++k )
// // 	  if( valid[k] ) 
// // 	    ++valid_char_count;
	  
	
// // 	for (int j(0);j<modelchars;++j) 
// // 	  if( valid[j] ) {
// // 	    ulong p=tospend/(valid_char_count);
// // 	    probs[j] +=p;
// // 	    --valid_char_count;
// // 	    tospend -=p;
// // 	  }
// // //  			  {
// // //  				ulong p=tospend/(modelchars-sym);
// // //  				probs[sym]+=p;
// // //  				tospend-=p;
// // //  			  }
// // //  			}
// // 			//	Usprintf(debug,TEXT("finaltospend %u\n"),tospend);
// // 			//	DebugOutput(debug);
			
// // 			// free(exclusions); // !!!
// // 			// !!! NB by IAM: p577 Stroustrup 3rd Edition: "Allocating an object using new and deleting it using free() is asking for trouble"
// // 	//		delete[] exclusions;
// // 			return true;
// }


void CWordLanguageModel::CollapseContext( CWordLanguageModel::CWordContext &context ) const {

  if( max_order == 0 ) {
    // If max_order = 0 then we are not keeping track of previous
    // words, so we just collapse the letter part of the context and
    // return

    context.head = m_pRoot;
    context.order = 0;
  }
  else {
 
	  context.head = context.word_head;
	  context.order = context.word_order;

//	  CWordnode *new_head;

	  int new_sbl( lookup_word_const( context.current_word ) );

	  CWordnode *new_tmp;
	  CWordnode *prev_tmp( NULL );

	  CWordnode *tmp( context.head );
	    
	  while( tmp != NULL ) {
	  
	    new_tmp = tmp->find_symbol( new_sbl );
	    
	    if( new_tmp == NULL ) {

	      new_tmp = m_NodeAlloc.Alloc();  // count is initialized to 1
	      new_tmp->sbl = new_sbl;  
	      new_tmp->next= tmp->child;
	      tmp->child=new_tmp;
	    }
	    
	    if( prev_tmp != NULL )
	      prev_tmp->vine = new_tmp;
	    else
	      context.head = new_tmp;
	    
	    prev_tmp = new_tmp;
	    tmp = tmp->vine;
	  }

	  prev_tmp->vine = m_pRoot;

	  context.current_word = "";
	  ++context.order;
	  ++context.word_order;
	  
	  if (context.order>max_order){
	    context.head=context.head->vine;
	    context.order--;
	    context.word_order--;
	  }
	  
	  context.word_head = context.head;
  }
}

void CWordLanguageModel::LearnSymbol(Context c, int Symbol)
{
  CWordContext& context = * (CWordContext *) (c);
  AddSymbol(context, Symbol);
}

void CWordLanguageModel::AddSymbol(CWordLanguageModel::CWordContext &context,symbol sym)
	// add symbol to the context
	// creates new nodes, updates counts
	// and leaves 'context' at the new context

{
	DASHER_ASSERT(sym>=0 && sym< GetSize());

	// Update the context with the new symbol

	CWordnode *vineptr,*temp;
	int updatecnt=1;
	
	temp=context.head->vine;
	context.head= AddSymbolToNode(context.head,sym,&updatecnt);

	// Add the new symbol to the string representation of the current word too

 	if( max_order > 0 ) {
 	  char sbuffer[5];
 	  snprintf( sbuffer, 5, "%04d", sym );
 	  context.current_word.append( sbuffer );
 	}

	// Propagate down the vine pointers

	vineptr=context.head;
	context.order++;
	
	while (temp!=0) {
		vineptr->vine= AddSymbolToNode(temp,sym,&updatecnt);    
		vineptr=vineptr->vine;
		temp=temp->vine;
	}
	vineptr->vine= m_pRoot;

	// Collapse the context if we have started a new word

	if( sym == SymbolAlphabet().GetSpaceSymbol() ) {
	  CollapseContext( context );
	}

	while (context.order> LanguageModelParams()->GetValue( std::string( "LMMaxOrder" ) ))
	{
	  context.head=context.head->vine;
	  context.order--;
	}
}

/////////////////////////////////////////////////////////////////////
// update context with symbol 'Symbol'

void CWordLanguageModel::EnterSymbol(Context c, int Symbol) const
{	

  CWordContext& context = * (CWordContext *) (c);
	
  // Add the symbol to the current word string

  //  cout << max_order << std::endl;

  if( max_order > 0 ) {
    char sbuffer[5];
    snprintf( sbuffer, 5, "%04d", Symbol );
    context.current_word.append( sbuffer );
  }

  // Collapse context if necessary - note that there's no point in
  // traversing the trie for the new symbol if we're just going to
  // collapse the context - the information required to update the
  // word part of the context is stored in the string.

  if( Symbol == SymbolAlphabet().GetSpaceSymbol() ) {
    CollapseContext( context );
    return;
  }

  // Traverse the trie if we don't have a space symbol

  CWordnode *find;

  while (context.head) {

    if( context.order < LanguageModelParams()->GetValue( std::string( "LMMaxOrder" ))) {
    find =context.head->find_symbol(Symbol);
    if (find) {
      context.order++;
      context.head=find;

      //      std::cout << context.order << std::endl;
      return;
    }

    }
    //    m_iMaxOrder = params->GetValue( std::string( "LMMaxOrder" ) );
    
    //    std::cout << "a" << std::endl;

    //    while (context.order>  params->GetValue( std::string( "LMMaxOrder" )))
    // {
    //std::cout << "b" << std::endl;

	context.order--;
	context.head=context.head->vine;
	// }
  }
  
  if (context.head==0) {
    context.head= m_pRoot;
    context.order=0;
  }
  //  std::cout << "(reduced) " << context.order << std::endl;
}

