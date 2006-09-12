#ifndef WIN32
#include "config.h"
#endif 

#include "ConversionManager.h"
#include "Event.h"
#include "EventHandler.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace Dasher;

CConversionManager::CConversionManager(CDasherModel *pModel, CLanguageModel *pLanguageModel, CConversionHelper *pHelper, int CMid) 
  : CNodeManager(2) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;
  m_pHelper = pHelper;
  
  m_iRefCount = 1;
  m_iCMID = CMid;
  m_iHZCount = 0;

  m_bTreeBuilt = false; 
  
  //clears the process phrase flags
  for(int i(0); i<MAX_HZ_NUM; i++)
    m_bPhrasesProcessed[i]=0;  
}

CConversionManager::~CConversionManager(){  
  for (int i(0);i<m_iHZCount; i++)
    RecursiveDelTree(m_pRoot[i]);
}




CDasherNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  // TODO: Parameters here are placeholders - need to figure out what's right
  pNewNode = new CDasherNode(pParent, m_pModel->GetStartConversionSymbol(), 0, Opts::Nodes2, iLower, iUpper, m_pLanguageModel, 9);
 
  // FIXME - handle context properly
  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pNodeManager->Ref();

  pNewNode->m_pUserData = 0;
  pNewNode->m_strDisplayText = "Convert";
  pNewNode->m_bShove = false;
  pNewNode->m_pBaseGroup = 0;

  return pNewNode;
}

void CConversionManager::PopulateChildren( CDasherNode *pNode ) {

  if(!m_pModel)
    return;

  if(!m_bTreeBuilt) {
    BuildTree(pNode);
    m_bTreeBuilt = true;
  }

  m_bTraceNeeded = true;//reset trace bool

  CDasherNode *pNewNode;

  SCENode *pCurrentSCENode(static_cast<SCENode *>(pNode->m_pUserData));
    
  if(pCurrentSCENode == 0){ 
    if(m_pRoot)
      pCurrentSCENode = m_pRoot[0];
  }
  else{

    if((pCurrentSCENode->AcCharCount != m_iHZCount)&&(!pCurrentSCENode->pChild))
      pCurrentSCENode ->pChild = m_pRoot[pCurrentSCENode->AcCharCount]->pChild;

    else if(pCurrentSCENode->AcCharCount == m_iHZCount)
      pCurrentSCENode->pChild =0;
      // std::cout<<"signal  "<<pCurrentSCENode->pChild<<std::endl;
      // if(pCurrentSCENode->pChild)
      //	std::cout<<"signal  "<<pCurrentSCENode->pChild->pszConversion<<std::endl;
   

    //else if (pCurrentSCENode->pChild) already has children
    //if complete, has pChild is NULL

  }
  
  SCENode *pCurrentSCEChild;
  
  if(pCurrentSCENode)
    pCurrentSCEChild = pCurrentSCENode->pChild;
  else
    pCurrentSCEChild = 0;
  
  if(pCurrentSCEChild) {
  
    if(m_iHZCount>1)
      if(!m_bPhrasesProcessed[pCurrentSCEChild->AcCharCount-1])
   	if(pCurrentSCEChild->AcCharCount<m_iHZCount)
	  ProcessPhrase(pCurrentSCEChild->AcCharCount-1);
    
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




    //TESTING FOR CALCULATESCORE STAGE 1
    //int test;
    //test = CalculateScore(pNode, 1);
    //std::cout<<"current character"<<pCurrentSCENode->pszConversion<<std::endl;
    //std::cout<<"the score for the second candidate is"<<test<<std::endl;



    //ASSIGNING SCORES AND CALCULATING NODE SIZE
    //Ph: feel free to edit this part to make it more structured
    int iSize[pCurrentSCEChild->IsHeadAndCandNum];
    int score[pCurrentSCEChild->IsHeadAndCandNum];
    int total =0;
    int max = 0;
    int CandNum = pCurrentSCEChild -> IsHeadAndCandNum;

    int iRemaining(m_pModel->GetLongParameter(LP_NORMALIZATION));



    for(int i(0); i < pCurrentSCEChild->IsHeadAndCandNum; ++i){
      
      score[i] = CalculateScore(pNode, i);
      total += score[i];
      if(i!=0)
	if (score[i]>score[i-1])
	  max = score[i];
    }

    for(int i(0); i < pCurrentSCEChild->IsHeadAndCandNum; ++i) {


      //TESTING FOR RESIZING FREQUENT HZ CHARACTERS
      //if(i<5)
      //	if(score[i]<max-5){
      //	  std::cout<<"first scores are"<<score[i]<<std::endl;
      //  score[i]=max-5;
      //	}

      if(CandNum == 1)
      	iSize[i] = m_pModel->GetLongParameter(LP_NORMALIZATION);
      else
	iSize[i] = m_pModel->GetLongParameter(LP_NORMALIZATION)*((CandNum-i-1)+2*CandNum*score[i])/(CandNum*(CandNum-1)/2+2*CandNum*total);
      
      //PREVIOUS MODEL: m_pModel->GetLongParameter(LP_NORMALIZATION)/((i + 1) * (i + 2));
      
      if(iSize[i] < 1)
	iSize[i] == 1;
      
      iRemaining -= iSize[i];
    }

    int iLeft(pCurrentSCEChild->IsHeadAndCandNum);
    
    for(int i(0); i < pCurrentSCEChild->IsHeadAndCandNum; ++i) {
      int iDiff(iRemaining / iLeft);

      iSize[i] += iDiff;
      
      iRemaining -= iDiff;
      --iLeft;
    }

    int iIdx(0);
    int iCum(0);
  
    int parentClr = pNode->Colour();

    do {
//       int iLbnd( iIdx*(m_pModel->GetLongParameter(LP_NORMALIZATION)/pCurrentCMNode->m_iNumChildren)); 
//       int iHbnd( (iIdx+1)*(m_pModel->GetLongParameter(LP_NORMALIZATION)/pCurrentCMNode->m_iNumChildren)); 

      int iLbnd(iCum);
      int iHbnd(iCum + iSize[iIdx]);

      iCum = iHbnd;
      
      // TODO: Parameters here are placeholders - need to figure out
      // what's right
      
      pNewNode = new CDasherNode(pNode, m_pModel->GetStartConversionSymbol(), 0, Opts::Nodes2, iLbnd, iHbnd, m_pLanguageModel, m_pHelper->AssignColour(parentClr, pCurrentSCEChild, iIdx));
      
      // FIXME - handle context properly
      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      
      pNewNode->m_pNodeManager = this;
      pNewNode->m_pNodeManager->Ref();

      pNewNode->m_pUserData = pCurrentSCEChild;
      pNewNode->m_strDisplayText = pCurrentSCEChild->pszConversion;
      pNewNode->m_bShove = true;
      pNewNode->m_pBaseGroup = 0;
      
      pNode->Children().push_back(pNewNode);

      pCurrentSCEChild = pCurrentSCEChild->pNext;
      ++iIdx;
    }while(pCurrentSCEChild);
			     
    
  }

  else {
    // TODO: Placeholder algorithm here
    // TODO: Add an 'end of conversion' node?
      int iLbnd(0);
    int iHbnd(m_pModel->GetLongParameter(LP_NORMALIZATION)); 
      
    pNewNode = m_pModel->GetRoot(0, pNode, iLbnd, iHbnd, NULL);
    pNewNode->Seen(false);
      
    pNode->Children().push_back(pNewNode);
    //    pNode->SetHasAllChildren(false);
  }
}

