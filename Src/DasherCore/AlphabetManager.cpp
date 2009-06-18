// AlphabetManager.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "../Common/Common.h"

#include "AlphabetManager.h"
#include "ConversionManager.h"
#include "DasherInterfaceBase.h"
#include "DasherNode.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"


#include <vector>
#include <sstream>
#include <iostream>

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CAlphabetManager::CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel, CLanguageModel::Context iLearnContext, int iConversionID) 
  : CNodeManager(0), m_pLanguageModel(pLanguageModel), m_pNCManager(pNCManager) {
  m_pInterface = pInterface;

  m_iLearnContext = iLearnContext;

  //Added for Mandarin Dasher
  m_iConversionID = iConversionID;

}

CDasherNode *CAlphabetManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {

  CDasherNode *pNewNode;

  // TODO: iOffset has gotten a bit hacky here

  int iSymbol;
  int iOffset;
  int iColour;
  
  std::string strContext;

  CLanguageModel::Context iContext;

  if(pUserData && static_cast<SRootData *>(pUserData)->szContext) {
    std::string strRootText(static_cast<SRootData *>(pUserData)->szContext);

    int iMaxContextLength = m_pLanguageModel->GetContextLength() + 1;

    // TODO: No need to explicitly pass context
    // TODO: Utility function for looking up symbolic context

    int iStart = static_cast<SRootData *>(pUserData)->iOffset - iMaxContextLength;
    if(iStart < 0)
      iStart = 0;

    strContext = m_pInterface->GetContext(iStart, static_cast<SRootData *>(pUserData)->iOffset - iStart);
    BuildContext(strContext, false, iContext, iSymbol);

    iOffset = static_cast<SRootData *>(pUserData)->iOffset - 1;
    iColour = m_pNCManager->GetColour(iSymbol, 0);
  }
  else {
    // Create a root node
    
    if(pUserData)
      iOffset = static_cast<SRootData *>(pUserData)->iOffset;
    else
      iOffset = -1;
    iColour = 7;

    strContext = m_pNCManager->GetAlphabet()->GetDefaultContext();
    BuildContext(strContext, true, iContext, iSymbol);
  }

  // FIXME - Make this a CDasherComponent
  

  // Stuff which could in principle be done in the symbol node creation routine
  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = iColour;
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = true;
  pDisplayInfo->strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol);

  pNewNode = new CDasherNode(pParent, iLower, iUpper, pDisplayInfo);

  pNewNode->m_pNodeManager = this;

  SAlphabetData *pNodeUserData = new SAlphabetData;
  pNewNode->m_pUserData = pNodeUserData;

  pNodeUserData->iOffset = iOffset;
  pNodeUserData->iPhase = 0;
  pNodeUserData->iSymbol = iSymbol;



  //Added for Mandarin Dasher
  if((m_iConversionID==2)&&(pParent)){
    CConversionManager::SConversionData *pParentConversionData = static_cast<CConversionManager::SConversionData *>(pParent->m_pUserData);
  
    pNodeUserData->iContext = m_pLanguageModel->CloneContext(pParentConversionData->iContext);
    
  }
  else if(m_iConversionID==2){
    pNodeUserData->iContext = m_pLanguageModel->CreateEmptyContext();
  }
  else
    pNodeUserData->iContext = iContext;


  pNodeUserData->pLanguageModel = m_pLanguageModel;

  pNewNode->SetFlag(NF_SEEN, true);


  
  //    if(m_bGameMode) {
  //    pNodeUserData->iGameOffset = -1;
       pNewNode->SetFlag(NF_GAME, true);
    //  }
  
  return pNewNode;
}

void CAlphabetManager::PopulateChildren( CDasherNode *pNode ) {
  PopulateChildrenWithSymbol( pNode, -2, 0 );
}

CDasherNode *CAlphabetManager::CreateGroupNode(CDasherNode *pParent, SGroupInfo *pInfo, std::vector<unsigned int> *pCProb, unsigned int iStart, unsigned int iEnd, unsigned int iMin, unsigned int iMax) {

#ifdef WIN32
  unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) * 
	  (unsigned __int64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
  unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) * 
	  (unsigned __int64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) / 
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
#else
  unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) * 
	  (unsigned long long int)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
  unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) * 
	  (unsigned long long int)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) / 
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
#endif

  SAlphabetData *pParentData = static_cast<SAlphabetData *>(pParent->m_pUserData);
  
  // TODO: More sensible structure in group data to map directly to this
  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = pInfo->iColour;
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = pInfo->bVisible;
  pDisplayInfo->strDisplayText = pInfo->strLabel;

  CDasherNode *pNewNode = new CDasherNode(pParent, iLbnd, iHbnd, pDisplayInfo);

  pNewNode->m_pNodeManager = this;
  pNewNode->SetFlag(NF_SUBNODE, true);

  SAlphabetData *pNodeUserData = new SAlphabetData;
  pNewNode->m_pUserData = pNodeUserData;

  // When creating a group node...
  pNodeUserData->iOffset = pParentData->iOffset; // ...the offset is the same as the parent...
  pNodeUserData->iPhase = pParentData->iPhase;
  // TODO: Sort out symbol for groups
  pNodeUserData->iSymbol = 0; //...but the Symbol is just a 0.
  pNodeUserData->pLanguageModel = pParentData->pLanguageModel;
  pNodeUserData->iContext = pParentData->pLanguageModel->CloneContext(pParentData->iContext);

  return pNewNode;
}

