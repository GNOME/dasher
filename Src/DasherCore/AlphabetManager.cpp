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
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "LanguageModelling/CTWLanguageModel.h"

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

CAlphabetManager::CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphabetMap)
  : m_pNCManager(pNCManager), m_pAlphabet(pAlphabet), m_pAlphabetMap(pAlphabetMap), m_pInterface(pInterface) {
}

void CAlphabetManager::CreateLanguageModel(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore) {
  // FIXME - return to using enum here
  switch (m_pInterface->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
    default:
      // If there is a bogus value for the language model ID, we'll default
      // to our trusty old PPM language model.      
    case 0:
      m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
      break;
    case 2:
      m_pLanguageModel = new CWordLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet, m_pAlphabetMap);
      break;
    case 3:
      m_pLanguageModel = new CMixtureLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet, m_pAlphabetMap);
      break;  
    case 4:
      m_pLanguageModel = new CCTWLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
      break;
  }
  
  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();
}

CTrainer *CAlphabetManager::GetTrainer() {
  return new CTrainer(m_pLanguageModel, m_pAlphabetMap);
}

const CAlphInfo *CAlphabetManager::GetAlphabet() const {
  return m_pAlphabet;
}

CAlphabetManager::~CAlphabetManager() {
  m_pLanguageModel->ReleaseContext(m_iLearnContext);
  //the alphabet belongs to the AlphIO, and may be reused later;
  delete m_pAlphabetMap; //the map was created for this mgr.
  delete m_pLanguageModel;
}

int CAlphabetManager::GetColour(symbol sym, int iOffset) const {
  int iColour = m_pAlphabet->GetColour(sym);
  
  // This is for backwards compatibility with old alphabet files -
  // ideally make this log a warning (unrelated TODO: automate
  // validation of alphabet files, plus maintenance of repository
  // etc.)
  if(iColour == -1) {
    if(sym == m_pAlphabet->GetSpaceSymbol()) {
      iColour = 9;
    }
    else {
      iColour = (sym % 3) + 10;
    }
  }
  
  // Loop on low colours for nodes (TODO: go back to colour namespaces?)
  if((iOffset&1) == 0 && iColour < 130)
    iColour += 130;
  
  return iColour;
}


CAlphabetManager::CAlphNode::CAlphNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const string &strDisplayText, CAlphabetManager *pMgr)
: CDasherNode(pParent, iOffset, iLbnd, iHbnd, iColour, strDisplayText), m_pProbInfo(NULL), m_pMgr(pMgr) {
}

CAlphabetManager::CSymbolNode::CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(pParent, iOffset, iLbnd, iHbnd, pMgr->GetColour(_iSymbol, iOffset), strGroup+pMgr->m_pAlphabet->GetDisplayText(_iSymbol), pMgr), iSymbol(_iSymbol) {
}

CAlphabetManager::CSymbolNode::CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const string &strDisplayText, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(pParent, iOffset, iLbnd, iHbnd, iColour, strDisplayText, pMgr), iSymbol(_iSymbol) {
}

CAlphabetManager::CGroupNode::CGroupNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, CAlphabetManager *pMgr, const SGroupInfo *pGroup)
: CAlphNode(pParent, iOffset, iLbnd, iHbnd,
            pGroup ? (pGroup->bVisible ? pGroup->iColour : iBkgCol)
            : (iOffset&1) ? 7 : 137, //special case for root nodes
            pGroup ? strEnc+pGroup->strLabel : strEnc, pMgr), m_pGroup(pGroup) {
}