void CConversionManager::ClearNode( CDasherNode *pNode ) {
  // TODO: Need to implement this
  
  pNode->m_pNodeManager->Unref();
}

void CConversionManager::BuildTree(CDasherNode *pRoot) {
  CDasherNode *pCurrentNode(pRoot->Parent());
 
  SCENode * pStartTemp;
  std::string strCurrentString;
  bool ConversionSuccess;
   m_pHelper->ClearData(m_iCMID);

  while(pCurrentNode) {
    if(pCurrentNode->m_pNodeManager->GetID() == 2)
      break;
    
    ///  std::cout<<pCurrentNode->m_strDisplayText<<"   "<<pCurrentNode<<"   "<<pCurrentNode->Parent()<<std::endl;

    // TODO: Need to make this the edit text rather than the display text
    strCurrentString = pCurrentNode->m_strDisplayText + strCurrentString;
    pCurrentNode = pCurrentNode->Parent();
  }


  ConversionSuccess = m_pHelper ->Convert(strCurrentString, &pStartTemp , &m_iHZCount, m_iCMID); 
  
  //m_iHZCount returned from Convert is not 100% dependable(BUT THE
  //FOLLOWING CODE SHOULD FIX IT)
  //std::cout << m_iHZCount << std::endl;

  if((!ConversionSuccess)||(m_iHZCount==0))
    m_pRoot = 0;
  
  else{

    if(m_iHZCount>MAX_HZ_NUM)
      m_iHZCount = MAX_HZ_NUM;
      
    m_pRoot = new SCENode *[m_iHZCount];
    
    int i;
    
    for(i=0; (i< m_iHZCount)&&(pStartTemp); i++){
      m_pRoot[i] = new SCENode;
      
      m_pRoot[i]->pszConversion = "Convert";
      m_pRoot[i]->pChild = pStartTemp;
      pStartTemp = pStartTemp->pChild;
      m_pRoot[i]->pNext = 0;
      m_pRoot[i]->IsHeadAndCandNum = 0;
    }
    
    if(m_pRoot[0]&&(i!=m_iHZCount))
      m_iHZCount = i;
  } 
}
    

