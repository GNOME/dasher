// PPMLanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1999-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PPMLanguageModel_h__
#define __PPMLanguageModel_h__


#include "../Common/NoClones.h"
#include "LanguageModel.h"

#include <vector>
#include <stdio.h>

//static char dumpTrieStr[40000];
const int MAX_ORDER = 5;
const int maxcont =200;

namespace Dasher {class CPPMLanguageModel;}
class Dasher::CPPMLanguageModel : public Dasher::CLanguageModel, private NoClones
{
public:
	CPPMLanguageModel(CAlphabet *_alphabet);
	~CPPMLanguageModel();
	
	
	void ReleaseContext(CContext*);
	
	CContext* GetEmptyContext() const;
	
	CContext* CloneContext(const CContext*);
	
	void EnterSymbol(CContext* pContext, int Symbol);
	
	//inline bool GetProbs(CContext*,std::vector<symbol> &newchars,std::vector<unsigned int> &groups,std::vector<unsigned int> &probs,double addprob);
	virtual bool GetProbs(const CContext* pContext, std::vector<unsigned int> &Probs, int norm) const;
	
	void LearnSymbol(CContext* Context, int Symbol);
	void dump();
	
private:

	class CPPMnode {
	public:
		CPPMnode* find_symbol(int sym) const;
		CPPMnode* add_symbol_to_node(int sym,int *update);
		CPPMnode* child;
		CPPMnode* next;
		CPPMnode* vine;
		short int count;
		const short int symbol;
		CPPMnode(int sym);
	};
	
	class CPPMContext : public CContext {
	public:
		CPPMContext(CPPMContext const &input) {head = input.head;	order= input.order;}
		CPPMContext(CPPMnode* _head=0, int _order=0) : head(_head),order(_order) {};
		~CPPMContext() {};
		void dump();
		CPPMnode* head;
		int order;
	};

	void AddSymbol(CPPMContext& context,int sym);
	void dumpSymbol(int sym);
	void dumpString( char *str, int pos, int len );
	void dumpTrie( CPPMnode *t, int d );

	CPPMContext *m_rootcontext;
	CPPMnode *root;


};

////////////////////////////////////////////////////////////////////////
// Inline functions 
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

inline CPPMLanguageModel::CPPMnode::CPPMnode(int sym) : symbol(sym)
{
	child=next=vine=0;
	count=1;
}

///////////////////////////////////////////////////////////////////

inline void CPPMLanguageModel::CPPMContext::dump() 
	// diagnostic output
{
	// TODO uncomment this when headers sorted out
	//dchar debug[128];
	//Usprintf(debug,TEXT("head %x order %d\n"),head,order);
	//DebugOutput(debug);
}

///////////////////////////////////////////////////////////////////

inline CContext* CPPMLanguageModel::GetEmptyContext() const
{
	return  static_cast<CContext *>(new CPPMLanguageModel::CPPMContext(*m_rootcontext));
}

///////////////////////////////////////////////////////////////////

inline CContext* CPPMLanguageModel::CloneContext(const CContext *pCopyThis)
{
	const CPPMContext *pPPMcontext=static_cast<const CPPMContext *> (pCopyThis);
	return  static_cast<CContext *> (new CPPMContext(*pPPMcontext));
}

///////////////////////////////////////////////////////////////////

inline void CPPMLanguageModel::ReleaseContext(CContext *release)
{
	delete release;
}

#endif /* #ifndef __PPMLanguageModel_H__ */
