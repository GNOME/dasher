// ConversionManager.cpp
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

#ifndef WIN32
#include "config.h"
#endif 

#include "ConversionManager.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace Dasher;

CConversionManager::CConversionManager(CNodeCreationManager *pNCManager, CConversionHelper *pHelper, CAlphabet *pAlphabet, int CMid) 
  : CNodeManager(2) {

  m_pNCManager = pNCManager;
  m_pHelper = pHelper;
  m_pAlphabet = pAlphabet;
  m_pRoot = NULL;
   
//DOESN'T SEEM INTRINSIC
//and check why pHelper may be empty
  if(pHelper)
    m_pLanguageModel = pHelper->GetLanguageModel();
  else
    m_pLanguageModel = NULL;

  if(m_pLanguageModel)
    m_iLearnContext = m_pLanguageModel->CreateEmptyContext();

  m_iRefCount = 1;
  m_iCMID = CMid;
  //  m_iHZCount = 0;

  m_bTreeBuilt = false; 
}

CConversionManager::~CConversionManager(){  
  if(m_pRoot && *m_pRoot)
    (*m_pRoot)->Unref();
}


CDasherNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  int iOffset = *(static_cast<int *>(pUserData));
 
  // TODO: Parameters here are placeholders - need to figure out what's right

  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = 9; // TODO: Hard coded value
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = true;
  pDisplayInfo->strDisplayText = ">"; // TODO: Hard coded value, needs i18n
       
  pNewNode = new CDasherNode(pParent, iLower, iUpper, pDisplayInfo);
 
  // FIXME - handle context properly
  // TODO: Reimplemnt -----
  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
  // -----

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pNodeManager->Ref();


  SConversionData *pNodeUserData = new SConversionData;
  pNewNode->m_pUserData = pNodeUserData;
  pNodeUserData->bType = false;
  pNodeUserData->iOffset = iOffset + 1;

  if(m_pHelper)
    pNodeUserData->pLanguageModel = m_pHelper->GetLanguageModel();
  else
    pNodeUserData->pLanguageModel = NULL;
  
  //   // std::cout<<m_pLanguageModel<<"lala"<<std::endl;
  if(m_pLanguageModel) {
    CLanguageModel::Context iContext;
    iContext = m_pLanguageModel->CreateEmptyContext();
    pNodeUserData->iContext = iContext;
  }

  pNodeUserData->pSCENode = 0;

  return pNewNode;
}

// TODO: This function needs to be significantly tidied up
// TODO: get rid of pSizes

