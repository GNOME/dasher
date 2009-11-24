#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include "LanguageModelling/LanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "PinYinConversionHelper.h"
#include "NodeCreationManager.h"

#include "SCENode.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace Dasher;

CPinYinConversionHelper::CPinYinConversionHelper(CNodeCreationManager *pNCManager, Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, Dasher::CAlphIO *pCAlphIO, const std::string strCHAlphabetPath, CAlphabet * pAlphabet, CPPMPYLanguageModel *pLanguageModel)
: CConversionHelper(pNCManager,pAlphabet,pLanguageModel) {

  m_pPYAlphabet = pAlphabet;
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

void CPinYinConversionHelper::BuildTree(CConvHNode *pRoot) {
  DASHER_ASSERT(pRoot->bisRoot);

  // Find the pinyin (roman) text (stored in Display text) of the
  // previous alphabet node.

  // Get pinyin string (to translate) from 'Display Text' in the
  // alphabet file (refer to alphabet.spyDict.xml).

  std::string strCurrentString(m_pAlphabet->GetDisplayText(pRoot->iSymbol));

  SCENode *pStartTemp;
  Convert(strCurrentString, &pStartTemp);

  // Store all conversion trees (SCENode trees) in the pUserData->pSCENode
  // of each Conversion Root.

  pRoot->pSCENode = pStartTemp;
}

bool CPinYinConversionHelper::Convert(const std::string &strSource, SCENode ** pRoot) {

  *pRoot = 0;
  return (pParser && pParser->Convert(strSource, pRoot));
}

unsigned int CPinYinConversionHelper::GetSumPYProbs(Dasher::CLanguageModel::Context context, std::vector<SCENode *> &pPYCandStart, int norm){

  std::vector <unsigned int> Probs;
  unsigned int sumProb=0;
  
  GetLanguageModel()->GetProbs(context, Probs, norm, 0);

  for (std::vector<SCENode *>::iterator it = pPYCandStart.begin(); it != pPYCandStart.end(); it++) {
    SCENode *pCurrentNode(*it);
    
    std::vector <symbol >Symbols;
    std::string HZ = static_cast<std::string>(pCurrentNode->pszConversion);
    // Distribute the remaining space evenly
    
    m_pCHAlphabet->GetSymbols(Symbols, HZ);    

    if(Symbols.size()!=0)
      sumProb += Probs[Symbols[0]];

  }

  return sumProb;
}

void CPinYinConversionHelper::GetProbs(Dasher::CLanguageModel::Context context, std::vector < unsigned int >&Probs, int norm){
}

void CPinYinConversionHelper::AssignSizes(const std::vector<SCENode *> &vChildren, Dasher::CLanguageModel::Context context, long normalization, int uniform){

  //test print:
  int print = 0;

  //  std::cout<<"Head node "<<pNewStart->pszConversion<<std::endl;
  std::vector <unsigned int> Probs;

  int iSymbols = m_pCHAlphabet->GetNumberSymbols(); 
  int iLeft(vChildren.size());
  int iRemaining(normalization);

  int uniform_add;
  int nonuniform_norm;
  int iNorm = normalization;

  //Kept normalization base from old code   
  uniform_add = ((iNorm * uniform) / 1000) / (iSymbols - 2);  // Subtract 2 from no symbols to lose control/root nodes
  nonuniform_norm = iNorm - (iSymbols - 2) * uniform_add;
  
  unsigned long long int sumProb=0;

  std::vector <symbol> Symbols;
  std::vector <symbol> SymbolStore;

  //CLanguageModel::Context iCurrentContext;

  //  int iNumSymbols =0;
  for (std::vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    SCENode *pNode(*it);
    Symbols.clear();

    std::string HZ(pNode->pszConversion);

    m_pCHAlphabet->GetSymbols(Symbols, HZ);    

    if(Symbols.size()!=0){
      pNode->Symbol = Symbols[0];
    }
    else
      pNode->Symbol = -1;
  }

  //  std::cout<<"size of symbolstore "<<SymbolStore.size()<<std::endl;  
 
  //  std::cout<<"norm input: "<<nonuniform_norm/(iSymbols/iNChildren/100)<<std::endl;

  GetLanguageModel()->GetPartProbs(context, vChildren, nonuniform_norm, 0);

  //std::cout<<"after get probs "<<std::endl;

  for (std::vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    sumProb += (*it)->NodeSize;
 }

 //  std::cout<<"Sum Prob "<<sumProb<<std::endl;
 //  std::cout<<"norm "<<nonuniform_norm<<std::endl;

//Match, sumProbs = nonuniform_norm  
//but fix one element 'Da4'
// Finally, iterate through the nodes and actually assign the sizes.

  if(print)
    std::cout<<"sumProb "<<sumProb<<std::endl;

  for (std::vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    SCENode *pNode(*it);
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
  }

  if(print)
    std::cout<<"iRemaining "<<iRemaining<<std::endl;


  // Last of all, allocate anything left over due to rounding error


  
  for (std::vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    SCENode *pNode(*it);
    int iDiff(iRemaining / iLeft);
    
    pNode->NodeSize += iDiff;
    
    iRemaining -= iDiff;
    --iLeft;
    
    //    std::cout<<"Node size for "<<pNode->pszConversion<<std::endl;
    //std::cout<<"is "<<pNode->NodeSize<<std::endl;
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

CPinYinConversionHelper::CPYConvNode *CPinYinConversionHelper::makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo) {
  return new CPYConvNode(pParent, iLbnd, iHbnd, pDispInfo, this);
}

CPinYinConversionHelper::CPYConvNode::CPYConvNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CPinYinConversionHelper *pMgr)
: CConvHNode(pParent, iLbnd, iHbnd, pDispInfo, pMgr) {
};

void CPinYinConversionHelper::CPYConvNode::SetFlag(int iFlag, bool bValue)
{
	//Blanked out for new Mandarin Dasher, if we want to have the language model learn as one types, need to work on this part
	if (iFlag == NF_COMMITTED && bValue)
    CConvNode::SetFlag(iFlag, bValue); //skip CConvHNode::SetFlag, which does learn-as-you-type
  else
    CConvHNode::SetFlag(iFlag, bValue);
}

CLanguageModel::Context CPinYinConversionHelper::CPYConvNode::GetConvContext() {
  return iContext;
}

void CPinYinConversionHelper::CPYConvNode::SetConvSymbol(int _iSymbol) {
  iSymbol = _iSymbol;
}