//THIS FUNCTION IS CALLED WHEN A SET OF CHILDREN IS BEING POPULATED
//AND BEFORE CALCULATING EACH NODE'S SCORE. THE POSITION IN THE
//SENTENCE IS GIVEN TO THE PY HELPER->LIBRARY AND A LIST OF PHRASES
//CORRESPONDING TO THE CHARACTER IS RETURNED. THESE PHRASES ARE
//PROCESSED INTO THE CONTEXT DATA IN PY HELPER
    
void CConversionManager::ProcessPhrase(HZIDX HZIndex){
  
  SCENode * pPhraseList;
  SCENode * pNode;
  
  bool stop=0;

  int iIdx(0);
  int i;
  int score[m_iHZCount-HZIndex];

  CANDIDX CandIndex[m_iHZCount-HZIndex]; //list to store candidates
					 //returned from HZlookup,
					 //used to allocate data
  std::string strtemp;

  std::vector<int>  cell;

  if(!(m_pHelper->GetPhraseList(HZIndex, &pPhraseList, m_iCMID)))
    return;

  if(pPhraseList->AcCharCount>4)
    pNode = pPhraseList->pNext;
  else 
    pNode = pPhraseList;

  while((pNode)&&(iIdx<=MAX_CARE_PHRASE)){

    //this section needs research. What scores would be a good estimate.

    switch(pNode->AcCharCount){
    case 2:
      score[0] = 2;
      score[1] = 3;
      break;
    case 3:
      score[0] = 3;
      score[1] = 4;
      score[2] = 5;
      break;
    case 4:
      score[0] = 4;
      score[1] = 5;
      score[2] = 6;
      score[3] = 7;
      break;
    default:
      for(int j(0); j< m_iHZCount-HZIndex; j++)
	score[j] = 5+j;
      break;
	}
    
    
    for(i=0 ; (i<pNode->AcCharCount); i++){
      strtemp=pNode->pszConversion;

      //TESTING
      //std::cout<<"accharcount"<<pNode->AcCharCount<<std::endl;
      //std::cout<<"the cut string is"<<strtemp.substr(3*i,3)<<std::endl;
      //std::cout<<"list to look from
      //is"<<m_pRoot[HZIndex+i]->pChild->pszConversion<<std::endl;


      CandIndex[i] = HZLookup(HZIndex+i, strtemp.substr(3*i, 3));
      
      //TESTING
      //std::cout<<"the lookup is"<<CandIndex[i]<<std::endl;

      if(CandIndex[i]==-1)
	break;
      else{
	cell.push_back(score[i]);
	for(int j(0); j< i; j++)
	  cell.push_back(CandIndex[i-j-1]);
	
	//
	//say the phrase is XYZ(this) push back in each cell in the
	//order: score, Z, Y, X so as to match with vTrace in
	//calculatescore

	if(!(HZIndex + i> MAX_HZ_NUM -1))	
	  (*(m_pHelper->GetDP(m_iCMID)))[HZIndex +i][CandIndex[i]][HZIndex].push_back(cell);
	cell.clear();
	
      }
    }

    pNode = pNode ->pNext;
    iIdx ++;
  }
  m_bPhrasesProcessed[HZIndex]=1;
}


    
CANDIDX CConversionManager::HZLookup(HZIDX HZIndex, const std::string &strSource){


  // this was done before candindex was put into node member, change
  // if have time

  int iIdx(0);
  if((HZIndex > m_iHZCount-1)||strSource.size()!=3)
    return -1;

  SCENode * pNode = m_pRoot[HZIndex]->pChild;

  while(pNode&&(iIdx<=MAX_CARE_CAND)){
    
    if(strSource== pNode->pszConversion)
      return iIdx;
    pNode = pNode->pNext;
    iIdx++;
  }
  
  return -1;
}

//CALCULATES SCORE OF A CERTAIN CANDIDATE HZ CHARACTER NODE TO BE
//POPULATED, FROM THE CONTEXT DATA IN PY HELPER. FINDS VTRACE TO MATCH
//CONTEXT SEQUENCE STORED IN THE LAST LEVEL OF DATABASE

