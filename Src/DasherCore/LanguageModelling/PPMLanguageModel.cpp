// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"
#include "PPMLanguageModel.h"

#include <math.h>
#include <stack>
using namespace Dasher;
using namespace std;

// static TCHAR debug[256];
typedef unsigned long ulong;

///////////////////////////////////////////////////////////////////

void CPPMLanguageModel::CPPMContext::dump() 
	// diagnostic output
{
	// TODO uncomment this when headers sorted out
	//dchar debug[128];
	//Usprintf(debug,TEXT("head %x order %d\n"),head,order);
	//DebugOutput(debug);
}


////////////////////////////////////////////////////////////////////////
/// PPMnode definitions 
////////////////////////////////////////////////////////////////////////

CPPMLanguageModel::CPPMnode *CPPMLanguageModel::CPPMnode::find_symbol(int sym) const
// see if symbol is a child of node
{
	//  printf("finding symbol %d at node %d\n",sym,node->id);
	CPPMnode *found=child;
	while (found) {
		if (found->symbol==sym)
			return found;
		found=found->next;
	}
	return 0;
}


CPPMLanguageModel::CPPMnode * CPPMLanguageModel::AddSymbolToNode(CPPMnode* pNode, int sym,int *update)
{
	CPPMnode *pReturn = pNode->find_symbol(sym);
	
	if (pReturn!=NULL)
	{
		if (*update) 
		{   // perform update exclusions
			pReturn->count++;
			*update=0;
		}
		return pReturn;
	}

	pReturn = m_NodeAlloc.Alloc();  // count is initialized to 1
	pReturn->symbol = sym;  
	pReturn->next= pNode->child;
	pNode->child=pReturn;
	return pReturn;		
	
}


/////////////////////////////////////////////////////////////////////
// CPPMLanguageModel defs
/////////////////////////////////////////////////////////////////////

CPPMLanguageModel::CPPMLanguageModel(const CAlphabet* pAlphabet)
  : CLanguageModel(pAlphabet), max_order( 5 ), 
	m_NodeAlloc(8192), m_ContextAlloc(1024)
{
	m_pRoot= m_NodeAlloc.Alloc();
	m_pRoot->symbol = -1;
	m_rootcontext=new CPPMContext(m_pRoot,0);
}