void CConversionManager::AssignChildSizes(SCENode **pNode, CLanguageModel::Context context, int iNChildren) {
  DASHER_ASSERT(m_pHelper);

    // Calculate sizes for the children. Note that normalisation is
    // done additiviely rather than multiplicatively, so it's not
    // quite what was originally planned (but I don't think this is
    // much of a problem). More serious is the fact that the ordering
    // is being lost when the tree is created, as nodes begininning
    // with the same character are merged. This needs to be though
    // out, but the probabilities should probably be done at the time
    // of construction of the candidate tree rather than the Dasher
    // tree (aside - is there any real point having two separate trees
    // - surely we should just create Dasher nodes right away?).
    //
    // The algorithm should also allow for the possibility of the
    // conversion engine returning probabilities itself, which should
    // be used in preference to the values infered from the ordering
    //
    // Finally, maybe the choices should be presented in lexographic
    // order, rather than in order returned (really not sure about
    // this - it needs to be thought through).


    //    std::cout << "b" << std::endl;

    //TESTING FOR CALCULATESCORE STAGE 1
    //int test;
    //test = CalculateScore(pNode, 1);
    //std::cout<<"current character"<<pCurrentSCENode->pszConversion<<std::endl;
    //std::cout<<"the score for the second candidate is"<<test<<std::endl;



    //ASSIGNING SCORES AND CALCULATING NODE SIZE
    //Ph: feel free to edit this part to make it more structured
//     int iSize[pCurrentSCEChild->IsHeadAndCandNum];
//     int score[pCurrentSCEChild->IsHeadAndCandNum];
//     int total =0;
//     int max = 0;
//     int CandNum = pCurrentSCEChild -> IsHeadAndCandNum;

// CHANGE    int iRemaining(m_pNCManager->GetLongParameter(LP_NORMALIZATION));

    // Thoughts on the general idea here - this is very close to being
    // a fully fledged language model, so I think we should go with
    // that idea, but maybe we need something mode flexible. I'd
    // imagine:
    //
    // 1. Probabilities provided directly with translation? Maybe hard
    // to represent in the lattice itself. 
    //
    // 2. Full n-gram language model provided - in general assign
    // probabilities to paths through the lattice
    // 
    // 3. Ordered results, but no probabilities - using a power law
    // rule or the like.
    //
    // Tempted to assume (1) and (2) can be implemented together, with
    // a second call to the library at node creation time, and (3) can
    // be implemented as a fallback if that doesn't work.
    //
    // Things to be thought out:
    // - How to deal with contexts - backtrace at time of call or stored in node?
    // - Sharing of language model infrastructure?
    


    // Lookup scores for each of the children

    // TODO: Reimplement -----
    
//     for(int i(0); i < pCurrentSCEChild->IsHeadAndCandNum; ++i){
//       score[i] = CalculateScore(pNode, i);
//       total += score[i];
//       if(i!=0)
// 	if (score[i]>score[i-1])
// 	  max = score[i];
//     }

    // -----

    // Use the scores to calculate the size of the nodes


  iNChildren = 0;
  SCENode *pChild(*pNode);

  while(pChild) {
    pChild = pChild->GetNext();
    ++iNChildren;
  }


  m_pHelper->AssignSizes(pNode, context, m_pNCManager->GetLongParameter(LP_NORMALIZATION), m_pNCManager->GetLongParameter(LP_UNIFORM), iNChildren);

}
    //    for(int i(0); i < iNChildren; ++i) {

      //TESTING FOR RESIZING FREQUENT HZ CHARACTERS
      //if(i<5)
      //	if(score[i]<max-5){
      //	  std::cout<<"first scores are"<<score[i]<<std::endl;
      //  score[i]=max-5;
      //	}

      // TODO: Reimplement new model -----

//       if(CandNum == 1)
//       	iSize[i] = m_pNCManager->GetLongParameter(LP_NORMALIZATION);
//       else
// 	iSize[i] = m_pNCManager->GetLongParameter(LP_NORMALIZATION)*((CandNum-i-1)+2*CandNum*score[i])/(CandNum*(CandNum-1)/2+2*CandNum*total);
      
      // ----

      //PREVIOUS MODEL: m_pNCManager->GetLongParameter(LP_NORMALIZATION)/((i + 1) * (i + 2));


      /*
      SCENode * pIt;
      
      pIt=pNode;

      uint freq[iNChildren];
      for(int i(0); i<iNChildren; i++)
	freq[i] = 0;
      uint totalFreq=0;
      uint maxFreq=0;

    
      while(pIt){
	freq[pIt->CandIndex]=pIt->HZFreq;
	totalFreq+=freq[pIt->CandIndex];
	if(pIt->HZFreq>maxFreq)
	  maxFreq=pIt->HZFreq;
	pIt = pIt->pNext;
      }

      pSizes[i] = m_pNCManager->GetLongParameter(LP_NORMALIZATION)*(100+5*freq[i])/(100*iNChildren+5*totalFreq);
      */
	//((i + 1) * (i + 2));

    /*

      if(pSizes[i] < 1)
	pSizes[i] = 1;
      
      iRemaining -= pSizes[i];
    }

    // Distribute the remaining space evenly

    int iLeft(iNChildren);
    
    for(int i(0); i < iNChildren; ++i) {
      int iDiff(iRemaining / iLeft);

      pSizes[i] += iDiff;
      
      iRemaining -= iDiff;
      --iLeft;
    }
}

    */

