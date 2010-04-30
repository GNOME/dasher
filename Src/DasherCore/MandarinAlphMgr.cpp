// MandarinAlphMgr.cpp
//
// Copyright (c) 2009 The Dasher Team
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

#include "MandarinAlphMgr.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "DasherInterfaceBase.h"
#include "DasherNode.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"


#include <vector>
#include <sstream>
#include <iostream>

using namespace std;
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

CMandarinAlphMgr::CMandarinAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel)
  : CAlphabetManager(pInterface, pNCManager, pLanguageModel),
    m_pParser(new CPinyinParser(pInterface->GetStringParameter(SP_SYSTEM_LOC) +"/alphabet.chineseRuby.xml")),
    m_pCHAlphabet(new CAlphabet(pInterface->GetInfo("Chinese / 简体中文 (simplified chinese, in pin yin groups)"))) {
}

CMandarinAlphMgr::~CMandarinAlphMgr() {
  delete m_pParser;
}

CDasherNode *CMandarinAlphMgr::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {

  if (iSymbol <= 1288) {
    //Will wrote:
    //Modified for Mandarin Dasher
    //The following logic switch allows punctuation nodes in Mandarin to be treated in the same way as English (i.e. display and populate next round) instead of invoking a conversion node
    //ACL I think by "the following logic switch" he meant that symbols <= 1288 are "normal" nodes, NOT punctuation nodes,
    // whereas punctuation is handled by the fallthrough case (standard AlphabetManager CreateSymbolNode)

    /*old code:
     * CDasherNode *pNewNode = m_pNCManager->GetConvRoot(pParent, iLbnd, iHbnd, pParent->m_iOffset+1);
	   * static_cast<CPinYinConversionHelper::CPYConvNode *>(pNewNode)->SetConvSymbol(iSymbol);
	   * return pNewNode;
     */

    //from ConversionManager:
    CDasherNode::SDisplayInfo *pInfo = new CDasherNode::SDisplayInfo;
    pInfo->bVisible = true;
    pInfo->bShove = true;
    pInfo->strDisplayText = "";
    pInfo->iColour = 9;
    //CTrieNode parallels old PinyinConversionHelper's SetConvSymbol:
    CConvRoot *pNewNode = new CConvRoot(pParent, iLbnd, iHbnd, pInfo, this, m_pParser->GetTrieNode(m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol)));

    //keep same offset, as we still haven't entered/selected a definite symbol
    pNewNode->m_iOffset = pParent->m_iOffset;

    //from ConversionHelper:
    //pNewNode->m_pLanguageModel = m_pLanguageModel;
    pNewNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);

	  return pNewNode;
  }
  return CAlphabetManager::CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd);
}

CMandarinAlphMgr::CConvRoot::CConvRoot(CDasherNode *pParent, unsigned int iLbnd, unsigned int iHbnd, SDisplayInfo *pDisplayInfo, CMandarinAlphMgr *pMgr, CTrieNode *pTrie)
: CDasherNode(pParent, iLbnd, iHbnd, pDisplayInfo), m_pMgr(pMgr), m_pTrie(pTrie) {
  
}

int CMandarinAlphMgr::CConvRoot::ExpectedNumChildren() {
  if (m_vChInfo.empty()) BuildConversions();
  return m_vChInfo.size();
}

void CMandarinAlphMgr::CConvRoot::BuildConversions() {
  if (!m_pTrie || !m_pTrie->list()) {
    //TODO some kind of fallback??? e.g. start new char?
    DASHER_ASSERT(false);
    return;
  }
  for(set<string>::iterator it = m_pTrie->list()->begin(); it != m_pTrie->list()->end(); ++it) {
    std::vector<symbol> vSyms;
    m_pMgr->m_pCHAlphabet->GetSymbols(vSyms, *it);
    DASHER_ASSERT(vSyms.size()==1); //does it ever happen? if so, Will's code would effectively push -1
    DASHER_ASSERT(m_pMgr->m_pCHAlphabet->GetText(vSyms[0]) == *it);
    m_vChInfo.push_back(std::pair<symbol, unsigned int>(vSyms[0],0));
  }
  //TODO would be nicer to do this only if we need the size info (i.e. PopulateChildren not ExpectedNumChildren) ?
  m_pMgr->AssignSizes(m_vChInfo, iContext);
}

