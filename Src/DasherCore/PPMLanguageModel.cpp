// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "PPMLanguageModel.h"

using namespace Dasher;
using namespace std;

// static TCHAR debug[256];
typedef unsigned long ulong;

////////////////////////////////////////////////////////////////////////
/// PPMnode definitions 
////////////////////////////////////////////////////////////////////////

CPPMLanguageModel::CPPMnode *CPPMLanguageModel::CPPMnode::find_symbol(int sym)
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


CPPMLanguageModel::CPPMnode * CPPMLanguageModel::CPPMnode::add_symbol_to_node(int sym,int *update)
{
	CPPMnode *born,*search;
	search=find_symbol(sym);
	if (!search) {
		born = new CPPMnode(sym);
		born->next=child;
		child=born;
		//   node->count=1;
		return born;		
	} else {
		if (*update) {   // perform update exclusions
			search->count++;
			*update=0;
		}
		return search;
	}
	
}


/////////////////////////////////////////////////////////////////////
// CPPMLanguageModel defs
/////////////////////////////////////////////////////////////////////

CPPMLanguageModel::CPPMLanguageModel(CAlphabet *_alphabet,int _normalization)
	: CLanguageModel(_alphabet,_normalization)
{
	root=new CPPMnode(-1);
	m_rootcontext=new CPPMContext(root,0);
}


CPPMLanguageModel::~CPPMLanguageModel()
{
	delete root;
}


bool CPPMLanguageModel::GetProbs(CContext *context,vector<unsigned int> &probs,double addprob)
	// get the probability distribution at the context
{
	// seems like we have to have this hack for VC++
	CPPMContext *ppmcontext=static_cast<CPPMContext *> (context);
	
	
	int modelchars=GetNumberModelChars();
	int norm=CLanguageModel::normalization();
	probs.resize(modelchars);
	CPPMnode *temp,*s; 
	int loop,total;
	int sym; 
	ulong spent=0; 
	ulong size_of_slice;
	bool *exclusions=new bool [modelchars];
	ulong uniform=modelchars;
	ulong tospend=norm-uniform;
	temp=ppmcontext->head;
	for (loop=0; loop <modelchars; loop++) {   /* set up the exclusions array */
		probs[loop]=0;
		exclusions[loop]=0;
	}
	while (temp!=0) {
		//	Usprintf(debug,TEXT("tospend %u\n"),tospend);
		//	DebugOutput(TEXT("round\n"));
		total=0;
		s=temp->child;
		while (s) {
			sym=s->symbol; 
			if (!exclusions[s->symbol])
				total=total+s->count;
			s=s->next;
		}
		if (total) {
			//	Usprintf(debug,TEXT"escape %u\n"),tospend*
			size_of_slice=tospend;
			s=temp->child;
			while (s) {
				if (!exclusions[s->symbol]) {
					exclusions[s->symbol]=1;
					ulong p=size_of_slice*(2*s->count-1)/2/ulong(total);
					probs[s->symbol]+=p;
					tospend-=p;		
				}
				//				Usprintf(debug,TEXT("sym %u counts %d p %u tospend %u \n"),sym,s->count,p,tospend);	 
				//				DebugOutput(debug);
				s=s->next;
			}
		}
		temp = temp->vine;
	}
	//	Usprintf(debug,TEXT("Norm %u tospend %u\n"),Norm,tospend);
	//	DebugOutput(debug);
	
	size_of_slice=tospend;
	int symbolsleft=0;
	for (sym=1;sym<modelchars;sym++)
		if (!probs[sym])
			symbolsleft++;
		for (sym=1;sym<modelchars;sym++) 
			if (!probs[sym]) {
				ulong p=size_of_slice/symbolsleft;
				probs[sym]+=p;
				tospend-=p;
			}
			
			// distribute what's left evenly	
			tospend+=uniform;
			for (sym=1;sym<modelchars;sym++) {
				ulong p=tospend/(modelchars-sym);
				probs[sym]+=p;
				tospend-=p;
			}
			//	Usprintf(debug,TEXT("finaltospend %u\n"),tospend);
			//	DebugOutput(debug);
			
			// free(exclusions); // !!!
			// !!! NB by IAM: p577 Stroustrup 3rd Edition: "Allocating an object using new and deleting it using free() is asking for trouble"
			delete[] exclusions;
			return true;
}


void CPPMLanguageModel::AddSymbol(CPPMLanguageModel::CPPMContext &context,int symbol)
	// add symbol to the context
	// creates new nodes, updates counts
	// and leaves 'context' at the new context
{
	// sanity check
	if (symbol==0 || symbol>=GetNumberModelChars())
		return;
	
	CPPMnode *vineptr,*temp;
	int updatecnt=1;
	
	temp=context.head->vine;
	context.head=context.head->add_symbol_to_node(symbol,&updatecnt);
	vineptr=context.head;
	context.order++;
	
	while (temp!=0) {
		vineptr->vine=temp->add_symbol_to_node(symbol,&updatecnt);    
		vineptr=vineptr->vine;
		temp=temp->vine;
	}
	vineptr->vine=root;
	if (context.order>MAX_ORDER){
		context.head=context.head->vine;
		context.order--;
	}
}


// update context with symbol 'Symbol'
void CPPMLanguageModel::EnterSymbol(CContext* Context, modelchar Symbol)
{
	CPPMLanguageModel::CPPMContext& context = * static_cast<CPPMContext *> (Context);
	
	CPPMnode *find;
	CPPMnode *temp=context.head;
	
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
		context.head=root;
		context.order=0;
	}
	
}


void CPPMLanguageModel::LearnSymbol(CContext* Context, modelchar Symbol)
{
	CPPMLanguageModel::CPPMContext& context = * static_cast<CPPMContext *> (Context);
	AddSymbol(context, Symbol);
}


void CPPMLanguageModel::dumpSymbol(int symbol)
{
	if ((symbol <= 32) || (symbol >= 127))
		printf( "<%d>", symbol );
	else
		printf( "%c", symbol );
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