// TODO: use these functions elsewhere in the file
CDasherNode *CAlphabetManager::CreateSymbolNode(CDasherNode *pParent, symbol iSymbol, std::vector<unsigned int> *pCProb, unsigned int iStart, unsigned int iEnd, unsigned int iMin, unsigned int iMax, symbol iExistingSymbol, CDasherNode *pExistingChild) {
  // TODO: Node deletion etc.
  //std::cout << "isymbol: "<<iSymbol << " " << m_pNCManager->GetStartConversionSymbol() << std::endl;

#ifdef WIN32
  unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) * 
	  (unsigned __int64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) / 
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
  unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) * 
	  (unsigned __int64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) / 
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
#else
  unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) * 
	  (unsigned long long int)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) / 
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
  unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) * 
	  (unsigned long long int)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) / 
	  ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
#endif


  SAlphabetData *pParentData = static_cast<SAlphabetData *>(pParent->m_pUserData);
  CDasherNode *pNewNode = NULL;

  
  //Modified for Mandarin Dasher

  //The following logic switch allows punctuation nodes in Mandarin to be treated in the same way as English (i.e. display and populate next round) instead of invoking a conversion node

  if((m_iConversionID!=2)||((m_iConversionID==2)&&(iSymbol>1288))){
    //Does not invoke conversion node

  // TODO: Better way of specifying alternate roots
  // TODO: Building with existing node
    if(iSymbol == iExistingSymbol) {
      pNewNode = pExistingChild;
      pNewNode->SetRange(iLbnd, iHbnd);
      pNewNode->SetParent(pParent);
      
      DASHER_ASSERT(static_cast<SAlphabetData *>(pExistingChild->m_pUserData)->iOffset == pParentData->iOffset + 1);
      
    }
    // TODO: Need to fix fact that this is created even when control mode is switched off
    else if(iSymbol == m_pNCManager->GetControlSymbol()) {
      pNewNode = m_pNCManager->GetRoot(1, pParent, iLbnd, iHbnd, &(pParentData->iOffset)); 
      
      // For now, just hack it so we get a normal root node here
      if(!pNewNode) {
	pNewNode = m_pNCManager->GetRoot(0, pParent, iLbnd, iHbnd, NULL);
      }
    }
    else if(iSymbol == m_pNCManager->GetStartConversionSymbol()) {
      //  else if(iSymbol == m_pNCManager->GetSpaceSymbol()) {
      
      // TODO: Need to consider the case where there is no compile-time support for this
      pNewNode = m_pNCManager->GetRoot(2, pParent, iLbnd, iHbnd, &(pParentData->iOffset));
    }         
    else {
      int iPhase = (pParentData->iPhase + 1) % 2;
      int iColour = m_pNCManager->GetColour(iSymbol, iPhase);
      
      // TODO: Exceptions / error handling in general
      
      CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
      pDisplayInfo->iColour = iColour;
      pDisplayInfo->bShove = true;
      pDisplayInfo->bVisible = true;
      pDisplayInfo->strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol);
      
      
      
      pNewNode = new CDasherNode(pParent, iLbnd, iHbnd, pDisplayInfo);
      
      
      //     std::stringstream ssLabel;

      //     ssLabel << m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol) << ": " << pNewNode;
      
      //    pDisplayInfo->strDisplayText = ssLabel.str();
      
      
      pNewNode->m_pNodeManager = this;
      
      pNewNode->m_iNumSymbols = 1;
      
      SAlphabetData *pNodeUserData = new SAlphabetData;
      pNewNode->m_pUserData = pNodeUserData;
      
      pNodeUserData->iOffset = pParentData->iOffset + 1;
      pNodeUserData->iPhase = iPhase;
      pNodeUserData->iSymbol = iSymbol;
      
      pNodeUserData->pLanguageModel = pParentData->pLanguageModel; // TODO: inconsistent with above?
      

      //Context carry-over. This code may worth looking at debug      
      if(m_iConversionID==2){     
	pNodeUserData->iContext = m_pLanguageModel->CloneContext(pParentData->iContext);
      }
      else{
	CLanguageModel::Context iContext;
	iContext = m_pLanguageModel->CloneContext(pParentData->iContext);
	//m_pLanguageModel->EnterSymbol(iContext, iSymbol); 
	
	pNodeUserData->iContext = iContext;
      }
      
    }
  }
  
  //modified for Mandarin dasher
  else{    
    
    pNewNode = m_pNCManager->GetRoot(2, pParent, iLbnd, iHbnd, &(pParentData->iOffset));
    
    static_cast<SAlphabetData *>(pNewNode->m_pUserData)->iSymbol = iSymbol;
    
  }
  
  return pNewNode;
}