int CConversionManager::CalculateScore(CDasherNode * pNode, CANDIDX CandIndex){   
  CDasherNode *pIterateDNode(pNode);
  SCENode *pTemp;

  HZIDX HZIndex;
  int score=0;

  bool addtick=1; //bool to signal add score



  //THIS SECTION IS TO FIND VTRACE, IN THE SAME WAY AS FINDING INPUT
  //PY STRING

  if(m_bTraceNeeded){
    vTrace.clear();
    
    while(pIterateDNode&&(pIterateDNode->m_pNodeManager->GetID() == 2)) {
      
      pTemp=static_cast<SCENode*>(pIterateDNode->m_pUserData);
      
      if(!pTemp)
	pIterateDNode=0;
      else{
	vTrace.push_back(pTemp->CandIndex);
	pIterateDNode= pIterateDNode->Parent();
      }
    }
    
    //THE FOLLOWING IS TESTING FOR VTRACE
    // if(vTrace.size()!=0){
    // std::cout<<"signal"<<std::endl;
    // for(std::vector<int>::iterator it(vTrace.begin());it!=vTrace.end();it++)
    //  std::cout<< *it <<std::endl;  
    //}

  }
  m_bTraceNeeded= false;



  
  if(CandIndex>=MAX_CARE_CAND)
    return 0;



  pTemp=static_cast<SCENode*>(pNode->m_pUserData);
  
  if(pTemp)
    HZIndex = pTemp->AcCharCount;
  else
    HZIndex = 0;


  //THE DATA IS CONSTRUCTED OF UNITS OF SINGLE CELLS STORING CONTEXT SEQUENCE
  //AND A CORRESPONDING SCORE 

  //LEVEL 1 : HZ INDEX : NUMBER OF CHARACTERS CONVERTED
  //LEVEL 2 : CAND INDEX : NUMBER OF CANDIDATES WITH EACH POSITION
  //LEVEL 3 : SUB HZ INDEX :(COULD BE REDUNDANT) CORRESPONDES TO
  //          INDEX OF CHARACTERS WHICH WERE CONSISTED IN PHRASES
  //LEVEL 4 : SUB CAND INDEX : WHICH CAND WAS IN THE PHRASE 
  //LEVLE 5 : CELL: STORING PHASES IN HZ INDEX AND ASSIGNED SCORE
  //          IN THE WAY: 1.SCORE 2.Z 3.Y 4.X FOR PHRASE XYZ
  //          PREVIOUSLY PROCESSED

  for(std::vector<std::vector<std::vector<int> > >::iterator itIndex((*(m_pHelper->GetDP(m_iCMID)))[HZIndex][CandIndex].begin()); itIndex!=(*(m_pHelper->GetDP(m_iCMID)))[HZIndex][CandIndex].end();itIndex++){

    for(std::vector<std::vector<int> >::iterator itCand(itIndex->begin()); itCand!=itIndex->end(); itCand++){

      //IF HAS LEFT CONTEXT INFORMATION, MATCH VTRACE WITH CELL
      //POSITION 1

      if((*itCand).size() !=1){
	for(int i(0); i<(*itCand).size()-1; i++)
	  if((*itCand)[i+1]!=vTrace[i]){
	    addtick = 0;
	    break;
	  }
	  if(addtick)
	    score+=(*itCand)[0];
	  addtick =1;
      }
      else
	score+=(*itCand)[0];
    }
  }
    
  return score;
} 


void CConversionManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  m_pModel->m_bContextSensitive = true; 

  SCENode *pCurrentSCENode(static_cast<SCENode *>(pNode->m_pUserData));

  if(pCurrentSCENode) {
    Dasher::CEditEvent oEvent(1, pCurrentSCENode->pszConversion);
    m_pModel->InsertEvent(&oEvent);
    
    if((pNode->GetChildren())[0]->m_pNodeManager != this) {
      Dasher::CEditEvent oEvent(11, "");
      m_pModel->InsertEvent(&oEvent);
    }
  }
  else {
    Dasher::CEditEvent oEvent(10, "");
    m_pModel->InsertEvent(&oEvent);
  }
}

void CConversionManager::Undo( CDasherNode *pNode ) {
  SCENode *pCurrentSCENode(static_cast<SCENode *>(pNode->m_pUserData));

  if(pCurrentSCENode) {
    if(strlen(pCurrentSCENode->pszConversion) > 0) {
      Dasher::CEditEvent oEvent(2, pCurrentSCENode->pszConversion);
      m_pModel->InsertEvent(&oEvent);
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
