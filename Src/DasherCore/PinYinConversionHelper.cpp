
#ifndef _WIN32
#include "config.h"
#endif 

#include "LanguageModelling/LanguageModel.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include <ChiCEInterface.h>
#include <iostream>
#include <fstream>
#include "PinYinConversionHelper.h"

#include <SCENodeNew.h>

using namespace Dasher;

CPinYinConversionHelper::CPinYinConversionHelper(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, Dasher::CAlphIO *pCAlphIO){

  //TESTING FOR UTF-8 CHINESE IN C++ STRING
  /*
  FILE *fp;
  char* SimpChAlphabet = (char*)malloc(62); 
  fp = fopen("test.txt", "rw");
  fread(SimpChAlphabet, 1, 62, fp);
  fclose(fp);
  */

  const std::string SimpChAlphabet = "Chinese / 简体中文 (simplified chinese, in pin yin groups)";


  Dasher::CAlphIO::AlphInfo oAlphInfo = pCAlphIO->GetInfo(SimpChAlphabet);
  
  m_pAlphabet = new CAlphabet(oAlphInfo);

  //  std::cout<<SimpChAlphabet<<std::endl;
  //  for (int i = 0; i < 20; i++)
  //  std::cout<<m_pAlphabet->GetDisplayText(i)<<std::endl;

  // TODO: Need to figure out what this does - it was previously set to true in PopulateChildren

  CSymbolAlphabet alphabet(m_pAlphabet->GetNumberTextSymbols());
  alphabet.SetSpaceSymbol(m_pAlphabet->GetSpaceSymbol());      // FIXME - is this right, or do we have to do some kind of translation?
  alphabet.SetAlphabetPointer(m_pAlphabet);    // Horrible hack, but ignore for now.

  m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, alphabet);


  //Train the PPM Model for Chinese Alphabet
  TrainChPPM();



  //Old Code
  m_bTraceNeeded = true;//reset trace bool

  //clears the process phrase flags
  for(int i(0); i<MAX_HZ_NUM; i++)
    m_bPhrasesProcessed[i]=0;  


  BuildDataBase();
  CEInitialise();

}
  
bool CPinYinConversionHelper::Convert(const std::string &strSource, SCENode ** pRoot, int * childCount, int CMid) {

  SCENodeNew *pConversionList;
  int iHZCount;

  if(CEConvert (strSource.c_str(), &pConversionList, &iHZCount, CMid)){ 
    SCENodeNew *pHead(pConversionList);

    std::vector<SCENodeNew *> vHeads;

    while(pHead) {
      vHeads.push_back(pHead);
      pHead = pHead->pChild;
    }

    SCENode *pTail = NULL;
    SCENode *pNextTail = NULL;

    for(std::vector<SCENodeNew *>::reverse_iterator it(vHeads.rbegin()); it != vHeads.rend(); ++it) {
      SCENodeNew *pCurrentNode(*it);

      SCENode *pPreviousNode = NULL;

      while(pCurrentNode) {
	SCENode *pNewNode = new SCENode;
	
	if(pTail)
	  pNewNode->SetChild(pTail);

	if(pPreviousNode) {
	  pPreviousNode->SetNext(pNewNode);
	}
	else {
	  pNextTail = pNewNode;
	  pNextTail->Ref();
	}

	if(pPreviousNode)
	  pPreviousNode->Unref();

	pPreviousNode = pNewNode;
	pCurrentNode = pCurrentNode->pNext;
      }

      if(pPreviousNode)
	pPreviousNode->Unref();

      if(pTail)
	pTail->Unref();

      pTail = pNextTail;
  
    }

    *pRoot = pTail;


//     // TODO: Now need to convert...

//     *pRoot= pStart;

//     // Connect up the rest of the nodes to make a lattice
//     SCENode *pHead(pStart);
    
//     while(pHead) {
//       SCENode *pNewChild(pHead->GetChild());
//       SCENode *pCurrent(pHead->GetNext());
      
//       while(pCurrent) {
// 	pCurrent->SetChild(pNewChild);
// 	pCurrent = pCurrent->GetNext();
//       }
      
//       pHead = pHead->GetChild();
//     }
   
    return 1;
  }
  else{
    *pRoot = 0;
    return 0;
  }
}