CAlphabetManager::CAlphNode *CAlphabetManager::GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset) {

  int iNewOffset(max(-1,iOffset-1));

  pair<symbol, CLanguageModel::Context> p = GetContextSymbols(pParent, iNewOffset, m_pAlphabetMap);
  
  CAlphNode *pNewNode;
  if(p.first==0 || !bEnteredLast) {
    //couldn't extract last symbol (so probably using default context), or shouldn't
    pNewNode = new CGroupNode(pParent, iNewOffset, iLower, iUpper, "", 0, this, NULL); //default background colour
  } else {
    pNewNode = new CSymbolNode(pParent, iNewOffset, iLower, iUpper, "", this, p.first);
    //if the new node is not child of an existing node, then it
    // represents a symbol that's already happened - so we're either
    // going backwards (rebuildParent) or creating a new root after a language change
    DASHER_ASSERT (!pParent);
  }

  pNewNode->iContext = p.second;
  return pNewNode;
}

pair<symbol, CLanguageModel::Context> CAlphabetManager::GetContextSymbols(CDasherNode *pParent, int iRootOffset, const CAlphabetMap *pAlphMap) {
  vector<symbol> vContextSymbols; bool bHaveFinalSymbol = true;
  //no context is ever available at offset -1 (=choice between symbols with offset 0)
  if (iRootOffset!=-1) {
    // TODO: make the LM get the context, rather than force it to fix max context length as an int
    int iStart = max(0, iRootOffset - m_pLanguageModel->GetContextLength());
    if(pParent) {
      pParent->GetContext(m_pInterface, pAlphMap, vContextSymbols, iStart, iRootOffset+1 - iStart);
    } else {
      pAlphMap->GetSymbols(vContextSymbols, m_pInterface->GetContext(iStart, iRootOffset+1 - iStart));
    }
  
    for (std::vector<symbol>::iterator it = vContextSymbols.end(); it!=vContextSymbols.begin();) {
      if (*(--it) == 0) {
        //found an impossible symbol! erase from beginning up to it (inclusive)
        vContextSymbols.erase(vContextSymbols.begin(), ++it);
        break;
      }
    }
  }
  if (vContextSymbols.empty()) {
    bHaveFinalSymbol = false;
    pAlphMap->GetSymbols(vContextSymbols, m_pAlphabet->GetDefaultContext());
  }
  
  CLanguageModel::Context iContext = m_pLanguageModel->CreateEmptyContext();
  
  //enter the symbols we could make sense of, into the LM context...
  for (vector<symbol>::iterator it=vContextSymbols.begin(); it != vContextSymbols.end(); it++) {
    m_pLanguageModel->EnterSymbol(iContext, *it);
  }
  return pair<symbol,CLanguageModel::Context>(bHaveFinalSymbol ? vContextSymbols[vContextSymbols.size()-1] : 0, iContext);
}

bool CAlphabetManager::CSymbolNode::GameSearchNode(string strTargetUtf8Char) {
  if (m_pMgr->m_pAlphabet->GetText(iSymbol) == strTargetUtf8Char) {
    SetFlag(NF_GAME, true);
    return true;
  }
  return false;
}
bool CAlphabetManager::CGroupNode::GameSearchNode(string strTargetUtf8Char) {
  if (GameSearchChildren(strTargetUtf8Char)) {
    SetFlag(NF_GAME, true);
    return true;
  }
return false;
}

CLanguageModel::Context CAlphabetManager::CAlphNode::CloneAlphContext(CLanguageModel *pLanguageModel) {
  if (iContext) return pLanguageModel->CloneContext(iContext);
  return CDasherNode::CloneAlphContext(pLanguageModel);
}

void CAlphabetManager::CSymbolNode::GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN) && iOffset+iLength-1 == offset()) {
    if (iLength > 1) Parent()->GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength-numChars());
    vContextSymbols.push_back(iSymbol);
  } else {
    CDasherNode::GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength);
  }
}

symbol CAlphabetManager::CSymbolNode::GetAlphSymbol() {
  return iSymbol;
}

void CAlphabetManager::CSymbolNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, NULL, NULL);
}
int CAlphabetManager::CAlphNode::ExpectedNumChildren() {
  int i=m_pMgr->m_pAlphabet->iNumChildNodes;
  return (m_pMgr->m_pNCManager->GetBoolParameter(BP_CONTROL_MODE)) ? i+1 : i;
}