void CConversionManager::PopulateChildren( CDasherNode *pNode ) {
  DASHER_ASSERT(m_pNCManager);

  SConversionData * pCurrentDataNode (static_cast<SConversionData *>(pNode->m_pUserData));
  CDasherNode *pNewNode;

  // If no helper class is present then just drop straight back to an
  // alphabet root. This should only happen in error cases, and the
  // user should have been warned here.
  //
  if(!m_pHelper) {
    int iLbnd(0);
    int iHbnd(m_pNCManager->GetLongParameter(LP_NORMALIZATION)); 
    
    CAlphabetManager::SRootData oRootData;
    oRootData.szContext = NULL;
    oRootData.iOffset = pCurrentDataNode->iOffset + 1;

    pNewNode = m_pNCManager->GetRoot(0, pNode, iLbnd, iHbnd, &oRootData);
    pNewNode->SetFlag(NF_SEEN, false);
    
    pNode->Children().push_back(pNewNode);

    return;
  }

  // Do the conversion and build the tree (lattice) if it hasn't been
  // done already.
  //
  if(!m_bTreeBuilt) {
    BuildTree(pNode);
    m_bTreeBuilt = true;
  }

  SCENode *pCurrentSCEChild;

  if(pCurrentDataNode->pSCENode)
    pCurrentSCEChild = pCurrentDataNode->pSCENode->GetChild();
  else {
    if(m_pRoot && !pCurrentDataNode->bType)
      pCurrentSCEChild = m_pRoot[0];
    else
      pCurrentSCEChild = 0;
  }
  
  if(pCurrentSCEChild) {

    //    std::cout << "Current SCE Child: " << pCurrentSCEChild << std::endl;

    // TODO: Reimplement (in subclass) -----
    
//     if(m_iHZCount>1)
//       if(!m_bPhrasesProcessed[pCurrentSCEChild->AcCharCount-1])
//    	if(pCurrentSCEChild->AcCharCount<m_iHZCount)
// 	  ProcessPhrase(pCurrentSCEChild->AcCharCount-1);

    // -----

    //int *iSize;
    
    //    iSize = new int[pCurrentSCEChild->IsHeadAndCandNum];

    

    
    AssignChildSizes(&pCurrentSCEChild, pCurrentDataNode->iContext, pCurrentSCEChild->IsHeadAndCandNum);

    int iIdx(0);
    int iCum(0);
  
    //    int parentClr = pNode->Colour();
    // TODO: Fixme
    int parentClr = 0;

    // Finally loop through and create the children

    do {
      //      std::cout << "Current scec: " << pCurrentSCEChild << std::endl;

      int iLbnd(iCum);
      int iHbnd(iCum + pCurrentSCEChild->NodeSize);

      iCum = iHbnd;
      
      // TODO: Parameters here are placeholders - need to figure out
      // what's right
      

      CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
      pDisplayInfo->iColour = m_pHelper->AssignColour(parentClr, pCurrentSCEChild, iIdx);
      pDisplayInfo->bShove = true;
      pDisplayInfo->bVisible = true;
      
      //  std::cout << "#" << pCurrentSCEChild->pszConversion << "#" << std::endl;

      pDisplayInfo->strDisplayText = pCurrentSCEChild->pszConversion;
       
      pNewNode = new CDasherNode(pNode, iLbnd, iHbnd, pDisplayInfo);
      
      // TODO: Reimplement ----

      // FIXME - handle context properly
      //      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      // -----
      
      pNewNode->m_pNodeManager = this;
      pNewNode->m_pNodeManager->Ref();

      SConversionData *pNodeUserData = new SConversionData;
      pNodeUserData->bType = false;
      pNodeUserData->pSCENode = pCurrentSCEChild;
      pNodeUserData->pLanguageModel = pCurrentDataNode->pLanguageModel;
      pNodeUserData->iOffset = pCurrentDataNode->iOffset + 1;

      if(pCurrentDataNode->pLanguageModel) {
	CLanguageModel::Context iContext;
	iContext = pCurrentDataNode->pLanguageModel->CloneContext(pCurrentDataNode->iContext);
	
	if(pCurrentSCEChild ->Symbol !=-1)
	  pNodeUserData->pLanguageModel->EnterSymbol(iContext, pCurrentSCEChild->Symbol); // TODO: Don't use symbols?
      
      
	pNodeUserData->iContext = iContext;
      }
      
      pNewNode->m_pUserData = pNodeUserData;
      // SAlphabetData *pNodeUserData = new SAlphabetData;
      
      //pNewNode->m_pUserData = pNodeUserData;
      
      //pNodeUserData->iPhase = iNewPhase;
      //pNodeUserData->iSymbol = iIdx;
      
      pNode->Children().push_back(pNewNode);

      pCurrentSCEChild = pCurrentSCEChild->GetNext();
      ++iIdx;
    }while(pCurrentSCEChild);


    // delete[] iSize;
  }

  else {
    if((static_cast<SConversionData *>(pNode->m_pUserData))->bType) {
      // TODO: Placeholder algorithm here
      // TODO: Add an 'end of conversion' node?
      int iLbnd(0);
      int iHbnd(m_pNCManager->GetLongParameter(LP_NORMALIZATION)); 
      
      CAlphabetManager::SRootData oRootData;
      oRootData.szContext = NULL;
      oRootData.iOffset = pCurrentDataNode->iOffset;
   
      pNewNode = m_pNCManager->GetRoot(0, pNode, iLbnd, iHbnd, &oRootData);
      pNewNode->SetFlag(NF_SEEN, false);
      
      pNode->Children().push_back(pNewNode);
      //    pNode->SetHasAllChildren(false);
    }
    else {
      int iLbnd(0);
      int iHbnd(m_pNCManager->GetLongParameter(LP_NORMALIZATION)); 

      CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
      pDisplayInfo->iColour = m_pHelper->AssignColour(0, pCurrentSCEChild, 0);
      pDisplayInfo->bShove = true;
      pDisplayInfo->bVisible = true;
      pDisplayInfo->strDisplayText = "";
       
      pNewNode = new CDasherNode(pNode, iLbnd, iHbnd, pDisplayInfo);
      
      // TODO: Reimplement ----

      // FIXME - handle context properly
      //      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      // -----
      
      pNewNode->m_pNodeManager = this;
      pNewNode->m_pNodeManager->Ref();

      SConversionData *pNodeUserData = new SConversionData;
      pNodeUserData->bType = true;
      pNodeUserData->pSCENode = NULL;
      pNodeUserData->pLanguageModel = pCurrentDataNode->pLanguageModel;
      pNodeUserData->iOffset = pCurrentDataNode->iOffset + 1;
     
      pNewNode->m_pUserData = pNodeUserData;

      pNewNode->SetFlag(NF_SEEN, false);
      
      pNode->Children().push_back(pNewNode);
    }
  }
}

