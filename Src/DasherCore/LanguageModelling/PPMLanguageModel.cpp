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
#include <climits>

using namespace Dasher;
using namespace std;

/////////////////////////////////////////////////////////////////////

CPPMLanguageModel::CPPMLanguageModel(const CSymbolAlphabet& SymbolAlphabet, CLanguageModelParams *_params)
  : CLanguageModel(SymbolAlphabet, _params), m_iMaxOrder( 5 ), 
	m_NodeAlloc(8192), m_ContextAlloc(1024)
{
	m_pRoot= m_NodeAlloc.Alloc();
	m_pRoot->symbol = -1;

	m_pRootContext= m_ContextAlloc.Alloc();
	m_pRootContext->head=m_pRoot;
	m_pRootContext->order=0;
}

/////////////////////////////////////////////////////////////////////

CPPMLanguageModel::~CPPMLanguageModel()
{
}

/////////////////////////////////////////////////////////////////////
// Get the probability distribution at the context

void CPPMLanguageModel::GetProbs( Context context,vector<unsigned int> &probs, int norm) const
{
	const CPPMContext *ppmcontext= (const CPPMContext *)(context);
	
	
	int iNumSymbols = GetSize();

	probs.resize( iNumSymbols );
	
	std::vector<bool> exclusions(iNumSymbols);

	int i;

	// Note - iNumSymbols is one more than the alphabet 'size' due to zero symbol being added to represent the root node (?) Hack things to
	// ensure zero probs for 0 symbol for now, but I thought that the idea of the new symbol alphabets was to prevent this sort of thing
	// from being necessary

	for( i=1 ; i < iNumSymbols; i++)
	{
		probs[i] = 0;
		exclusions[i] = false;
	}

	unsigned int iToSpend = norm;

	CPPMnode* pTemp=ppmcontext->head;


	while (pTemp != 0) 
	{
		int iTotal=0;
		
		CPPMnode* pSymbol = pTemp->child;
		while (pSymbol)
		{

			int sym = pSymbol->symbol; 
			if (!exclusions[sym])
				iTotal += pSymbol->count;
			pSymbol = pSymbol->next;
		}

		if (iTotal) 
		{
			unsigned int size_of_slice = iToSpend;
			pSymbol = pTemp->child;
			while (pSymbol) 
			{
				if (!exclusions[pSymbol->symbol]) 
				{
					exclusions[pSymbol->symbol]=1;

					// Cast to unsigned long long
					// int here to prevent
					// overflow problems. We
					// probably need to double
					// check that there are still
					// no issues if he have huge
					// counts,

					unsigned int p = static_cast<unsigned long long int>(size_of_slice)*(2*pSymbol->count - 1)/2/iTotal;
					probs[pSymbol->symbol]+=p;

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
	

	for (i=1; i < iNumSymbols ; i++)
	  if ( ! probs[i] )
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

	for (i=1;  i < iNumSymbols ; i++) 
	{
		if (!probs[i] ) 
		{
			unsigned int p=size_of_slice/symbolsleft;
			probs[i]+=p;
			iToSpend -= p;
		}
	}


	int iLeft = iNumSymbols - 1; // Subtract one because we're ignoreing the zero symbol

	for (int j=1; j< iNumSymbols; ++j) 
	{
		unsigned int p= iToSpend/iLeft;
		probs[j] +=p;
		--iLeft;
		iToSpend -=p;
	}


	DASHER_ASSERT(iToSpend == 0);
}


void CPPMLanguageModel::AddSymbol(CPPMLanguageModel::CPPMContext &context,int sym)
	// add symbol to the context
	// creates new nodes, updates counts
	// and leaves 'context' at the new context
{
	DASHER_ASSERT(sym>=0 && sym< GetSize());

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

	m_iMaxOrder = params->GetValue( std::string( "LMMaxOrder" ) );

	while (context.order> m_iMaxOrder)
	{
		context.head=context.head->vine;
		context.order--;
	}
}

/////////////////////////////////////////////////////////////////////
// Update context with symbol 'Symbol'

void CPPMLanguageModel::EnterSymbol(Context c, int Symbol) const
{
	DASHER_ASSERT(Symbol>=0 && Symbol< GetSize());

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
	DASHER_ASSERT(Symbol>=0 && Symbol< GetSize());

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


////////////////////////////////////////////////////////////////////////
/// PPMnode definitions 
////////////////////////////////////////////////////////////////////////

CPPMLanguageModel::CPPMnode *CPPMLanguageModel::CPPMnode::find_symbol(int sym) const
// see if symbol is a child of node
{
	//  printf("finding symbol %d at node %d\n",sym,node->id);
	CPPMnode *found=child;

	while (found) {
	  if (found->symbol==sym) {
			return found;
	  }
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

		  if( pReturn->count < USHRT_MAX ) // Truncate counts at storage limit
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