void CAlphabetManager::GetProbs(vector<unsigned int> *pProbInfo, CLanguageModel::Context context) {
  const unsigned int iSymbols = m_pAlphabet->GetNumberTextSymbols();
  unsigned long iNorm(m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  
  // TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%
  // TODO: New method (see commented code) has been removed as it wasn' working.
  
  const int iControlSpace = m_pNCManager->GetBoolParameter(BP_CONTROL_MODE) ? iNorm / 20 : 0;
  
  iNorm -= iControlSpace;
  
  const unsigned long uniform(m_pNCManager->GetLongParameter(LP_UNIFORM)); 
  //the case for control mode on, generalizes to handle control mode off also,
  // as then iNorm - control_space == iNorm...
  const unsigned int iUniformAdd = ((iNorm * uniform) / 1000) / iSymbols;
  const unsigned long iNonUniformNorm = iNorm - iSymbols * iUniformAdd;
  //  m_pLanguageModel->GetProbs(context, Probs, iNorm, ((iNorm * uniform) / 1000));
  
  //ACL used to test explicitly for MandarinDasher and if so called GetPYProbs instead
  // (by statically casting to PPMPYLanguageModel). However, have renamed PPMPYLanguageModel::GetPYProbs
  // to GetProbs as per ordinary language model, so no need to test....
  m_pLanguageModel->GetProbs(context, *pProbInfo, iNonUniformNorm, 0);
  
  DASHER_ASSERT(pProbInfo->size() == m_pAlphabet->GetNumberTextSymbols()+1);//initial 0
  
  for(unsigned int k(1); k < pProbInfo->size(); ++k)
    (*pProbInfo)[k] += iUniformAdd;
  
#ifdef DEBUG
  {
    int iTotal = 0;
    for(int k = 0; k < pProbInfo->size(); ++k)
      iTotal += (*pProbInfo)[k];
    DASHER_ASSERT(iTotal == m_pNCManager->GetLongParameter(LP_NORMALIZATION) - iControlSpace);
  }
#endif  
}

std::vector<unsigned int> *CAlphabetManager::CAlphNode::GetProbInfo() {
  if (!m_pProbInfo) {
    m_pProbInfo = new std::vector<unsigned int>();
    m_pMgr->GetProbs(m_pProbInfo, iContext);

    // work out cumulative probs in place
    for(unsigned int i = 1; i < m_pProbInfo->size(); i++) {
      (*m_pProbInfo)[i] += (*m_pProbInfo)[i - 1];
    }
  }
  return m_pProbInfo;
}

std::vector<unsigned int> *CAlphabetManager::CGroupNode::GetProbInfo() {
  if (m_pGroup && Parent() && Parent()->mgr() == mgr()) {
    DASHER_ASSERT(Parent()->offset() == offset());
    return (static_cast<CAlphNode *>(Parent()))->GetProbInfo();
  }
  //nope, no usable parent. compute here...
  return CAlphNode::GetProbInfo();
}

void CAlphabetManager::CGroupNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, m_pGroup, NULL);
}

int CAlphabetManager::CGroupNode::ExpectedNumChildren() {
  return (m_pGroup) ? m_pGroup->iNumChildNodes : CAlphNode::ExpectedNumChildren();
}