unsigned int CPinYinConversionHelper::GetSumPYProbs(Dasher::CLanguageModel::Context context, SCENode * pPYCandStart, int norm){

  std::vector <unsigned int > Probs;
  unsigned int sumProb=0;
  
  m_pLanguageModel->GetProbs(context, Probs, norm);

  SCENode * pCurrentNode = pPYCandStart;

  while(pCurrentNode){
    
    std::vector <symbol >Symbols;
    std::string HZ = static_cast<std::string>(pCurrentNode->pszConversion);
    // Distribute the remaining space evenly
    
    m_pAlphabet->GetSymbols(&Symbols, &HZ, 0);    

    if(Symbols.size()!=0)
      sumProb += Probs[Symbols[0]];
    pCurrentNode = pCurrentNode->GetNext();

  }

  return sumProb;
}

void CPinYinConversionHelper::GetProbs(Dasher::CLanguageModel::Context context, std::vector < unsigned int >&Probs, int norm){
}
  
  


void CPinYinConversionHelper::AssignSizes(SCENode * pStart, CLanguageModel::Context context, long normalization, int uniform, int iNChildren){

  SCENode *pNode = pStart;

  std::vector <unsigned int > Probs;

  int iSymbols = m_pAlphabet->GetNumberSymbols(); 
  int iLeft(iNChildren);
  int iRemaining(normalization);

  int uniform_add;
  int nonuniform_norm;
  int control_space;
  int iNorm = normalization;
  

  //IGNORE CONTROL MODE FOR NOW
  //  if(!GetBoolParameter(BP_CONTROL_MODE)) {
  control_space = 0;
  uniform_add = ((iNorm * uniform) / 1000) / (iSymbols - 2);  // Subtract 2 from no symbols to lose control/root nodes
  nonuniform_norm = iNorm - (iSymbols - 2) * uniform_add;
    // }
    /*
      else {
      control_space = int (iNorm * 0.05);
      uniform_add = (((iNorm - control_space) * uniform / 1000) / (iSymbols - 2));        // Subtract 2 from no symbols to lose control/root nodes
      nonuniform_norm = iNorm - control_space - (iSymbols - 2) * uniform_add;
    }
    */


  //  context = m_pLanguageModel->CreateEmptyContext();

   //Testing Code for PYCHelper GetPYSumProbs
 
  /*
 CLanguageModel::Context iContext = m_pLanguageModel->CreateEmptyContext();

  SCENode * pTemp = pStart;
  while(pTemp){
    std::cout<<"test sum probs"<<GetSumPYProbs(iContext, pTemp, nonuniform_norm)<<std::endl;
    std::cout<<"test norm"<<nonuniform_norm<<std::endl;
    pTemp=pTemp->pChild;
  }
  */

  m_pLanguageModel->GetProbs(context, Probs, nonuniform_norm);

  /*  
  std::vector<unsigned int>::iterator it;
  for(it = Probs.begin();it!=Probs.end(); it++)
    std::cout<<*it<<",";
  
  std::cout<<"end"<<std::endl;
  */

  //  unsigned int sum; 


  unsigned long long int sumProb=0;

  std::vector <symbol >Symbols;
  std::string HZ;
  CLanguageModel::Context iCurrentContext;


  //std::cout<<"start"<<std::endl;
  while(pNode){

    Symbols.clear();
    HZ = static_cast<std::string>(pNode->pszConversion);
    m_pAlphabet->GetSymbols(&Symbols, &HZ, 0);    

    if(Symbols.size()!=0){
      pNode->Symbol = Symbols[0];
      //sumProb += Probs[Symbols[0]];

      
      iCurrentContext=m_pLanguageModel->CloneContext(context);
      m_pLanguageModel->EnterSymbol(iCurrentContext, pNode->Symbol);
      
      if(pStart->GetChild()){
	pNode->SumPYProbStore = GetSumPYProbs(iCurrentContext, pStart->GetChild(), nonuniform_norm);
	//std::cout<<"sumpyprobstore"<<pNode->SumPYProbStore<<std::endl;
      }
      else
	pNode->SumPYProbStore = 1;
	
      sumProb += (Probs[pNode->Symbol]*(pNode->SumPYProbStore));
      //std::cout<<"Probs[symbol]"<<Probs[Symbols[0]]<<std::endl;
      //std::cout<<"sumProbs"<<sumProb<<std::endl;
    }
    else
      pNode->Symbol = -1;

    pNode = pNode->GetNext();
  }



  pNode = pStart;
  while(pNode){
    /*
    std::vector <symbol >Symbols;
    std::string HZ = static_cast<std::string>(pNode->pszConversion);
 
    
    m_pAlphabet->GetSymbols(&Symbols, &HZ, 0);    
    */

    if(pNode->Symbol!=-1){
      if(sumProb!=0){

	//	iCurrentContext=m_pLanguageModel->CloneContext(context);
	//m_pLanguageModel->EnterSymbol(iCurrentContext, pNode->Symbol);
      
	
	pNode->NodeSize =static_cast<unsigned long long int>(Probs[pNode->Symbol])*(pNode->SumPYProbStore)*normalization/sumProb;

	/*
	std::cout<<"HZ"<<pNode->pszConversion<<std::endl;
	std::cout<<"Probs"<<Probs[pNode->Symbol]<<std::endl;
	std::cout<<"SumProbStore"<<pNode->SumPYProbStore<<std::endl;
	std::cout<<"above"<<Probs[pNode->Symbol]*(pNode->SumPYProbStore)<<std::endl;
	std::cout<<"sumprob"<<sumProb<<std::endl;
	std::cout<<"nodesize"<<pNode->NodeSize<<std::endl;
	*/
      }
    }
    else{
      pNode->NodeSize = 0;//hopefully this will be not be displayed
    }

    if(pNode->NodeSize < 1)
      pNode->NodeSize = 1;
    
    iRemaining -= pNode->NodeSize; 

    pNode = pNode->GetNext();
  }

  pNode = pStart;

  while(pNode){
    
    int iDiff(iRemaining / iLeft);
    
    pNode->NodeSize += iDiff;
    
    iRemaining -= iDiff;
    --iLeft;
    pNode = pNode->GetNext();
  }
  /*
  pNode = pStart;
  while(pNode){
    std::cout<<"size"<<pNode->NodeSize<<std::endl;
    pNode = pNode ->pNext;
  }
  */
  /*
  pNode = pStart;
  while(pNode){
    std::cout<<pNode->NodeSize<<",";
    pNode = pNode->pNext;
  }

  std::cout<<std::endl;
  */

  //std::cout<<catStr<<std::endl;
  
  //for(int i=0; i<Symbols.size(); i++)
  //  std::cout<<Symbols[i]<<",";

  //std::cout<<std::endl;
  
  //for(int i=0; i<Symbols.size();i++)
  //  std::cout<<m_pAlphabet->GetText(Symbols[i]);

  //std::cout<<std::endl;

  // pSizes[i] = m_pNCManager->GetLongParameter(LP_NORMALIZATION)*(100+5*freq[i])/(100*iNChildren+5*totalFreq);

}

