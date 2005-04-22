// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WordLanguageModel_h__
#define __WordLanguageModel_h__


#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"
#include "LanguageModel.h"
#include "../DasherTypes.h"

#include <vector>
#include <map>
#include <string>
#include <stdio.h>

//static char dumpTrieStr[40000];
//const int maxcont =200;

namespace Dasher 
{

class CWordLanguageModel : public CLanguageModel, private NoClones
{
public:
	CWordLanguageModel(const CAlphabet* pAlphabet);
	virtual ~CWordLanguageModel();
	
	Context CreateEmptyContext();
	void ReleaseContext(Context context);
	Context CloneContext(Context context);


	
	void EnterSymbol(Context context, symbol Symbol); // FIXME - lost const
	void LearnSymbol(Context context, symbol Symbol);
	
	//inline bool GetProbs(CContext*,std::vector<symbol> &newchars,std::vector<unsigned int> &groups,std::vector<unsigned int> &probs,double addprob);
	virtual bool GetProbs(Context context, std::vector<unsigned int> &Probs, int norm) const;
	
	void dump();
	
private:

	class CWordnode {
	public:
		CWordnode* find_symbol(int sym) const;
		CWordnode* child;
		CWordnode* next;
		CWordnode* vine;
		short int count;
		int sbl;

		CWordnode(int sym);
		CWordnode();
	};
	
	class CWordContext 
	{
	public:
		CWordContext(CWordContext const &input) {head = input.head; word_head = input.word_head; current_word = input.current_word; order= input.order; word_order = input.word_order; }
		CWordContext(CWordnode* _head=0, int _order=0) : head(_head),order(_order),word_head(_head),word_order(0) {}; // FIXME - doesn't work if we're trying to create a non-empty context
		~CWordContext() {};
		void dump();
		CWordnode* head;
		int order;

		std::string current_word;
		CWordnode* word_head;
		int word_order;
		
	};

	CWordnode* AddSymbolToNode(CWordnode* pNode, symbol sym,int *update);
	
	void AddSymbol(CWordContext& context,symbol sym);
	void dumpSymbol( symbol sym);
	void dumpString( char *str, int pos, int len );
	void dumpTrie( CWordnode *t, int d );

	void CollapseContext(CWordContext &context);

	int lookup_word( const std::string &w );

	CWordContext *m_rootcontext;
	CWordnode* m_pRoot;

	std::map< std::string, int > dict; // Dictionary
	int nextid;

	int max_order;

	mutable CSimplePooledAlloc<CWordnode> m_NodeAlloc;
	CPooledAlloc<CWordContext> m_ContextAlloc;
};

////////////////////////////////////////////////////////////////////////
// Inline functions 
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

inline Dasher::CWordLanguageModel::CWordnode::CWordnode(symbol sym) : sbl(sym)
{
	child=next=vine=0;
	count=1;
}

////////////////////////////////////////////////////////////////////////

inline CWordLanguageModel::CWordnode::CWordnode()
{
	child=next=vine=0;
	count=1;
}


///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CWordLanguageModel::CreateEmptyContext()
{
	CWordContext* pCont = m_ContextAlloc.Alloc();
	*pCont = *m_rootcontext;
	return (Context)pCont;
}

///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CWordLanguageModel::CloneContext(Context Copy)
{
	CWordContext* pCont = m_ContextAlloc.Alloc();
	CWordContext* pCopy = (CWordContext*)Copy;
	*pCont = *pCopy;
	return (Context)pCont;	
}

///////////////////////////////////////////////////////////////////

inline void CWordLanguageModel::ReleaseContext(Context release)
{
	m_ContextAlloc.Free( (CWordContext*) release );
}

///////////////////////////////////////////////////////////////////

} // end namespace Dasher

#endif /* #ifndef __WordLanguageModel_H__ */
