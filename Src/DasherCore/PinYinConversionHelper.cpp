
#ifndef _WIN32
#include "config.h"
#endif 

#include "LanguageModelling/LanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "PinYinConversionHelper.h"


#include "SCENode.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace Dasher;

CPinYinConversionHelper::CPinYinConversionHelper(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, Dasher::CAlphIO *pCAlphIO, const std::string strCHAlphabetPath, CAlphabet * pAlphabet, CLanguageModel * pLanguageModel){

  m_pPYAlphabet = pAlphabet;
  m_pLanguageModel = static_cast<CPPMPYLanguageModel *>(pLanguageModel);
  // This section cleans initialises the Chinese character alphabet 

  const std::string CHAlphabet = "Chinese / 简体中文 (simplified chinese, in pin yin groups)";

  Dasher::CAlphIO::AlphInfo oAlphInfo = pCAlphIO->GetInfo(CHAlphabet);
  //Dasher::CAlphIO::AlphInfo oAlphInfopy = pCAlphIO->GetInfo(PYAlphabet);
  
  m_pCHAlphabet = new CAlphabet(oAlphInfo);
  //m_pPYAlphabet = new CAlphabet(oAlphInfopy);

  CSymbolAlphabet chalphabet(m_pCHAlphabet->GetNumberTextSymbols());
  // CSymbolAlphabet pyalphabet(m_pPYAlphabet->GetNumberTextSymbols());


  chalphabet.SetSpaceSymbol(m_pCHAlphabet->GetSpaceSymbol());     
  // pyalphabet.SetSpaceSymbol(m_pPYAlphabet->GetSpaceSymbol()); 

  chalphabet.SetAlphabetPointer(m_pCHAlphabet);  
  //pyalphabet.SetAlphabetPointer(m_pPYAlphabet);    
 
  //  m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, alphabet);

  //  std::cout<<"pyalphabet exists? "<<pyalphabet.GetSize()<<std::endl;

  m_iPriorityScale = 0;

  pParser = new CPinyinParser(strCHAlphabetPath);
}
  
bool CPinYinConversionHelper::Convert(const std::string &strSource, SCENode ** pRoot) {

  *pRoot = 0;
 
  return (pParser && pParser->Convert(strSource, pRoot));
}

unsigned int CPinYinConversionHelper::GetSumPYProbs(Dasher::CLanguageModel::Context context, SCENode * pPYCandStart, int norm){

  std::vector <unsigned int> Probs;
  unsigned int sumProb=0;
  
  m_pLanguageModel->GetProbs(context, Probs, norm, 0);

  SCENode * pCurrentNode = pPYCandStart;

  while(pCurrentNode){
    
    std::vector <symbol >Symbols;
    std::string HZ = static_cast<std::string>(pCurrentNode->pszConversion);
    // Distribute the remaining space evenly
    
    m_pCHAlphabet->GetSymbols(&Symbols, &HZ, 0);    

    if(Symbols.size()!=0)
      sumProb += Probs[Symbols[0]];
    pCurrentNode = pCurrentNode->GetNext();

  }

  return sumProb;
}

void CPinYinConversionHelper::GetProbs(Dasher::CLanguageModel::Context context, std::vector < unsigned int >&Probs, int norm){
}