bool CPinYinConversionHelper::GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid){
//   SCENode * pStart;
  
//   if(CEGetPhraseList(HZIndex, &pStart, CMid)){ 
   
//     *psOutput= pStart;
   
//     return 1;
//   }
//   else{
//     *psOutput = 0;
//     return 0;
//   }

  return 0;
}

void CPinYinConversionHelper::BuildDataBase(){
  
  std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > >IndexStack;
  std::vector<std::vector<std::vector<std::vector<int> > > >CandStack;
  std::vector<std::vector<std::vector<int> > > subIndexStack;
  std::vector<std::vector<int> > subCandStack;
  std::vector<int> cell;
  
  vContextData.clear();
  cell.push_back(0);
  subCandStack.push_back(cell);
  

    for(int i(0); i<MAX_HZ_NUM; i++){
      for(int j(0); j<MAX_CARE_CAND; j++){      
	for(int k(0); k<i+1; k++)	  
	  subIndexStack.push_back(subCandStack);
	CandStack.push_back(subIndexStack);
	subIndexStack.clear();
      }
      IndexStack.push_back(CandStack);
      CandStack.clear();
    }

    for(int k(0); k<MAX_CM_NUM; k++)
      vContextData.push_back(IndexStack);
}	


void CPinYinConversionHelper::ClearData(int CMid){
  
  std::vector<int>  cell;
  cell.push_back(0);

  for(int i(0); i<MAX_HZ_NUM; i++){
      for(int j(0); j<MAX_CARE_CAND/*(m_pRoot[i]->pChild->IsHeadAndCandNum)*/; j++){      
	for(int k(0); k<i+1; k++){
	  vContextData[CMid][i][j][k].clear();
	  vContextData[CMid][i][j][k].push_back(cell);
	}
      }
  }

}


