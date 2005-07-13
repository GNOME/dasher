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
#include "PPMLanguageModel.h"
#include "../DasherTypes.h"

#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <fstream>
#include <iostream>

//static char dumpTrieStr[40000];
//const int maxcont =200;

namespace Dasher 
{

class CWordLanguageModel : public CLanguageModel
{
public:
	CWordLanguageModel(const CSymbolAlphabet& Alphabet, CLanguageModelParams *_params);
	virtual ~CWordLanguageModel();
	
	Context CreateEmptyContext();
	void ReleaseContext(Context context);
	Context CloneContext(Context context);

	virtual void GetProbs(Context Context, std::vector<unsigned int> &Probs, int iNorm) const;

	virtual void EnterSymbol(Context context, int Symbol);
	virtual void LearnSymbol(Context context, int Symbol);	

	virtual int GetMemory() {
	  return NodesAllocated;
	}

	
private:

	class CWordnode {
	public:
		CWordnode* find_symbol(int sym) const;
		CWordnode* child;
		CWordnode* next;
		CWordnode* vine;
		unsigned int count;
		int sbl;

		CWordnode(int sym);
		CWordnode();


		void RecursiveDump( std::ofstream &file );
	};
	
	class CWordContext 
	{
	public:
		CWordContext(CWordContext const &input) {
		  head = input.head; 
		  word_head = input.word_head; 
		  current_word = input.current_word; 
		  order= input.order; 
		  word_order = input.word_order;
		}

		CWordContext(CWordnode* _head=0, int _order=0) : 
		  head(_head),order(_order),word_head(_head),word_order(0) {

		  //		  std::cout << "oops - badness" << std::endl;

		}; // FIXME - doesn't work if we're trying to create a non-empty context

		~CWordContext() {};
		void dump();

		CWordnode* head; // Overall order for context
		int order;       // 

		std::string current_word; // String representation of the current word
		CWordnode* word_head;     // The head of the word part of the context
		int word_order;           // Order of word-based part of context

		std::vector< unsigned int > oSpellingProbs; // Cached probabilities from the spelling model
		
		int m_iSpellingNorm; // Normalisation for spelling probabilities
		double m_dSpellingFactor; // 

		CPPMLanguageModel::Context oSpellingContext; // Context to use when making spelling predictions

		
	};

	CWordnode* AddSymbolToNode(CWordnode* pNode, symbol sym,int *update, bool bLearn);
	
	void AddSymbol(CWordContext& context,symbol sym, bool bLearn);

	void CollapseContext(CWordContext &context, bool bLearn);

	int lookup_word( const std::string &w );
	int lookup_word_const( const std::string &w ) const; 

	CWordContext *m_rootcontext;
	CWordnode* m_pRoot;

	std::map< std::string, int > dict; // Dictionary
	int nextid;
	int iWordStart;

	int wordidx;

	int NodesAllocated;

	int max_order;

	CPPMLanguageModel *pSpellingModel; // Use this to predict the spellings of new words



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

	pCont->oSpellingContext = pSpellingModel->CreateEmptyContext();

	return (Context)pCont;
}

///////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CWordLanguageModel::CloneContext(Context Copy)
{

  //  std::cout << "Cloning context" << std::endl;

	CWordContext* pCont = m_ContextAlloc.Alloc();
	CWordContext* pCopy = (CWordContext*)Copy;
	*pCont = *pCopy;
	return (Context)pCont;	
}

///////////////////////////////////////////////////////////////////

inline void CWordLanguageModel::ReleaseContext(Context release)
{
  pSpellingModel->ReleaseContext( ((CWordContext *)release)->oSpellingContext );

  m_ContextAlloc.Free( (CWordContext*) release );
}

///////////////////////////////////////////////////////////////////

} // end namespace Dasher

#endif /* #ifndef __WordLanguageModel_H__ */
