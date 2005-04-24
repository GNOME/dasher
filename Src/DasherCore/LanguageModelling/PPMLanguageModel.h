// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PPMLanguageModel_h__
#define __PPMLanguageModel_h__


#include "../../Common/NoClones.h"
#include "../../Common/Allocators/PooledAlloc.h"
#include "LanguageModel.h"

#include <vector>
#include <stdio.h>

//static char dumpTrieStr[40000];
const int maxcont =200;

namespace Dasher 
{

class CPPMLanguageModel : public CLanguageModel, private NoClones
{
public:
	CPPMLanguageModel(const CAlphabet* pAlphabet, CLanguageModelParams *_params);
	virtual ~CPPMLanguageModel();
	
	Context CreateEmptyContext();
	void ReleaseContext(Context context);
	Context CloneContext(Context context);
	
	void EnterSymbol(Context context, int Symbol);
	void LearnSymbol(Context context, int Symbol);
	
	//inline bool GetProbs(CContext*,std::vector<symbol> &newchars,std::vector<unsigned int> &groups,std::vector<unsigned int> &probs,double addprob);
	virtual bool GetProbs(Context context, std::vector<unsigned int> &Probs, int norm) const;

	
	void dump();
	
private:

	class CPPMnode {
	public:
		CPPMnode* find_symbol(int sym) const;
		CPPMnode* child;
		CPPMnode* next;
		CPPMnode* vine;
		short int count;
		short int symbol;
		CPPMnode(int sym);
		CPPMnode();
	};
	
	class CPPMContext 
	{
	public:
		CPPMContext(CPPMContext const &input) {head = input.head;	order= input.order;}
		CPPMContext(CPPMnode* _head=0, int _order=0) : head(_head),order(_order) {};
		~CPPMContext() {};
		void dump();
		CPPMnode* head;
		int order;
	};

	CPPMnode* AddSymbolToNode(CPPMnode* pNode, int sym,int *update);
	
	void AddSymbol(CPPMContext& context,int sym);
	void dumpSymbol(int sym);
	void dumpString( char *str, int pos, int len );
	void dumpTrie( CPPMnode *t, int d );

	CPPMContext *m_rootcontext;
	CPPMnode* m_pRoot;

	int max_order;
	double backoff_const;

	mutable CSimplePooledAlloc<CPPMnode> m_NodeAlloc;
	CPooledAlloc<CPPMContext> m_ContextAlloc;
};

////////////////////////////////////////////////////////////////////////
// Inline functions 
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

inline Dasher::CPPMLanguageModel::CPPMnode::CPPMnode(int sym) : symbol(sym)
{
	child=next=vine=0;
	count=1;
}

////////////////////////////////////////////////////////////////////////

inline CPPMLanguageModel::CPPMnode::CPPMnode()
{
	child=next=vine=0;
	count=1;
}


///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CPPMLanguageModel::CreateEmptyContext()
{
	CPPMContext* pCont = m_ContextAlloc.Alloc();
	*pCont = *m_rootcontext;
	return (Context)pCont;
}

///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CPPMLanguageModel::CloneContext(Context Copy)
{
	CPPMContext* pCont = m_ContextAlloc.Alloc();
	CPPMContext* pCopy = (CPPMContext*)Copy;
	*pCont = *pCopy;
	return (Context)pCont;	
}

///////////////////////////////////////////////////////////////////

inline void CPPMLanguageModel::ReleaseContext(Context release)
{
	m_ContextAlloc.Free( (CPPMContext*) release );
}

///////////////////////////////////////////////////////////////////

} // end namespace Dasher

#endif /* #ifndef __PPMLanguageModel_H__ */