std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * CPinYinConversionHelper::GetDP(int CMid){

  return &vContextData[CMid];

}



//THIS FUNCTION IS CALLED WHEN A SET OF CHILDREN IS BEING POPULATED
//AND BEFORE CALCULATING EACH NODE'S SCORE. THE POSITION IN THE
//SENTENCE IS GIVEN TO THE PY HELPER->LIBRARY AND A LIST OF PHRASES
//CORRESPONDING TO THE CHARACTER IS RETURNED. THESE PHRASES ARE
//PROCESSED INTO THE CONTEXT DATA IN PY HELPER
    
void CPinYinConversionHelper::ProcessPhrase(HZIDX HZIndex){
  
  SCENode * pPhraseList;
  SCENode * pNode;
  
  //  bool stop=0;

  int iIdx(0);
  int i;
  int score[m_iHZCount-HZIndex];

  CANDIDX CandIndex[m_iHZCount-HZIndex]; //list to store candidates
					 //returned from HZlookup,
					 //used to allocate data
  std::string strtemp;

  std::vector<int>  cell;

  if(!(GetPhraseList(HZIndex, &pPhraseList, m_iCMID)))
    return;

  if(pPhraseList->AcCharCount>4)
    pNode = pPhraseList->GetNext();
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
	  (*(GetDP(m_iCMID)))[HZIndex +i][CandIndex[i]][HZIndex].push_back(cell);
	cell.clear();
	
      }
    }

    pNode = pNode ->GetNext();
    iIdx ++;
  }
  m_bPhrasesProcessed[HZIndex]=1;
}


    
CANDIDX CPinYinConversionHelper::HZLookup(HZIDX HZIndex, const std::string &strSource){


  // this was done before candindex was put into node member, change
  // if have time

  //  int iIdx(0);
  if((HZIndex > m_iHZCount-1)||strSource.size()!=3)
    return -1;

  // TODO: Reimplement -----

//   SCENode * pNode = m_pRoot[HZIndex]->pChild;

//   while(pNode&&(iIdx<=MAX_CARE_CAND)){
    
//     if(strSource== pNode->pszConversion)
//       return iIdx;
//     pNode = pNode->pNext;
//     iIdx++;
//   }

  // -----
  
  return -1;
}

//CALCULATES SCORE OF A CERTAIN CANDIDATE HZ CHARACTER NODE TO BE
//POPULATED, FROM THE CONTEXT DATA IN PY HELPER. FINDS VTRACE TO MATCH
//CONTEXT SEQUENCE STORED IN THE LAST LEVEL OF DATABASE