CPPMLanguageModel::~CPPMLanguageModel()
{

	delete m_rootcontext;

	// A non-recursive node deletion algorithm using a stack
/*	std::stack<CPPMnode*> deletenodes;
	deletenodes.push(m_pRoot);
	while (!deletenodes.empty())
	{
		CPPMnode* temp = deletenodes.top();
		deletenodes.pop();
		CPPMnode* next;
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

/////////////////////////////////////////////////////////////////////
// get the probability distribution at the context

bool CPPMLanguageModel::GetProbs( Context context,vector<unsigned int> &probs, int norm) const
{
	const CPPMContext *ppmcontext= (const CPPMContext *)(context);
	
	
	int modelchars=GetNumberModelChars();
	//	int norm=CLanguageModel::normalization();
	probs.resize( GetNumberModelChars() );
	for( vector<unsigned int>::iterator it( probs.begin() ); it != probs.end(); ++it )
	  *it = 0;

	vector<bool> exclusions( probs.size() );
	for( vector<bool>::iterator it2( exclusions.begin() ); it2 != exclusions.end(); ++it2 )
	  *it2 = false;

	vector<bool> valid( probs.size() );
	for( unsigned int i(0); i < valid.size(); ++i )
	  valid[i] = isRealSymbol( i );
	
	CPPMnode *temp,*s; 
	//	int loop,total;
	int sym; 
	ulong size_of_slice;
	ulong tospend=norm;
	temp=ppmcontext->head;

	int total;

	while (temp!=0) {

	  int controlcount=0;

		total=0;
		s=temp->child;
		while (s) {
		  sym=s->symbol; 
		  if (!exclusions[sym] && valid[sym]) {
		    if( sym == GetControlSymbol() ) {
		      // Do nothing
		    } 
		    else if( sym == GetSpaceSymbol() ) {
		      total=total+s->count;
		      
		      controlcount = int(0.4 * s->count); // FIXME - and here
		      
		      if( controlcount < 1 )
			controlcount = 1;

		      if( GetControlSymbol() != -1 )
			total = total + controlcount;

		    }
		    else {
		      total=total+s->count;
		    }
		  }
		  s=s->next;
		}
		if (total) {
		  size_of_slice=tospend;
		  s=temp->child;
		  while (s) {
		    if (!exclusions[s->symbol] && valid[s->symbol]) {
		      //		      exclusions[s->symbol]=1; 
		      if( s->symbol == GetControlSymbol() ) {
			// Do nothing
		      } 
		      else if( s->symbol == GetSpaceSymbol() ) {
			ulong p=((size_of_slice/2)/ulong(total))*(2*s->count-1);
			probs[s->symbol]+=p;
			tospend-=p;
			exclusions[s->symbol]=1;
			if( GetControlSymbol() != -1 )
			  if( !exclusions[GetControlSymbol()] ) {
			    ulong p=((size_of_slice/2)/ulong(total))*(2*controlcount-1);
			    probs[GetControlSymbol()]+=p;
			    tospend-=p;
			    exclusions[GetControlSymbol()]=1;
			  }
		      }
		      else {
			ulong p=((size_of_slice/2)/ulong(total))*(2*s->count-1);
			probs[s->symbol]+=p;
			tospend-=p;	
			exclusions[s->symbol]=1;
		      }
		    }
		    s=s->next;
		  }
		}
		temp = temp->vine;
	}
	//	Usprintf(debug,TEXT("Norm %u tospend %u\n"),Norm,tospend);
	//	DebugOutput(debug);
	
	size_of_slice=tospend;
	int symbolsleft=0;
	for (sym=0;sym<modelchars;sym++)
	  if (!probs[sym] && valid[sym])
	    symbolsleft++;
	for (sym=0;sym<modelchars;sym++) 
	  if (!probs[sym] && valid[sym]) {
	    ulong p=size_of_slice/symbolsleft;
	    probs[sym]+=p;
	    tospend-=p;
	  }
	
			// distribute what's left evenly	
		//tospend+=uniform;

//  	int current_symbol(0);
//  	while( tospend > 0 )
//  	  {
//  	    if( valid[current_symbol] ) {
//  		probs[current_symbol] += 1;
//  		tospend -= 1;
//  	    }

//  	    ++current_symbol;
//  	    if( current_symbol == modelchars )
//  	      current_symbol = 0;
//  	  }

	int valid_char_count(0);

	for( int k(0); k < modelchars; ++k )
	  if( valid[k] ) 
	    ++valid_char_count;
	  
	
	for (int j(0);j<modelchars;++j) 
	  if( valid[j] ) {
	    ulong p=tospend/(valid_char_count);
	    probs[j] +=p;
	    --valid_char_count;
	    tospend -=p;
	  }
//  			  {
//  				ulong p=tospend/(modelchars-sym);
//  				probs[sym]+=p;
//  				tospend-=p;
//  			  }
//  			}
			//	Usprintf(debug,TEXT("finaltospend %u\n"),tospend);
			//	DebugOutput(debug);
			
			// free(exclusions); // !!!
			// !!! NB by IAM: p577 Stroustrup 3rd Edition: "Allocating an object using new and deleting it using free() is asking for trouble"
	//		delete[] exclusions;
			return true;
}


void CPPMLanguageModel::AddSymbol(CPPMLanguageModel::CPPMContext &context,int sym)
	// add symbol to the context
	// creates new nodes, updates counts
	// and leaves 'context' at the new context
{
	// sanity check
	if (sym==0 || sym>=GetNumberModelChars())
		return;
	
	CPPMnode *vineptr,*temp;
	int updatecnt=1;
	
	temp=context.head->vine;
	context.head= AddSymbolToNode(context.head,sym,&updatecnt);
	vineptr=context.head;
	context.order++;
	
	while (temp!=0) {
		vineptr->vine= AddSymbolToNode(temp,sym,&updatecnt);    
		vineptr=vineptr->vine;
		temp=temp->vine;
	}
	vineptr->vine= m_pRoot;
	if (context.order>max_order){
		context.head=context.head->vine;
		context.order--;
	}
}

/////////////////////////////////////////////////////////////////////
// update context with symbol 'Symbol'

void CPPMLanguageModel::EnterSymbol(Context c, int Symbol) const
{
	CPPMLanguageModel::CPPMContext& context = * (CPPMContext *) (c);
	
	CPPMnode *find;
	
	while (context.head) {
		find =context.head->find_symbol(Symbol);
		if (find) {
			context.order++;
			context.head=find;
			//	Usprintf(debug,TEXT("found context %x order %d\n"),head,order);
			//	DebugOutput(debug);
			return;
		}
		context.order--;
		context.head=context.head->vine;
	}
	
	if (context.head==0) {
		context.head= m_pRoot;
		context.order=0;
	}
	
}

/////////////////////////////////////////////////////////////////////

void CPPMLanguageModel::LearnSymbol(Context c, int Symbol)
{
	CPPMLanguageModel::CPPMContext& context = * (CPPMContext *) (c);
	AddSymbol(context, Symbol);
}


void CPPMLanguageModel::dumpSymbol(int sym)
{
	if ((sym <= 32) || (sym >= 127))
		printf( "<%d>", sym );
	else
		printf( "%c", sym );
}


void CPPMLanguageModel::dumpString( char *str, int pos, int len )
	// Dump the string STR starting at position POS
{
	char cc;
	int p;
	for (p = pos; p<pos+len; p++) {
		cc = str [p];
		if ((cc <= 31) || (cc >= 127))
			printf( "<%d>", cc );
		else
			printf( "%c", cc );
	}
}


void CPPMLanguageModel::dumpTrie( CPPMLanguageModel::CPPMnode *t, int d )
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