void CMandarinAlphMgr::CConvRoot::PopulateChildren() {
  if (m_vChInfo.empty()) BuildConversions();
  
  int iIdx(0);
  int iCum(0);
  
  //    int parentClr = pNode->Colour();
  // TODO: Fixme
  int parentClr = 0;
  // Finally loop through and create the children
  for (vector<pair<symbol, unsigned int> >::const_iterator it = m_vChInfo.begin(); it!=m_vChInfo.end(); it++) {
    //      std::cout << "Current scec: " << pCurrentSCEChild << std::endl;
    unsigned int iLbnd(iCum);
    unsigned int iHbnd(iCum + it->second);
    
    iCum = iHbnd;
    
    // TODO: Parameters here are placeholders - need to figure out
    // what's right    
    
    CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
    pDisplayInfo->iColour = (m_vChInfo.size()==1) ? GetDisplayInfo()->iColour : m_pMgr->AssignColour(parentClr, iIdx);
    pDisplayInfo->bShove = true;
    pDisplayInfo->bVisible = true;
    
    //  std::cout << "#" << pCurrentSCEChild->pszConversion << "#" << std::endl;
    
    //The chinese characters are in the _text_ (not label - that's e.g. "liang4")
    // of the alphabet (& the pszConversion from PinyinParser was converted to symbol
    // by CAlphabet::GetSymbols, which does string->symbol by _text_; we're reversing that)
    pDisplayInfo->strDisplayText = m_pMgr->m_pCHAlphabet->GetText(it->first);
    
    CMandNode *pNewNode = new CMandSym(this, iLbnd, iHbnd, pDisplayInfo, m_pMgr, it->first);
    
    // TODO: Reimplement ----
    
    // FIXME - handle context properly
    //      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
    // -----
    
    pNewNode->m_iOffset = m_iOffset + 1;
    
    pNewNode->iContext = m_pMgr->m_pLanguageModel->CloneContext(this->iContext);
      
    m_pMgr->m_pLanguageModel->EnterSymbol(iContext, it->first); // TODO: Don't use symbols?      
      
    DASHER_ASSERT(GetChildren().back()==pNewNode);
    
    ++iIdx;
  }
  
}