void CPinYinConversionHelper::AssignSizes(SCENode **pStart, Dasher::CLanguageModel::Context context, long normalization, int uniform, int iNChildren){

  //test print:
  int print = 0;

  SCENode *pNewStart = *pStart;

  SCENode *pNode = pNewStart;


  //  std::cout<<"Head node "<<pNewStart->pszConversion<<std::endl;
  std::vector <unsigned int> Probs;

  int iSymbols = m_pCHAlphabet->GetNumberSymbols(); 
  int iLeft(iNChildren);
  int iRemaining(normalization);

  int uniform_add;
  int nonuniform_norm;
  int control_space;
  int iNorm = normalization;

  //Kept normalization base from old code   
  uniform_add = ((iNorm * uniform) / 1000) / (iSymbols - 2);  // Subtract 2 from no symbols to lose control/root nodes
  nonuniform_norm = iNorm - (iSymbols - 2) * uniform_add;
  
  unsigned long long int sumProb=0;

  std::vector <symbol> Symbols;
  std::vector <symbol> SymbolStore;

  CLanguageModel::Context iCurrentContext;

  //  int iNumSymbols =0;
  while(pNode){

    Symbols.clear();

    std::string HZ(pNode->pszConversion);

    m_pCHAlphabet->GetSymbols(&Symbols, &HZ, 0);    

    if(Symbols.size()!=0){
      pNode->Symbol = Symbols[0];
    }
    else
      pNode->Symbol = -1;

    pNode = pNode->GetNext();
  }

  //  std::cout<<"size of symbolstore "<<SymbolStore.size()<<std::endl;  
 
  //  std::cout<<"norm input: "<<nonuniform_norm/(iSymbols/iNChildren/100)<<std::endl;

  m_pLanguageModel->GetPartProbs(context, pStart, iNChildren, nonuniform_norm, 0);

  //std::cout<<"after get probs "<<std::endl;

 pNode = pNewStart;
 while(pNode){
    sumProb += pNode->NodeSize;

    //    std::cout<<"Nodesize: "<<pNode->NodeSize<<std::endl;
    pNode = pNode->GetNext();
 }

 //  std::cout<<"Sum Prob "<<sumProb<<std::endl;
 //  std::cout<<"norm "<<nonuniform_norm<<std::endl;

//Match, sumProbs = nonuniform_norm  
//but fix one element 'Da4'
// Finally, iterate through the nodes and actually assign the sizes.

  if(print)
    std::cout<<"sumProb "<<sumProb<<std::endl;

  pNode = pNewStart;
  while(pNode){
    if((pNode->Symbol!=-1)&&(sumProb!=0)){
      // std::cout<<"Probs size "<<pNode->NodeSize<<std::endl;
      pNode->NodeSize = static_cast<unsigned long long int>(pNode->NodeSize*(normalization/sumProb));//*(100 - m_iPriorityScale * pNode->GetPriority()) 
	//std::cout<<"Node size "<<pNode->NodeSize<<std::endl;
    }
    else{
      // TODO: Need to fix this branch - how do we end up here?
      // Is a symbol in pszConversion is not found in the alphabet
      pNode->NodeSize = 0;
    }

    if(print)
      std::cout<<"Nodesize "<<pNode->NodeSize<<std::endl;

    if(pNode->NodeSize < 1)
      pNode->NodeSize = 1;
    
    iRemaining -= pNode->NodeSize; 

    //  std::cout<<pNode->pszConversion<<std::endl;
    // std::cout<<pNode->Symbol<<std::endl;
    //    std::cout<<"Probs i "<<pNode<<std::endl;
    // std::cout<<"Symbol i"<<SymbolStore[iIdx]<<std::endl;
    // std::cout<<"symbols size "<<SymbolStore.size()<<std::endl;
    // std::cout<<"Symbols address "<<&SymbolStore<<std::endl;

    pNode = pNode->GetNext();
  }

  if(print)
    std::cout<<"iRemaining "<<iRemaining<<std::endl;


  // Last of all, allocate anything left over due to rounding error


  
  pNode = pNewStart;
  while(pNode){
    int iDiff(iRemaining / iLeft);
    
    pNode->NodeSize += iDiff;
    
    iRemaining -= iDiff;
    --iLeft;
    
    //    std::cout<<"Node size for "<<pNode->pszConversion<<std::endl;
    //std::cout<<"is "<<pNode->NodeSize<<std::endl;
    pNode = pNode->GetNext();
  }
  
}
  /*
  unsigned sumSize=0;
  pNode = pNewStart;
  while(pNode){
    sumSize+=pNode->NodeSize;
    pNode = pNode->GetNext();
  }
  */
//  if(print){
//    std::cout<<"Normalization "<<normalization<<std::endl;
    //    std::cout<<"SumNodesize is "<<sumSize<<std::endl;
    //   if(sumSize!=normalization)
    //    std::cout<<"Not equal! sum is "<<sumSize<<std::endl;
//  }



