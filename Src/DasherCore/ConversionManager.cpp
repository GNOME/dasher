#ifndef WIN32
#include "config.h"
#endif 

#include "ConversionManager.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace Dasher;

CConversionManager::CConversionManager(CNodeCreationManager *pNCManager, CConversionHelper *pHelper, int CMid) 
  : CNodeManager(2) {
  m_pNCManager = pNCManager;
  m_pHelper = pHelper;
   
//DOESN'T SEEM INTRINSIC
//and check why pHelper may be empty
  if(pHelper)
    m_pLanguageModel = pHelper->GetLanguageModel();

  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();

  m_iRefCount = 1; // TODO: Is this the right way to handle this, or should we initilise to 0 and enforce a reference from the creator?
  m_iCMID = CMid;
  //  m_iHZCount = 0;

  m_bTreeBuilt = false; 
}

CConversionManager::~CConversionManager(){  
  //  for (int i(0);i<m_iHZCount; i++)
  RecursiveDelTree(m_pRoot[0]);
}


CDasherNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  // TODO: Parameters here are placeholders - need to figure out what's right
  pNewNode = new CDasherNode(pParent, m_pNCManager->GetStartConversionSymbol(), 0, Opts::Nodes2, iLower, iUpper, NULL, 9);
 
  // FIXME - handle context properly
  // TODO: Reimplemnt -----
  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
  // -----

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pNodeManager->Ref();


  SConversionData *pNodeUserData = new SConversionData;
  pNewNode->m_pUserData = pNodeUserData;
  pNodeUserData->pLanguageModel = m_pHelper->GetLanguageModel();

  pNewNode->m_strDisplayText = "Convert";
  pNewNode->SetShove(false);
  pNewNode->m_pBaseGroup = 0;

  CLanguageModel::Context iContext;

  // std::cout<<m_pLanguageModel<<"lala"<<std::endl;
  if(m_pLanguageModel){
    iContext = m_pLanguageModel->CreateEmptyContext();
    pNodeUserData->iContext = iContext;
  }

  pNodeUserData->pSCENode = 0;

  return pNewNode;
}

// TODO: This function needs to be significantly tidied up
// TODO: get rid of pSizes

void CConversionManager::AssignChildSizes(SCENode *pNode, CLanguageModel::Context context, int iNChildren) {
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

  if(!m_pNCManager)
    return;

  // Do the conversion and build the tree (lattice) if it hasn't been
  // done already.

  if(!m_bTreeBuilt) {
    BuildTree(pNode);
    m_bTreeBuilt = true;
  }

  CDasherNode *pNewNode;

  
  
  SConversionData * pCurrentDataNode (static_cast<SConversionData *>(pNode->m_pUserData));
  SCENode *pCurrentSCEChild;
  
  if(pCurrentDataNode->pSCENode)
    pCurrentSCEChild = pCurrentDataNode->pSCENode->pChild;
  else {
    if(m_pRoot)
      pCurrentSCEChild = m_pRoot[0];
    else
      pCurrentSCEChild = 0;
  }
  
  if(pCurrentSCEChild) {

    // TODO: Reimplement (in subclass) -----
    
//     if(m_iHZCount>1)
//       if(!m_bPhrasesProcessed[pCurrentSCEChild->AcCharCount-1])
//    	if(pCurrentSCEChild->AcCharCount<m_iHZCount)
// 	  ProcessPhrase(pCurrentSCEChild->AcCharCount-1);

    // -----

    //int *iSize;
    
    //    iSize = new int[pCurrentSCEChild->IsHeadAndCandNum];


    
    AssignChildSizes(pCurrentSCEChild, pCurrentDataNode->iContext, pCurrentSCEChild->IsHeadAndCandNum);

    int iIdx(0);
    int iCum(0);
  
    int parentClr = pNode->Colour();

    // Finally loop through and create the children

    do {
      int iLbnd(iCum);
      int iHbnd(iCum + pCurrentSCEChild->NodeSize);

      iCum = iHbnd;
      
      // TODO: Parameters here are placeholders - need to figure out
      // what's right
      
      pNewNode = new CDasherNode(pNode, m_pNCManager->GetStartConversionSymbol(), 0, Opts::Nodes2, iLbnd, iHbnd, NULL, m_pHelper->AssignColour(parentClr, pCurrentSCEChild, iIdx));
      
      // TODO: Reimplement ----

      // FIXME - handle context properly
      //      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      // -----
      
      pNewNode->m_pNodeManager = this;
      pNewNode->m_pNodeManager->Ref();

      SConversionData *pNodeUserData = new SConversionData;
      pNodeUserData ->pSCENode = pCurrentSCEChild;
      pNodeUserData->pLanguageModel = pCurrentDataNode->pLanguageModel;

      CLanguageModel::Context iContext;
      iContext = pCurrentDataNode->pLanguageModel->CloneContext(pCurrentDataNode->iContext);
      if(pCurrentSCEChild ->Symbol !=-1)
	pNodeUserData->pLanguageModel->EnterSymbol(iContext, pCurrentSCEChild->Symbol); // TODO: Don't use symbols?
      
      
      pNodeUserData->iContext = iContext;
      
      pNewNode->m_pUserData = pNodeUserData;
      // SAlphabetData *pNodeUserData = new SAlphabetData;
      
      //pNewNode->m_pUserData = pNodeUserData;
      
      //pNodeUserData->iPhase = iNewPhase;
      //pNodeUserData->iSymbol = iIdx;

      pNewNode->m_strDisplayText = pCurrentSCEChild->pszConversion;
      pNewNode->SetShove(false);
      pNewNode->m_pBaseGroup = 0;
      
      pNode->Children().push_back(pNewNode);

      pCurrentSCEChild = pCurrentSCEChild->pNext;
      ++iIdx;
    }while(pCurrentSCEChild);


    // delete[] iSize;
  }

  else {
    // TODO: Placeholder algorithm here
    // TODO: Add an 'end of conversion' node?
    int iLbnd(0);
    int iHbnd(m_pNCManager->GetLongParameter(LP_NORMALIZATION)); 
      
    pNewNode = m_pNCManager->GetRoot(0, pNode, iLbnd, iHbnd, NULL);
    pNewNode->SetFlag(NF_SEEN, false);
      
    pNode->Children().push_back(pNewNode);
    //    pNode->SetHasAllChildren(false);
  }
}

