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
#include "../Common/MSVC_Unannoy.h"
#include <vector>
#include <stdio.h>

#include "LanguageModel.h"

//static char dumpTrieStr[40000];
const int MAX_ORDER = 5;
const int maxcont =200;

namespace Dasher {class CPPMLanguageModel;}
class Dasher::CPPMLanguageModel : public Dasher::CLanguageModel, private NoClones
{
public:
	CPPMLanguageModel(CAlphabet *_alphabet);
	~CPPMLanguageModel();
	
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
	
	void ReleaseContext(CContext*);
	CContext* GetRootContext();
	inline CContext* CloneContext(CContext*);
	void EnterSymbol(CContext* context, modelchar Symbol);
	//inline bool GetProbs(CContext*,std::vector<symbol> &newchars,std::vector<unsigned int> &groups,std::vector<unsigned int> &probs,double addprob);
	bool GetProbs(CContext*, std::vector<unsigned int> &Probs, int norm);
	
	void LearnSymbol(CContext* Context, modelchar Symbol);
	void dump();
	
private:
	CPPMContext *m_rootcontext;
	CPPMnode *root;
	void AddSymbol(CPPMContext& context,int symbol);
	void dumpSymbol(int symbol);
	void dumpString( char *str, int pos, int len );
	void dumpTrie( CPPMnode *t, int d );



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

inline CContext* CPPMLanguageModel::GetRootContext()
{
	CPPMContext * nc = new CPPMLanguageModel::CPPMContext(*m_rootcontext);
	CContext *cont=static_cast<CContext *> (nc);
	return  cont;
}

///////////////////////////////////////////////////////////////////

inline CContext* CPPMLanguageModel::CloneContext(CContext *copythis)
{
	CPPMContext *ppmcontext=static_cast<CPPMContext *> (copythis);
	CPPMContext * nc = new CPPMLanguageModel::CPPMContext(*ppmcontext);
	return  static_cast<CContext *> (nc);
}

///////////////////////////////////////////////////////////////////

inline void CPPMLanguageModel::ReleaseContext(CContext *release)
{
	delete release;
}

#endif /* #ifndef __PPMLanguageModel_H__ */