CAlphabetManager::CGroupNode *CAlphabetManager::CreateGroupNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo) {

  // When creating a group node...
  // ...the offset is the same as the parent...
  
  CGroupNode *pNewNode = new CGroupNode(pParent, pParent->offset(), iLbnd, iHbnd, strEnc, iBkgCol, this, pInfo);

  //...as is the context!
  pNewNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);

  return pNewNode;
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildGroup(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo) {
  if (pInfo == m_pGroup) {
    SetRange(iLbnd, iHbnd);
    SetParent(pParent);
    //offset doesn't increase for groups...
    DASHER_ASSERT (offset() == pParent->offset());
    return this;
  }
  CGroupNode *pRet=m_pMgr->CreateGroupNode(pParent, iLbnd, iHbnd, strEnc, iBkgCol, pInfo);
  if (pInfo->iStart <= m_pGroup->iStart && pInfo->iEnd >= m_pGroup->iEnd) {
    //created group node should contain this one
    m_pMgr->IterateChildGroups(pRet,pInfo,this);
  }
  return pRet;
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildGroup(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strEnc, int iBkgCol, const SGroupInfo *pInfo) {
  CGroupNode *pRet=m_pMgr->CreateGroupNode(pParent, iLbnd, iHbnd, strEnc, iBkgCol, pInfo);
  if (pInfo->iStart <= iSymbol && pInfo->iEnd > iSymbol) {
    m_pMgr->IterateChildGroups(pRet, pInfo, this);
  }
  return pRet;
}

CDasherNode *CAlphabetManager::CreateSymbolNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol) {

    // TODO: Exceptions / error handling in general
    
    // Uniquely, a paragraph symbol can be two characters
    // (and we can't call numChars() on the symbol before we've constructed it!)
    int iNewOffset = pParent->offset()+1;
    if (m_pAlphabet->GetText(iSymbol)=="\r\n") iNewOffset++;
    CSymbolNode *pAlphNode = new CSymbolNode(pParent, iNewOffset, iLbnd, iHbnd, "", this, iSymbol);
  
    //     std::stringstream ssLabel;

    //     ssLabel << m_pAlphabet->GetDisplayText(iSymbol) << ": " << pNewNode;

    //    pDisplayInfo->strDisplayText = ssLabel.str();

    pAlphNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);
    m_pLanguageModel->EnterSymbol(pAlphNode->iContext, iSymbol); // TODO: Don't use symbols?

  return pAlphNode;
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildSymbol(CAlphNode *pParent,  unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol) {
  if(iSymbol == this->iSymbol) {
    SetRange(iLbnd, iHbnd);
    SetParent(pParent);
    DASHER_ASSERT(offset() == pParent->offset() + numChars());
    return this;
  }
  return m_pMgr->CreateSymbolNode(pParent, iLbnd, iHbnd, strGroup, iBkgCol, iSymbol);
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol) {
  return m_pMgr->CreateSymbolNode(pParent, iLbnd, iHbnd, strGroup, iBkgCol, iSymbol);
}

