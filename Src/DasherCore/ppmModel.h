/////////////////////////////////////////
// PPMmodel.h
// Copyright 2002 David Ward
/////////////////////////////////////////

#ifndef __ppmModel_h__
#define __ppmModel_h__

#include "../Common/NoClones.h"
#include "../Common/MSVC_Unannoy.h"
#include <vector>
#include <stdio.h>

#include "LanguageModel.h"

static char dumpTrieStr[40000];
const int MAX_ORDER = 4;
const int maxcont =200;

namespace Dasher {class CPPMModel;}
class Dasher::CPPMModel : public Dasher::CLanguageModel, private NoClones
{
public:
	CPPMModel(CAlphabet *_alphabet, int _normalization);
	~CPPMModel();
	
	class CPPMnode {
	public:
		CPPMnode* find_symbol(int sym);
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
	bool GetProbs(CContext*, std::vector<unsigned int> &Probs, double AddProb);
	
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


#endif /* #ifndef __PPMMODEL_H__ */
