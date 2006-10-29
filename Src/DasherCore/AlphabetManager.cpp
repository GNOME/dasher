
#include "../Common/Common.h"

#include "AlphabetManager.h"
//#include "DasherModel.h"
#include "DasherNode.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"

#include <vector>
#include <iostream>

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

CAlphabetManager::CAlphabetManager( CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel, CLanguageModel::Context iLearnContext, bool bGameMode, const std::string &strGameModeText ) 
  : CNodeManager(0), m_pLanguageModel(pLanguageModel), m_pNCManager(pNCManager) {

  m_iLearnContext = iLearnContext;
  m_bGameMode = bGameMode;
  m_strGameString = strGameModeText;
}

CDasherNode *CAlphabetManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  int iSymbol;

  if(pUserData)
    iSymbol = *((int *)pUserData);
  else
    iSymbol = 0;

  // FIXME - Make this a CDasherComponent

  int iColour;
  
  if(iSymbol == 0)
    iColour = 7;
  else
    iColour = m_pNCManager->GetColour(iSymbol);

  pNewNode = new CDasherNode(pParent, iSymbol, 0, Opts::Nodes1, iLower, iUpper, m_pLanguageModel, iColour);
  
  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext()); // FIXME - handle context properly
  pNewNode->m_pNodeManager = this;
  pNewNode->m_bShove = true;
  pNewNode->m_pBaseGroup = m_pNCManager->GetAlphabet()->m_pBaseGroup;
  pNewNode->m_strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol);
  pNewNode->Seen(true);

  SAlphabetData *pNodeUserData = new SAlphabetData;
  pNewNode->m_pUserData = pNodeUserData;

  pNodeUserData->iPhase = 0;
  pNodeUserData->iSymbol = iSymbol;
  pNodeUserData->pLanguageModel = m_pLanguageModel;

  // Hopefully this will obsolete any need to handle contexts outside
  // of the alphabet manager - check this and remove resulting
  // redundant code
  CLanguageModel::Context iContext;
  iContext = m_pLanguageModel->CreateEmptyContext();
  pNodeUserData->iContext = iContext;

  if(m_bGameMode) {
    pNodeUserData->iGameOffset = -1;
    pNewNode->SetGame(true);
  }

  return pNewNode;
}

void CAlphabetManager::PopulateChildren( CDasherNode *pNode ) {
  PopulateChildrenWithSymbol( pNode, -2, 0 );
}