void CAlphabetManager::RecursiveIterateGroup(CDasherNode *pParent, SGroupInfo *pInfo, std::vector<symbol> *pSymbols, std::vector<unsigned int> *pCProb, int iMin, int iMax, symbol iExistingSymbol, CDasherNode *pExistingChild) {
  // TODO: Think through alphabet file formats etc. to make this class easier.
  // TODO: Throw a warning if parent node already has children

  CDasherNode **pChildNodes = new CDasherNode *[iMax - iMin];

  for(int i(iMin); i < iMax; ++i) {
    pChildNodes[i - iMin] = NULL;
  }

  // Create child nodes and cache them for later
  // The SGroupInfo structure has something like linked list behaviour
  // Each SGroupInfo contains a pNext, a pointer to a sibling group info
  SGroupInfo *pCurrentNode(pInfo);
  while(pCurrentNode) {
    CDasherNode *pNewChild = CreateGroupNode(pParent,
					     pCurrentNode,
					     pCProb,
					     pCurrentNode->iStart,
					     pCurrentNode->iEnd,
					     iMin, iMax);

    RecursiveIterateGroup(pNewChild,
			  pCurrentNode->pChild,
			  pSymbols,
			  pCProb,
			  pCurrentNode->iStart,
			  pCurrentNode->iEnd,
			  iExistingSymbol, pExistingChild);

    for(int i(pCurrentNode->iStart); i < pCurrentNode->iEnd; ++i) {
      pChildNodes[i - iMin] = pNewChild;
    }

    pCurrentNode = pCurrentNode->pNext; // Move along the group
  }

  CDasherNode *pLastChild = NULL;

  // Now actually populate the children
  for(int i(iMin); i < iMax; ++i) {
    if(!pChildNodes[i-iMin]) {
      CDasherNode *pNewChild = CreateSymbolNode(pParent, (*pSymbols)[i], pCProb, i, i+1, iMin, iMax, iExistingSymbol, pExistingChild);
      pParent->Children().push_back(pNewChild);
      pLastChild = pNewChild;
    }
    else if (pChildNodes[i-iMin] != pLastChild) {
      pParent->Children().push_back(pChildNodes[i-iMin]);
      pLastChild = pChildNodes[i-iMin];
    }
  }

  //  std::cout << pParent << std::endl;

  pParent->SetFlag(NF_ALLCHILDREN, true);

  delete[] pChildNodes;
}

void CAlphabetManager::PopulateChildrenWithSymbol( CDasherNode *pNode, int iExistingSymbol, CDasherNode *pExistingChild ) {
  SAlphabetData *pParentUserData(static_cast<SAlphabetData *>(pNode->m_pUserData));

  // TODO: generally improve with iterators etc.
  // FIXME: this has to change for history stuff and Japanese dasher
  std::vector < symbol > newchars; // place to put this list of characters
  std::vector < unsigned int >cum; // for the probability list
  
  // TODO: Need to fix up relation to language model here (use one from node, not global).
  m_pNCManager->GetProbs(pParentUserData->iContext, newchars, cum, m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  int iChildCount = newchars.size();
  
  // work out cumulative probs in place
  for(int i = 1; i < iChildCount; i++) {
    cum[i] += cum[i - 1];
  }

  RecursiveIterateGroup(pNode, m_pNCManager->GetAlphabet()->m_pBaseGroup, &newchars, 
    &cum, 1, iChildCount, iExistingSymbol, pExistingChild);
}

void CAlphabetManager::ClearNode( CDasherNode *pNode ) {
  SAlphabetData *pUserData(static_cast<SAlphabetData *>(pNode->m_pUserData));

  pUserData->pLanguageModel->ReleaseContext(pUserData->iContext);
  delete pUserData;
}

void CAlphabetManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  symbol t = static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol;

  //std::cout << pNode << " " << pNode->Parent() << ": Output at offset " << static_cast<SAlphabetData *>(pNode->m_pUserData)->iOffset << " *" << m_pNCManager->GetAlphabet()->GetText(t) << "* " << std::endl;

  if(t) { // Ignore symbol 0 (root node)
    Dasher::CEditEvent oEvent(1, m_pNCManager->GetAlphabet()->GetText(t), static_cast<SAlphabetData *>(pNode->m_pUserData)->iOffset);
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
    Dasher::CEditEvent oEvent(2, m_pNCManager->GetAlphabet()->GetText(t), static_cast<SAlphabetData *>(pNode->m_pUserData)->iOffset);
    m_pNCManager->InsertEvent(&oEvent);
  }
}

