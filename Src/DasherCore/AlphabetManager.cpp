
#include "../Common/Common.h"

#include "AlphabetManager.h"
#include "DasherModel.h"
#include "DasherNode.h"
#include "Event.h"
#include "EventHandler.h"

#include <vector>

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CAlphabetManager::CAlphabetManager( CDasherModel *pModel, CLanguageModel *pLanguageModel ) 
  : m_pModel(pModel), m_pLanguageModel(pLanguageModel) {
}

CDasherNode *CAlphabetManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper) {
  CDasherNode *pNewNode;

  // FIXME - Make this a CDasherComponent

  pNewNode = new CDasherNode(*m_pModel, pParent, 0, 0, Opts::Nodes1, iLower, iUpper, m_pLanguageModel, false, 7);
  
  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext()); // FIXME - handle context properly
  pNewNode->m_pNodeManager = this;
  pNewNode->m_bShove = true;

  return pNewNode;
}

void CAlphabetManager::PopulateChildren( CDasherNode *pNode ) {
  PopulateChildrenWithSymbol( pNode, -2, 0 );
}

void CAlphabetManager::PopulateChildrenWithSymbol( CDasherNode *pNode, int iExistingSymbol, CDasherNode *pExistingChild ) {

  // Actually create the children here
  
  // FIXME: this has to change for history stuff and Japanese dasher
    std::vector < symbol > newchars; // place to put this list of characters
    std::vector < unsigned int >cum; // for the probability list

    m_pModel->GetProbs(pNode->Context(), newchars, cum, m_pModel->GetLongParameter(LP_NORMALIZATION));
    int iChildCount = newchars.size();

//    DASHER_TRACEOUTPUT("ChildCount %d\n", iChildCount);
    // work out cumulative probs in place
    for(int i = 1; i < iChildCount; i++)
      cum[i] += cum[i - 1];

    // create the children
    ColorSchemes NormalScheme, SpecialScheme;
    if((pNode->ColorScheme() == Nodes1) || (pNode->ColorScheme() == Special1)) {
      NormalScheme = Nodes2;
      SpecialScheme = Special2;
    }
    else {
      NormalScheme = Nodes1;
      SpecialScheme = Special1;
    }

    ColorSchemes ChildScheme;

    int iLbnd = 0;


pNode->Children().resize(iChildCount); 

    for(int j = 0; j < iChildCount; j++) {
      if(newchars[j] == m_pModel->GetSpaceSymbol())
        ChildScheme = SpecialScheme;
      else
        ChildScheme = NormalScheme;
      CDasherNode *pNewNode;

      if( newchars[j] == m_pModel->GetControlSymbol() )
	pNewNode = m_pModel->GetRoot(1, pNode, iLbnd, cum[j]);
      else if( newchars[j] == iExistingSymbol) {
	pNewNode = pExistingChild;
	pNewNode->SetRange(iLbnd, cum[j]);
      }
      else {
	pNewNode = new CDasherNode(*m_pModel, pNode, newchars[j], j, ChildScheme, iLbnd, cum[j], m_pLanguageModel, false, m_pModel->GetColour(j));
	pNewNode->m_pNodeManager = this;
	pNewNode->m_bShove = true;
      }

      pNode->Children()[j] = pNewNode;
      iLbnd = cum[j];
    }

}

void CAlphabetManager::ClearNode( CDasherNode *pNode ) {
  // Should this be responsible for actually doing the deletion

  //  std::cout << "Deleting: " << pNode << std::endl;

}

void CAlphabetManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  m_pModel->m_bContextSensitive = true;
  symbol t = pNode->Symbol();
  if(t) { // Ignore symbol 0 (root node)
    Dasher::CEditEvent oEvent(1, m_pModel->GetAlphabet().GetText(t));
    m_pModel->InsertEvent(&oEvent);

    // Track this symbol and its probability for logging purposes
    if (pAdded != NULL) {
      Dasher::SymbolProb sItem;
      sItem.sym    = t;
      sItem.prob   = pNode->GetProb(iNormalization);

      pAdded->push_back(sItem);
    }
  }
}

void CAlphabetManager::Undo( CDasherNode *pNode ) {
  symbol t = pNode->Symbol();
  if(t) { // Ignore symbol 0 (root node)
    Dasher::CEditEvent oEvent(2, m_pModel->GetAlphabet().GetText(t));
    m_pModel->InsertEvent(&oEvent);
  }
}

CDasherNode *CAlphabetManager::RebuildParent(CDasherNode *pNode, int iGeneration) {

  m_pModel->m_strContextBuffer = "";

  Dasher::CEditContextEvent oEvent(10);
  m_pModel->InsertEvent(&oEvent);

  std::string strContext(m_pModel->m_strContextBuffer);
  std::vector<symbol> vSymbols;
  m_pLanguageModel->SymbolAlphabet().GetAlphabetPointer()->GetSymbols(&vSymbols, &strContext, false);

  // Return if the context isn't long enough to build a new parent

  if(vSymbols.size() <= iGeneration)
    return 0; 


  ColorSchemes NormalScheme, SpecialScheme;
  if((pNode->ColorScheme() == Nodes1) || (pNode->ColorScheme() == Special1)) {
    NormalScheme = Nodes2;
    SpecialScheme = Special2;
  }
  else {
    NormalScheme = Nodes1;
    SpecialScheme = Special1;
  }

  ColorSchemes ChildScheme;
  if(vSymbols[vSymbols.size() - iGeneration - 1] == m_pModel->GetSpaceSymbol())
    ChildScheme = SpecialScheme;
  else
    ChildScheme = NormalScheme;
  

  CDasherNode *pNewNode;

  pNewNode = new CDasherNode(*m_pModel, 0, vSymbols[vSymbols.size() - iGeneration - 1], 0, ChildScheme, 0, 0, m_pLanguageModel, false, m_pModel->GetColour(vSymbols[vSymbols.size() - iGeneration - 1]));
  
  CLanguageModel::Context oContext(m_pLanguageModel->CreateEmptyContext());

  for(int i(0); i < vSymbols.size() - iGeneration; ++i)
    m_pLanguageModel->EnterSymbol(oContext, vSymbols[i]);

  pNewNode->SetContext(oContext);
  pNewNode->m_pNodeManager = this;
  pNewNode->m_bShove = true;
  pNewNode->Seen(true);

  PopulateChildrenWithSymbol( pNewNode, pNode->Symbol(), pNode );

  pNode->SetParent(pNewNode);

  return pNewNode;
}
