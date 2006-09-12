#include "config.h"
#ifdef CHINESE

#include <ChiCEInterface.h>
#include <iostream>
#include "PinYinConversionHelper.h"


CPinYinConversionHelper::CPinYinConversionHelper(){

  BuildDataBase();
  CEInitialise();

  colourStore[0][0]=66;//light blue
  colourStore[0][1]=64;//very light green
  colourStore[0][2]=62;//light yellow
  colourStore[1][0]=78;//light purple
  colourStore[1][1]=81;//brownish
  colourStore[1][2]=60;//red


}
  
bool CPinYinConversionHelper::Convert(const std::string &strSource, SCENode ** pRoot, int * HZCount, int CMid) {

  SCENode * pStart;

  if(CEConvert (strSource.c_str(), &pStart, HZCount, CMid)){ 
    
    *pRoot= pStart;
   
    return 1;
  }
  else{
    *pRoot = 0;
    return 0;
  }
}


bool CPinYinConversionHelper::GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid){
  SCENode * pStart;
  
  if(CEGetPhraseList(HZIndex, &pStart, CMid)){ 
   
    *psOutput= pStart;
   
    return 1;
  }
  else{
    *psOutput = 0;
    return 0;
  }
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


int CPinYinConversionHelper::AssignColour(int parentClr, SCENode* pNode, int childIndex){

  int which = -1;
  

  for (int i=0; i<2; i++)
    for(int j=0; j<3; j++)
      if (parentClr == colourStore[i][j])
	which = i;
    
  if(which == -1)
    return colourStore[0][childIndex%3];
  else if(which == 0)
    return colourStore[1][childIndex%3];
  else 
    return colourStore[0][childIndex%3]; 
}

#endif