void CConversionManager::ClearNode( CDasherNode *pNode ) {
  pNode->m_pNodeManager->Unref();
}

void CConversionManager::RecursiveDumpTree(SCENode *pCurrent, unsigned int iDepth) {
  while(pCurrent) {
    for(unsigned int i(0); i < iDepth; ++i) 
      std::cout << "-";
    
    std::cout << " " << pCurrent->pszConversion << " " << pCurrent->IsHeadAndCandNum << " " << pCurrent->CandIndex << " " << pCurrent->IsComplete << " " << pCurrent->AcCharCount << std::endl;

    RecursiveDumpTree(pCurrent->pChild, iDepth + 1);
    pCurrent = pCurrent->pNext;
  }
}

void CConversionManager::BuildTree(CDasherNode *pRoot) {
  CDasherNode *pCurrentNode(pRoot->Parent());
 
  std::string strCurrentString;
  if(m_pHelper)
    m_pHelper->ClearData(m_iCMID);

  while(pCurrentNode) {
    if(pCurrentNode->m_pNodeManager->GetID() == 2)
      break;

    // TODO: Need to make this the edit text rather than the display text
    strCurrentString = pCurrentNode->m_strDisplayText + strCurrentString;
    pCurrentNode = pCurrentNode->Parent();
  }

  // TODO: The remainder of this function is messy - to be sorted out
  int iHZCount;
  SCENode *pStartTemp;
  bool ConversionSuccess;

  ConversionSuccess = m_pHelper ->Convert(strCurrentString, &pStartTemp , &iHZCount, m_iCMID); 

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

  SCENode *pCurrentSCENode(static_cast<SCENode *>(pNode->m_pUserData));

  if(pCurrentSCENode) {
    Dasher::CEditEvent oEvent(1, pCurrentSCENode->pszConversion);
    m_pNCManager->InsertEvent(&oEvent);
    
    if((pNode->GetChildren())[0]->m_pNodeManager != this) {
      Dasher::CEditEvent oEvent(11, "");
      m_pNCManager->InsertEvent(&oEvent);
    }
  }
  else {
    Dasher::CEditEvent oEvent(10, "");
    m_pNCManager->InsertEvent(&oEvent);
  }
}

void CConversionManager::Undo( CDasherNode *pNode ) {
  SCENode *pCurrentSCENode(static_cast<SCENode *>(pNode->m_pUserData));

  if(pCurrentSCENode) {
    if(strlen(pCurrentSCENode->pszConversion) > 0) {
      Dasher::CEditEvent oEvent(2, pCurrentSCENode->pszConversion);
      m_pNCManager->InsertEvent(&oEvent);
    }
  }
}

bool CConversionManager::RecursiveDelTree(SCENode* pNode){

  SCENode * pTemp;

  if(!pNode)
    return 0;
  else if(pNode->pChild)
    return RecursiveDelTree(pNode->pChild);
  else{

    while(!pNode->pChild){
      pTemp = pNode->pNext;
      delete pNode;
      pNode = pTemp;
      if(!pNode)
	return 1;
    }
    return RecursiveDelTree(pNode->pChild);
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
    
     
      if(s!=-1)
	pLan->LearnSymbol(m_iLearnContext, s);
    }
    break;
  }
}