void CAlphabetManager::PopulateChildrenWithSymbol( CDasherNode *pNode, int iExistingSymbol, CDasherNode *pExistingChild ) {
  int iOldPhase(static_cast<SAlphabetData *>(pNode->m_pUserData)->iPhase);
  int iNewPhase((iOldPhase + 1) % 2);

  SAlphabetData *pParentUserData(static_cast<SAlphabetData *>(pNode->m_pUserData));

  // Actually create the children here
  
  // FIXME: this has to change for history stuff and Japanese dasher
  std::vector < symbol > newchars; // place to put this list of characters
  std::vector < unsigned int >cum; // for the probability list
  
  // TODO: Need to fix up relation to language model here (use one from node, not global).
  m_pNCManager->GetProbs(pParentUserData->iContext, newchars, cum, m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  int iChildCount = newchars.size();
  
  // work out cumulative probs in place
  for(int i = 1; i < iChildCount; i++)
    cum[i] += cum[i - 1];
  
  int iLbnd = 0;
  
  pNode->Children().resize(iChildCount); 
  
  for(int j = 0; j < iChildCount; j++) {
    CDasherNode *pNewNode;
    
    if(newchars[j] == m_pNCManager->GetControlSymbol())
      pNewNode = m_pNCManager->GetRoot(1, pNode, iLbnd, cum[j], NULL);
    else if(newchars[j] == m_pNCManager->GetStartConversionSymbol()) {
      // TODO: Need to consider the case where there is no compile-time support for this
      pNewNode = m_pNCManager->GetRoot(2, pNode, iLbnd, cum[j], NULL);
    }
    else if( newchars[j] == iExistingSymbol) {
      pNewNode = pExistingChild;
      pNewNode->SetRange(iLbnd, cum[j]);
    }
    else {
      int iColour(m_pNCManager->GetColour(newchars[j]));

      // This is provided for backwards compatibility. 
      // Colours should always be provided by the alphabet file
      if(iColour == -1) {
	if(newchars[j] == m_pNCManager->GetSpaceSymbol()) {
	  iColour = 9;
	}
	else if(newchars[j] == m_pNCManager->GetControlSymbol()) {
	  iColour = 8;
	}
	else {
	  iColour = (newchars[j] % 3) + 10;
	}
      }
      
      // Loop colours if necessary for the colour scheme
      if(iNewPhase == 1 && iColour < 130) // We don't loop on high
	iColour += 130;
      
      pNewNode = new CDasherNode(pNode, newchars[j], 0, Nodes1, iLbnd, cum[j], pParentUserData->pLanguageModel, iColour);
      pNewNode->m_pNodeManager = this;
      pNewNode->m_bShove = true;
      pNewNode->m_pBaseGroup = m_pNCManager->GetAlphabet()->m_pBaseGroup;
      
      SAlphabetData *pNodeUserData = new SAlphabetData;
      pNewNode->m_pUserData = pNodeUserData;
      
      pNodeUserData->iPhase = iNewPhase;
      pNodeUserData->iSymbol = newchars[j];
      
      if(m_bGameMode) {
	int iCurrentGameOffset(static_cast<SAlphabetData *>(pNode->m_pUserData)->iGameOffset);
	
	if((iCurrentGameOffset != -2) && ((iCurrentGameOffset + 1) < static_cast<int>(m_strGameString.size())) 
	   && ((m_pNCManager->GetAlphabet()->GetText(newchars[j]))[0] == m_strGameString[iCurrentGameOffset + 1])) {
	  static_cast<SAlphabetData *>(pNewNode->m_pUserData)->iGameOffset = iCurrentGameOffset + 1;
	  pNewNode->SetGame(true);
	}
	else
	  static_cast<SAlphabetData *>(pNewNode->m_pUserData)->iGameOffset = -2;
      }

      CLanguageModel::Context iContext;
      iContext = m_pLanguageModel->CloneContext(pParentUserData->iContext);
      m_pLanguageModel->EnterSymbol(iContext, newchars[j]); // TODO: Don't use symbols?
      
      pNodeUserData->pLanguageModel = pParentUserData->pLanguageModel;
      pNodeUserData->iContext = iContext;
      
      //      pNewNode->SetContext(iContext); // FIXME - handle context properly
    }
    
    pNewNode->m_strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(newchars[j]);
    
    pNode->Children()[j] = pNewNode;
    iLbnd = cum[j];
  }
  
  pNode->SetHasAllChildren(true);
}

void CAlphabetManager::ClearNode( CDasherNode *pNode ) {
  SAlphabetData *pUserData(static_cast<SAlphabetData *>(pNode->m_pUserData));
  
  pUserData->pLanguageModel->ReleaseContext(pUserData->iContext);
  delete pUserData;
}

void CAlphabetManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  //  m_pNCManager->m_bContextSensitive = true;
  symbol t = static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol;

  if(t) { // Ignore symbol 0 (root node)
    Dasher::CEditEvent oEvent(1, m_pNCManager->GetAlphabet()->GetText(t));
    m_pNCManager->InsertEvent(&oEvent);

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
  symbol t = static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol;
  if(t) { // Ignore symbol 0 (root node)
    Dasher::CEditEvent oEvent(2, m_pNCManager->GetAlphabet()->GetText(t));
    m_pNCManager->InsertEvent(&oEvent);
  }
}

CDasherNode *CAlphabetManager::RebuildParent(CDasherNode *pNode, int iGeneration) {

  // TODO: Reimplement -----

  // Ask for context information
  //  m_pNCManager->m_strContextBuffer = "";
  //  Dasher::CEditContextEvent oEvent(10);
  //  m_pNCManager->InsertEvent(&oEvent);

  //  std::string strContext(m_pNCManager->m_strContextBuffer);
  std::vector<symbol> vSymbols;
  //  m_pLanguageModel->SymbolAlphabet().GetAlphabetPointer()->GetSymbols(&vSymbols, &strContext, false);

  // -----

  CDasherNode *pNewNode;

  int iOldPhase(static_cast<SAlphabetData *>(pNode->m_pUserData)->iPhase);
  int iNewPhase;

  symbol iNewSymbol;

  CLanguageModel::Context oContext(m_pLanguageModel->CreateEmptyContext());

  if(static_cast<int>(vSymbols.size()) <= iGeneration + 1) {
    // Create a root node if there isn't any more context
    iNewPhase = 0;
    iNewSymbol = 0;

    pNewNode = new CDasherNode(0, 0, 0,  Opts::Nodes1, 0, 0, m_pLanguageModel, 7);
    
    // TODO: Horrible hard coded default context
    m_pNCManager->EnterText(oContext, ". ");
  }
  else {
    
    iNewPhase = ((iOldPhase + 2 - 1) % 2);
    iNewSymbol = vSymbols[vSymbols.size() - iGeneration - 2];

    int iColour(m_pNCManager->GetColour(vSymbols[vSymbols.size() - iGeneration - 2]));

    // Loop colours if necessary for the colour scheme
    if(iNewPhase == 1)
      iColour += 130;
            
    pNewNode = new CDasherNode(0, iNewSymbol, 0, Nodes1, 0, 0, m_pLanguageModel, iColour);

    for(unsigned int i(0); i < vSymbols.size() - iGeneration -1; ++i)
      m_pLanguageModel->EnterSymbol(oContext, vSymbols[i]);
  }
  
  pNewNode->m_pNodeManager = this;
  pNewNode->m_bShove = true;
  pNewNode->Seen(true);
  pNewNode->m_pBaseGroup = m_pNCManager->GetAlphabet()->m_pBaseGroup;

  SAlphabetData *pNodeUserData = new SAlphabetData;
  pNewNode->m_pUserData = pNodeUserData;

  pNodeUserData->iPhase = iNewPhase;
  pNodeUserData->iSymbol = iNewSymbol;
  pNodeUserData->pLanguageModel = m_pLanguageModel;
  pNodeUserData->iContext = oContext;

  PopulateChildrenWithSymbol(pNewNode, static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol, pNode);

  pNode->SetParent(pNewNode);
  
  return pNewNode;
}

void CAlphabetManager::SetFlag(CDasherNode *pNode, int iFlag, bool bValue) {
  switch(iFlag) {
  case NF_COMMITTED:
    if(bValue)
      // TODO: Reimplement (need a learning context, check whether symbol actually corresponds to character)
      static_cast<SAlphabetData *>(pNode->m_pUserData)->pLanguageModel->LearnSymbol(m_iLearnContext, static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol);
    break;
  }
}