void CConversionManager::ClearNode( CDasherNode *pNode ) {
  if(pNode->m_pUserData)
    delete (SConversionData *)(pNode->m_pUserData);
}

void CConversionManager::RecursiveDumpTree(SCENode *pCurrent, unsigned int iDepth) {
  while(pCurrent) {
    for(unsigned int i(0); i < iDepth; ++i) 
      std::cout << "-";
    
    std::cout << " " << pCurrent->pszConversion << " " << pCurrent->IsHeadAndCandNum << " " << pCurrent->CandIndex << " " << pCurrent->IsComplete << " " << pCurrent->AcCharCount << std::endl;

    RecursiveDumpTree(pCurrent->GetChild(), iDepth + 1);
    pCurrent = pCurrent->GetNext();
  }
}

void CConversionManager::BuildTree(CDasherNode *pRoot) {
  DASHER_ASSERT(m_pHelper);

  CDasherNode *pCurrentNode(pRoot->Parent());
 
  std::string strCurrentString;
  //  m_pHelper->ClearData(m_iCMID);

  while(pCurrentNode) {
    if(pCurrentNode->m_pNodeManager->GetID() == 2) {
      break;
    }

    // TODO: Need to make this the edit text rather than the display text
    CAlphabetManager::SAlphabetData *pAlphabetData = 
      static_cast<CAlphabetManager::SAlphabetData *>(pCurrentNode->m_pUserData);

    strCurrentString = m_pAlphabet->GetText(pAlphabetData->iSymbol) + strCurrentString;

    pCurrentNode = pCurrentNode->Parent();
  }

  // TODO: The remainder of this function is messy - to be sorted out
  int iHZCount;
  SCENode *pStartTemp;
  bool ConversionSuccess;

  ConversionSuccess = m_pHelper->Convert(strCurrentString, &pStartTemp , &iHZCount, m_iCMID); 

  if((!ConversionSuccess)||(iHZCount==0)) {
    m_pRoot = 0;
  }
  else{
    m_pRoot = new SCENode *[1];
    m_pRoot[0] = pStartTemp;
  } 
}
    
void CConversionManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  // TODO: Reimplement this
  //  m_pNCManager->m_bContextSensitive = true; 

  SCENode *pCurrentSCENode((static_cast<SConversionData *>(pNode->m_pUserData))->pSCENode);

  if(pCurrentSCENode) {
    Dasher::CEditEvent oEvent(1, pCurrentSCENode->pszConversion, static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
    m_pNCManager->InsertEvent(&oEvent);
    
    if((pNode->GetChildren())[0]->m_pNodeManager != this) {
      Dasher::CEditEvent oEvent(11, "", 0);
      m_pNCManager->InsertEvent(&oEvent);
    }
  }
  else {
    if((static_cast<SConversionData *>(pNode->m_pUserData))->bType) {
      Dasher::CEditEvent oOPEvent(1, "|", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
    else {
      Dasher::CEditEvent oOPEvent(1, ">", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
    
    Dasher::CEditEvent oEvent(10, "", 0);
    m_pNCManager->InsertEvent(&oEvent);
  }
}

void CConversionManager::Undo( CDasherNode *pNode ) {  
  SCENode *pCurrentSCENode((static_cast<SConversionData *>(pNode->m_pUserData))->pSCENode);

  if(pCurrentSCENode) {
    if(pCurrentSCENode->pszConversion && (strlen(pCurrentSCENode->pszConversion) > 0)) {
      Dasher::CEditEvent oEvent(2, pCurrentSCENode->pszConversion, static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oEvent);
    }
  } 
  else {
    if((static_cast<SConversionData *>(pNode->m_pUserData))->bType) {
      Dasher::CEditEvent oOPEvent(2, "|", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
    else {
      Dasher::CEditEvent oOPEvent(2, ">", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
  }
}

void CConversionManager::SetFlag(CDasherNode *pNode, int iFlag, bool bValue) {
  switch(iFlag) {
  case NF_COMMITTED:
    if(bValue){
      // TODO: Reimplement (need a learning context, check whether
      // symbol actually corresponds to character)

      CLanguageModel * pLan =  static_cast<SConversionData *>(pNode->m_pUserData)->pLanguageModel;

      SCENode * pSCENode = static_cast<SConversionData *>(pNode->m_pUserData)->pSCENode; 

      if(!pSCENode)
	return;

      symbol s =pSCENode ->Symbol;
    
     
      if((s!=-1) && m_pLanguageModel)
	pLan->LearnSymbol(m_iLearnContext, s);
    }
    break;
  }
}