int CPinYinConversionHelper::CalculateScore(CDasherNode * pNode, CANDIDX CandIndex){   
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

  for(std::vector<std::vector<std::vector<int> > >::iterator itIndex((*(GetDP(m_iCMID)))[HZIndex][CandIndex].begin()); itIndex!=(*(GetDP(m_iCMID)))[HZIndex][CandIndex].end();itIndex++){

    for(std::vector<std::vector<int> >::iterator itCand(itIndex->begin()); itCand!=itIndex->end(); itCand++){

      //IF HAS LEFT CONTEXT INFORMATION, MATCH VTRACE WITH CELL
      //POSITION 1

      if((*itCand).size() !=1){
	for(unsigned int i(0); i<(*itCand).size()-1; i++)
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


void CPinYinConversionHelper::TrainChPPM(){

  for(int i =0; i<10;i++)
    ProcessFile(i);

}

void CPinYinConversionHelper::ProcessFile(int index){


  CLanguageModel::Context trainContext;
  trainContext = m_pLanguageModel->CreateEmptyContext();

  FILE * fp;

  char strPath[200];
  
  const char* Alph="ABCDEFGHJKLMNPR";
 
  char str[4];
  std::string HZ;
  //  int  i, j, iLen;



  long pos=0;
  char cget;

  int trialcount=0;


  strcpy (strPath, (char *) getenv ("HOME"));
  strcat (strPath, "/training/corpus/character/");
  strcat (strPath, "C");
  strncat (strPath, Alph+index, 1);
  strcat (strPath, ".txt");

  printf("strPath is %s\n", strPath);

  fp = fopen (strPath, "rb");

  if (!fp)
    printf("cannot open file or incorrect directory\n");

  while(!feof(fp)){
    pos = ftell(fp);
    cget=fgetc(fp);
      
    //     printf("OXE$4= %d\n", (unsigned char)0xE4);  **228**
    //     printf("OXE9= %d\n", (unsigned char)0xE9);   **233**
    //printf("BEFORE print the integer code for unsigned char %d\n", (unsigned char) cget);
    while (((unsigned char)cget  < (unsigned char) 0xE4 || (unsigned char) cget > (unsigned char) 0xE9)&&!feof(fp)){

      if((unsigned char) cget > (unsigned char) 0xE9){
	 
	fseek(fp, pos, SEEK_SET);
	fread(str, sizeof(char)*3, 1, fp);
	str[3]='\0';
	pos = ftell(fp);
	cget = fgetc(fp);
	//printf("UNICODE SYMBOL/NUMBER |%s|\n", str);

      }
      else if(cget == 32){
	pos = ftell(fp);
	cget=fgetc(fp);
	//fputc(32, op);
      }
      else if(cget ==10){
	pos = ftell(fp);
	cget=fgetc(fp);
	//fputc(10, op);
      }
      else{   
	//printf("NON-UNICODE character |%c|\n", cget);

	pos = ftell(fp);
	cget=fgetc(fp);
	trialcount ++;
      }
    }
      
    if(!feof(fp)){
	
      fseek(fp, pos, SEEK_SET);
      fread(str, sizeof(char)*3, 1, fp);
      str[3]='\0';
      //printf("HZ |%s|\n",str);
      //	printf("first byte %d   ", (unsigned char)str[0]);
      //	printf("seconde byte %d\n", (unsigned char)str[1]);
      //	printf("third byte %d\n", (unsigned char)str[2]);

      HZ = static_cast<std::string>(str);
      
      //      std::cout<<"HZ is "<<HZ<<std::endl;

      std::vector<symbol> Sym;
      m_pAlphabet->GetSymbols(&Sym, &HZ, 0);
      
      if(Sym.size()!=0)
	m_pLanguageModel->LearnSymbol(trainContext, Sym[0]);
      else
	std::cout<<HZ<<"not found!"<<std::endl;

















      /*

  while(!feof(fp)){
   //   pos = ftell(fp);
    cget=fgetc(fp);

   if(!feof(fp)){
      fread(str, sizeof(char)*3, 1, fp);
      str[3]='\0';
      printf("HZ |%s|\n",str);
      //	printf("first byte %d   ", (unsigned char)str[0]);
      //	printf("seconde byte %d\n", (unsigned char)str[1]);
      //	printf("third byte %d\n", (unsigned char)str[2]);

      HZ = static_cast<std::string>(str);
      
      std::cout<<"HZ is "<<HZ<<std::endl;

      std::vector<symbol> Sym;
      m_pAlphabet->GetSymbols(&Sym, &HZ, 0);
      
      if(Sym.size()!=0)
	m_pLanguageModel->LearnSymbol(trainContext, Sym[0]);
      //else
	//	std::cout<<"not found!"<<std::endl;
 
      */
      
    }
  }
}