void CAlphabetManager::IterateChildGroups(CAlphNode *pParent, const SGroupInfo *pParentGroup, CAlphNode *buildAround) {
  std::vector<unsigned int> *pCProb(pParent->GetProbInfo());
  DASHER_ASSERT((*pCProb)[0] == 0);
  const int iMin(pParentGroup ? pParentGroup->iStart : 1);
  const int iMax(pParentGroup ? pParentGroup->iEnd : m_pAlphabet->GetNumberTextSymbols()+1);
  unsigned int iRange(pParentGroup ? ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]) : m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  
  // TODO: Think through alphabet file formats etc. to make this class easier.
  // TODO: Throw a warning if parent node already has children
  
  // Create child nodes and add them
  
  int i(iMin); //lowest index of child which we haven't yet added
  const SGroupInfo *pCurrentNode(pParentGroup ? pParentGroup->pChild : m_pAlphabet->m_pBaseGroup);
  // The SGroupInfo structure has something like linked list behaviour
  // Each SGroupInfo contains a pNext, a pointer to a sibling group info
  while (i < iMax) {
    CDasherNode *pNewChild;
    bool bSymbol = !pCurrentNode //gone past last subgroup
                  || i < pCurrentNode->iStart; //not reached next subgroup
    const int iStart=i, iEnd = (bSymbol) ? i+1 : pCurrentNode->iEnd;
    //uint64 is platform-dependently #defined in DasherTypes.h as an (unsigned) 64-bit int ("__int64" or "long long int")
    unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) *
                          (uint64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
                         iRange;
    unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) *
                          (uint64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
                         iRange;
    //loop for eliding groups with single children (see below).
    // Variables store necessary properties of any elided groups:
    std::string groupPrefix=""; int iBackgroundColour=pParent->getColour();
    const SGroupInfo *pInner=pCurrentNode;
    while (true) {
      if (bSymbol) {
        pNewChild = (buildAround) ? buildAround->RebuildSymbol(pParent, iLbnd, iHbnd, groupPrefix, iBackgroundColour, i) : CreateSymbolNode(pParent, iLbnd, iHbnd, groupPrefix, iBackgroundColour, i);
        i++; //make one symbol at a time - move onto next symbol in next iteration of (outer) loop
        break; //exit inner (group elision) loop
      } else if (pInner->iNumChildNodes>1) { //in/reached nontrivial subgroup - do make node for entire group:
        pNewChild= (buildAround) ? buildAround->RebuildGroup(pParent, iLbnd, iHbnd, groupPrefix, iBackgroundColour, pInner) : CreateGroupNode(pParent, iLbnd, iHbnd, groupPrefix, iBackgroundColour, pInner);
        i = pInner->iEnd; //make one group at a time - so move past entire group...
        pCurrentNode = pCurrentNode->pNext; //next sibling of _original_ pCurrentNode (above)
                                     // (maybe not of pCurrentNode now, which might be a subgroup filling the original)
        break; //exit inner (group elision) loop
      }
      //were about to create a group node, which would have only one child
      // (eventually, if the group node were PopulateChildren'd).
      // Such a child would entirely fill it's parent (the group), and thus,
      // creation/destruction of the child would cause the node's colour to flash
      // between that for parent group and child.
      // Hence, instead we elide the group node and create the child _here_...
      
      //1. however we also have to take account of the appearance of the elided group. Hence:
      groupPrefix += pInner->strLabel;
      if (pInner->bVisible) iBackgroundColour=pInner->iColour;
      //2. now go into the group...
      pInner = pInner->pChild;
      bSymbol = (pInner==NULL); //which might contain a single subgroup, or a single symbol
      if (bSymbol) pCurrentNode = pCurrentNode->pNext; //if a symbol, we've still moved past the outer (elided) group
      DASHER_ASSERT(iEnd == (bSymbol ? i+1 : pInner->iEnd)); //probability calcs still ok
      //3. loop round inner loop...
    }
    //created a new node - symbol or (group which will have >1 child).
    DASHER_ASSERT(pParent->GetChildren().back()==pNewChild);
  }

  if (!pParentGroup) AddExtras(pParent,pCProb);
  pParent->SetFlag(NF_ALLCHILDREN, true);
}

void CAlphabetManager::AddExtras(CAlphNode *pParent, vector<unsigned int> *pCProb) {
  //control mode:
  DASHER_ASSERT((pParent->GetChildren().back()->Hbnd() == m_pNCManager->GetLongParameter(LP_NORMALIZATION)) ^ m_pNCManager->GetBoolParameter(BP_CONTROL_MODE));
  if (m_pNCManager->GetBoolParameter(BP_CONTROL_MODE)) {
#ifdef _WIN32_WCE
    DASHER_ASSERT(false);
#endif
    //ACL leave offset as is - like its groupnode parent, but unlike its alphnode siblings,
    //the control node does not enter a symbol....
    m_pNCManager->GetControlManager()->GetRoot(pParent, pParent->GetChildren().back()->Hbnd(), m_pNCManager->GetLongParameter(LP_NORMALIZATION), pParent->offset());
  }
}

CAlphabetManager::CAlphNode::~CAlphNode() {
  delete m_pProbInfo;
  m_pMgr->m_pLanguageModel->ReleaseContext(iContext);
}