// TODO: Sort out node deletion etc.
CDasherNode *CAlphabetManager::RebuildParent(CDasherNode *pNode) {
  int iOffset(static_cast<SAlphabetData *>(pNode->m_pUserData)->iOffset);
  int iNewOffset = iOffset - 1;

  CDasherNode *pNewNode;

  int iOldPhase(static_cast<SAlphabetData *>(pNode->m_pUserData)->iPhase);
  int iNewPhase;

  symbol iNewSymbol;

  std::string strContext;
  CLanguageModel::Context iContext;

  if((iOffset == -1) || (static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol == 0)) {
    // pNode is already a root, or for some other reason has the null
    // symbol (eg because it was generated from a different alphabet)
    return NULL;
  }
  else if(iOffset == 0) {
    // TODO: Creating a root node, Shouldn't be a special case
    iNewPhase = 0;
    iNewSymbol = 0;
    strContext = m_pNCManager->GetAlphabet()->GetDefaultContext();
    BuildContext(strContext, true, iContext, iNewSymbol);

    CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
    pDisplayInfo->iColour = 7; // TODO: Hard coded value
    pDisplayInfo->bShove = true;
    pDisplayInfo->bVisible = true;
    pDisplayInfo->strDisplayText = "";
    
    pNewNode = new CDasherNode(NULL, 0, 0, pDisplayInfo);
  }
  else {
    int iMaxContextLength = m_pLanguageModel->GetContextLength() + 1;

    int iStart = iOffset - iMaxContextLength;
    if(iStart < 0)
      iStart = 0;

    strContext = m_pInterface->GetContext(iStart, iOffset - iStart);

    BuildContext(strContext, false, iContext, iNewSymbol);

    iNewPhase = ((iOldPhase + 2 - 1) % 2);

    int iColour(m_pNCManager->GetColour(iNewSymbol, iNewPhase));
            
    CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
    pDisplayInfo->iColour = iColour;
    pDisplayInfo->bShove = true;
    pDisplayInfo->bVisible = true;
    pDisplayInfo->strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(iNewSymbol);

    // TODO: Node creation outside of if statement
    pNewNode = new CDasherNode(NULL, 0, 0, pDisplayInfo);
  }

  // TODO: Some of this context stuff could be consolidated
 
  pNewNode->m_pNodeManager = this;
  pNewNode->SetFlag(NF_SEEN, true);

  SAlphabetData *pNodeUserData = new SAlphabetData;
  pNewNode->m_pUserData = pNodeUserData;

  pNodeUserData->iOffset = iNewOffset;
  pNodeUserData->iPhase = iNewPhase;
  pNodeUserData->iSymbol = iNewSymbol;
  pNodeUserData->pLanguageModel = m_pLanguageModel;
  pNodeUserData->iContext = iContext;


  PopulateChildrenWithSymbol(pNewNode, static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol, pNode);
  
  //  std::cout << "**" << std::endl;

  return pNewNode;
}

// TODO: Shouldn't there be an option whether or not to learn as we write?
// For want of a better solution, game mode exemption explicit in this function
void CAlphabetManager::SetFlag(CDasherNode *pNode, int iFlag, bool bValue) {
  switch(iFlag) {
  case NF_COMMITTED:
    if(bValue && !pNode->GetFlag(NF_GAME) && m_pInterface->GetBoolParameter(BP_LM_ADAPTIVE))
      //static_cast<SAlphabetData *>(pNode->m_pUserData)->pLanguageModel->LearnSymbol(m_iLearnContext, static_cast<SAlphabetData *>(pNode->m_pUserData)->iSymbol);
    break;
  }
}

void CAlphabetManager::BuildContext(std::string strContext, bool bRoot, CLanguageModel::Context &oContext, symbol &iSymbol) {
  // Hopefully this will obsolete any need to handle contexts outside
  // of the alphabet manager - check this and remove resulting
  // redundant code

  std::vector<symbol> vContextSymbols;
  m_pNCManager->GetAlphabet()->GetSymbolsFull(&vContextSymbols, &strContext);
 
  oContext = m_pLanguageModel->CreateEmptyContext();
  
  for(std::vector<symbol>::iterator it(vContextSymbols.begin()); it != vContextSymbols.end(); ++it)
    if(*it != 0)
      m_pLanguageModel->EnterSymbol(oContext, *it);
  
  if((vContextSymbols.size() == 0) || bRoot)
    iSymbol = 0;
  else
    iSymbol = vContextSymbols[vContextSymbols.size() - 1];
}