void CMandarinAlphMgr::AssignSizes(std::vector<pair<symbol,unsigned int> > &vChildren, Dasher::CLanguageModel::Context context) {

  const uint64 iNorm(m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  const unsigned int uniform((m_pNCManager->GetLongParameter(LP_UNIFORM)*iNorm)/1000);
  
  int iRemaining(iNorm);
  
  uint64 sumProb=0;
  
  //CLanguageModel::Context iCurrentContext;
  
  //  std::cout<<"size of symbolstore "<<SymbolStore.size()<<std::endl;  
  
  //  std::cout<<"norm input: "<<nonuniform_norm/(iSymbols/iNChildren/100)<<std::endl;
  
  //ACL pass in iNorm and uniform directly - GetPartProbs distributes the last param between
  // however elements there are in vChildren...
  static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->GetPartProbs(context, vChildren, iNorm, uniform);
  
  //std::cout<<"after get probs "<<std::endl;
  
  for (std::vector<pair<symbol,unsigned int> >::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    sumProb += it->second;
  }
  
  //  std::cout<<"Sum Prob "<<sumProb<<std::endl;
  //  std::cout<<"norm "<<nonuniform_norm<<std::endl;
  
  //Match, sumProbs = nonuniform_norm  
  //but fix one element 'Da4'
  // Finally, iterate through the nodes and actually assign the sizes.
  
 // std::cout<<"sumProb "<<sumProb<<std::endl;
  
  for (std::vector<pair<symbol,unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    DASHER_ASSERT(it->first>-1); //ACL Will's code tested for both these conditions explicitly, and if so 
    DASHER_ASSERT(sumProb>0);   //then used a probability of 0. I don't think either
                                //should ever happen if the alphabet files are right (there'd have to
                                //be either no conversions of the syllable+tone, or else the LM'd have
                                //to assign zero probability to each), so I'm removing these tests for now...
    iRemaining -= it->second = (it->second*iNorm)/sumProb;
    if (it->second==0) {
#ifdef DEBUG
      std::cout << "WARNING: Erasing zero-probability conversion with symbol " << it->first << std::endl;
#endif
      vChildren.erase(it--);
    }
    //  std::cout<<pNode->pszConversion<<std::endl;
    // std::cout<<pNode->Symbol<<std::endl;
    //    std::cout<<"Probs i "<<pNode<<std::endl;
    // std::cout<<"Symbol i"<<SymbolStore[iIdx]<<std::endl;
    // std::cout<<"symbols size "<<SymbolStore.size()<<std::endl;
    // std::cout<<"Symbols address "<<&SymbolStore<<std::endl;
  }
  
  //std::cout<<"iRemaining "<<iRemaining<<std::endl;
  
  
  // Last of all, allocate anything left over due to rounding error
  
  int iLeft(vChildren.size());
  
  for (std::vector<pair<symbol,unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    int iDiff(iRemaining / iLeft);
    
    it->second += iDiff;
    
    iRemaining -= iDiff;
    --iLeft;
    
    //    std::cout<<"Node size for "<<pNode->pszConversion<<std::endl;
    //std::cout<<"is "<<pNode->NodeSize<<std::endl;
  }
  
  DASHER_ASSERT(iRemaining == 0);
  
}

static int colourStore[2][3] = {
  {66,//light blue
    64,//very light green
    62},//light yellow
  {78,//light purple
    81,//brownish
    60},//red
};

//Pulled from CConversionHelper, where it's described as "needing a rethink"...
int CMandarinAlphMgr::AssignColour(int parentClr, int childIndex) {
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
};


CLanguageModel::Context CMandarinAlphMgr::CreateSymbolContext(CAlphNode *pParent, symbol iSymbol)
{
	//Context carry-over. This code may worth looking at debug
	return m_pLanguageModel->CloneContext(pParent->iContext);
}

CMandarinAlphMgr::CMandNode::CMandNode(CDasherNode *pParent, unsigned int iLbnd, unsigned int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CMandarinAlphMgr *pMgr, symbol iSymbol)
: CSymbolNode(pParent, iLbnd, iHbnd, pDispInfo, pMgr, iSymbol) {
}

CMandarinAlphMgr::CMandNode *CMandarinAlphMgr::makeSymbol(CDasherNode *pParent, unsigned int iLbnd, unsigned int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, symbol iSymbol) {
  return new CMandNode(pParent, iLbnd, iHbnd, pDispInfo, this, iSymbol);
}

void CMandarinAlphMgr::CMandNode::SetFlag(int iFlag, bool bValue) {
  //``disable learn-as-you-write for Mandarin Dasher''
   if (iFlag==NF_COMMITTED)
     CDasherNode::SetFlag(iFlag, bValue); //bypass CAlphNode setter!
  else
      CAlphNode::SetFlag(iFlag, bValue);
}

CMandarinAlphMgr::CMandSym::CMandSym(CDasherNode *pParent, unsigned int iLbnd, unsigned int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CMandarinAlphMgr *pMgr, symbol iSymbol)
: CMandNode(pParent, iLbnd, iHbnd, pDispInfo, pMgr, iSymbol) {
}

const std::string &CMandarinAlphMgr::CMandSym::outputText() {
  //use chinese, not pinyin, alphabet...
  return mgr()->m_pCHAlphabet->GetText(iSymbol);
}