const std::string &CAlphabetManager::CSymbolNode::outputText() {
  if (iSymbol == m_pMgr->m_pAlphabet->GetParagraphSymbol() && GetFlag(NF_SEEN)) {
    //Regardless of this particular platform's definition of a newline,
    // which is what we'd _output_, when reversing back over text
    // which may have been produced elsewhere, we represent occurrences
    // of _either_ \n or \r\n by a single paragraph symbol.
    //If the alphabet has a paragraph symbol, \r is not a symbol on its own
    // (and \n isn't a symbol other than paragraph). So look for a
    // \r before the \n.
    DASHER_ASSERT(m_pMgr->m_pInterface->GetContext(offset(),1)=="\n");
    static std::string rn("\r\n"),n("\n"); //must store strings somewhere to return by reference!
    return (m_pMgr->m_pInterface->GetContext(offset()-1,2)=="\r\n") ? rn : n;
  }
  return mgr()->m_pAlphabet->GetText(iSymbol);
}

int CAlphabetManager::CSymbolNode::numChars() {
  return (outputText()=="\r\n") ? 2 : 1;
}

void CAlphabetManager::CSymbolNode::Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  //std::cout << this << " " << Parent() << ": Output at offset " << m_iOffset << " *" << m_pMgr->m_pAlphabet->GetText(t) << "* " << std::endl;

  Dasher::CEditEvent oEvent(1, outputText(), offset());
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);

  // Track this symbol and its probability for logging purposes
  if (pAdded != NULL) {
    pAdded->push_back(Dasher::SymbolProb(iSymbol, outputText(), Range() / (double)iNormalization));
  }
}

void CAlphabetManager::CSymbolNode::Undo(int *pNumDeleted) {
  DASHER_ASSERT(GetFlag(NF_SEEN));
  Dasher::CEditEvent oEvent(2, outputText(), offset());
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);
  if (pNumDeleted) (*pNumDeleted)++;
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildParent() {
  // CAlphNode's always have a parent, they inserted a symbol; CGroupNode's
  // with an m_pGroup have a container i.e. the parent group, unless
  // m_pGroup==NULL => "root" node where Alphabet->m_pBaseGroup is the *first*child*...
  if (m_pGroup == NULL) return NULL;
  //offset of group node is same as parent...
  return CAlphNode::RebuildParent(offset());
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildParent() {
  //parent's offset usually one less than this, but can be two for the paragraph symbol.
  return CAlphNode::RebuildParent(offset()-numChars());
}

CDasherNode *CAlphabetManager::CAlphNode::RebuildParent(int iNewOffset) {
  //possible that we have a parent, as RebuildParent() rebuilds back to closest AlphNode.
  if (Parent()) return Parent();
  
  CAlphNode *pNewNode = m_pMgr->GetRoot(NULL, 0, 0, iNewOffset!=-1, iNewOffset+1);
  
  //now fill in the new node - recursively - until it reaches us
  m_pMgr->IterateChildGroups(pNewNode, NULL, this);

  //finally return our immediate parent (pNewNode may be an ancestor rather than immediate parent!)
  DASHER_ASSERT(Parent() != NULL);

  //although not required, we believe only NF_SEEN nodes are ever requested to rebuild their parents...
  DASHER_ASSERT(GetFlag(NF_SEEN));
  //so set NF_SEEN on all created ancestors (of which pNewNode is the last)
  CDasherNode *pNode = this;
  do {
    pNode = pNode->Parent();
    pNode->SetFlag(NF_SEEN, true);
  } while (pNode != pNewNode);
  
  return Parent();
}

// TODO: Shouldn't there be an option whether or not to learn as we write?
// For want of a better solution, game mode exemption explicit in this function
void CAlphabetManager::CSymbolNode::SetFlag(int iFlag, bool bValue) {
  CDasherNode::SetFlag(iFlag, bValue);
  switch(iFlag) {
  case NF_COMMITTED:
    if(bValue && !GetFlag(NF_GAME) && m_pMgr->m_pInterface->GetBoolParameter(BP_LM_ADAPTIVE))
      m_pMgr->m_pLanguageModel->LearnSymbol(m_pMgr->m_iLearnContext, iSymbol);
    break;
  }
}
