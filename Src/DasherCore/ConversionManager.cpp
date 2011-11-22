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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ConversionManager.h"
#include "Event.h"
#include "Observable.h"
#include "NodeCreationManager.h"
#include "DasherModel.h"
#include "DasherInterfaceBase.h"

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdlib.h>

//Note the new implementation in Mandarin Dasher may not be compatible with the previous implementation of Japanese Dasher
//Need to reconcile (a small project)

using namespace Dasher;
using namespace std;

CConversionManager::CConversionManager(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, CLanguageModel *pLanguageModel)
: CSettingsUser(pCreateFrom), m_pInterface(pInterface), m_pNCManager(pNCManager), m_pAlphabet(pAlphabet), m_pLanguageModel(pLanguageModel) {

  //Testing for alphabet details, delete if needed:
  /*
  int alphSize = pNCManager->GetAlphabet()->GetNumberSymbols();
  std::cout<<"Alphabet size: "<<alphSize<<std::endl;
  for(int i =0; i<alphSize; i++)
    std::cout<<"symbol: "<<i<<"    display text:"<<pNCManager->GetAlphabet()->GetDisplayText(i)<<std::endl;
  */
  colourStore[0][0]=66;//light blue
  colourStore[0][1]=64;//very light green
  colourStore[0][2]=62;//light yellow
  colourStore[1][0]=78;//light purple
  colourStore[1][1]=81;//brownish
  colourStore[1][2]=60;//red
  
  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();

}

CConversionManager::CConvNode *CConversionManager::makeNode(int iOffset, int iColour, CDasherScreen::Label *pLabel) {
  return new CConvNode(iOffset, iColour, pLabel, this);
}

void CConversionManager::ChangeScreen(CDasherScreen *pScreen) {
  if (m_pScreen==pScreen) return;
  for (map<string, CDasherScreen::Label *>::iterator it=m_vLabels.begin(); it!=m_vLabels.end(); it++)
    delete it->second;
  m_vLabels.clear();
  m_pScreen=pScreen;
}

CDasherScreen::Label *CConversionManager::GetLabel(const char *pszConversion) {
  string strConv(pszConversion);
  if (m_vLabels[strConv])
    return m_vLabels[strConv];
  return m_vLabels[strConv] = m_pScreen->MakeLabel(strConv);
}

CConversionManager::CConvNode *CConversionManager::GetRoot(int iOffset, CLanguageModel::Context newCtx) {

  // TODO: Parameters here are placeholders - need to figure out what's right

  //TODO: hard-coded colour, and hard-coded displaytext... (ACL: read from Alphabet -> startConversionSymbol ?)
  CConvNode *pNewNode = makeNode(iOffset, 9, NULL);

  pNewNode->iContext = newCtx;

  pNewNode->bisRoot = true;

  pNewNode->pSCENode = 0;

  return pNewNode;
}

CConversionManager::CConvNode::CConvNode(int iOffset, int iColour, CDasherScreen::Label *pLabel, CConversionManager *pMgr)
 : CDasherNode(iOffset, iColour, pLabel), m_pMgr(pMgr) {
}

CConversionManager::CConvNode::~CConvNode() {
  m_pMgr->m_pLanguageModel->ReleaseContext(iContext);
}

void CConversionManager::RecursiveDumpTree(ostream &out, SCENode *pCurrent, unsigned int iDepth) {
  const std::vector<SCENode *> &children = pCurrent->GetChildren();
  for (std::vector<SCENode *>::const_iterator it = children.begin(); it!=children.end(); it++) {
    SCENode *pCurrent(*it);
    for(unsigned int i(0); i < iDepth; ++i)
      out << "-";
    out << " " << pCurrent->pszConversion << std::endl;//" " << pCurrent->IsHeadAndCandNum << " " << pCurrent->CandIndex << " " << pCurrent->IsComplete << " " << pCurrent->AcCharCount << std::endl;
    RecursiveDumpTree(out, pCurrent, iDepth + 1);
  }
}

void CConversionManager::CConvNode::GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, std::vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN) && iOffset+iLength-1 == offset()) {
    //ACL I'm extrapolating from PinYinConversionHelper (in which root nodes have their
    // Symbol set by SetConvSymbol, and child nodes are created in PopulateChildren
    // from SCENode's with Symbols having been set in in AssignSizes); not really sure
    // whether this is applicable in the general case(! - but although I think it's right
    // for PinYin, it wouldn't actually be used there, as MandarinDasher overrides contexts
    // everywhere!)
    DASHER_ASSERT(bisRoot || pSCENode);
    if (bisRoot || pSCENode->Symbol!=-1) {
      if (iLength>1) Parent()->GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength-1);
      vContextSymbols.push_back(bisRoot ? iSymbol : pSCENode->Symbol);
      return;
    } //else, non-root with pSCENode->Symbol==-1 => fallthrough back to superclass code
  }
  CDasherNode::GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength);
}

void CConversionManager::CConvNode::Output() {
  // TODO: Reimplement this
  //  m_pNCManager->m_bContextSensitive = true;

  SCENode *pCurrentSCENode(pSCENode);

  if(pCurrentSCENode){
    m_pMgr->m_pInterface->editOutput(pCurrentSCENode->pszConversion, this);

    if((GetChildren())[0]->mgr() == m_pMgr) {
      if (static_cast<CConvNode *>(GetChildren()[0])->m_pMgr == m_pMgr) {
        m_pMgr->m_pInterface->editProtect(this); //TODO used to pass in offset 0, will now get this node's offset...
      }
    }
  }
  else {
    m_pMgr->m_pInterface->editOutput(bisRoot ? ">" : "|", this);

    m_pMgr->m_pInterface->editConvert(this); //TODO used to pass in offset 0, will now get this node's offset...
  }
}

void CConversionManager::CConvNode::Undo() {
  SCENode *pCurrentSCENode(pSCENode);

  if(pCurrentSCENode) {
    if(pCurrentSCENode->pszConversion && (strlen(pCurrentSCENode->pszConversion) > 0)) {
      m_pMgr->m_pInterface->editDelete(pCurrentSCENode->pszConversion, this);
    }
  }
  else {
    m_pMgr->m_pInterface->editDelete(bisRoot ? ">" : "|", this);
  }
}

// TODO: This function needs to be significantly tidied up
// TODO: get rid of pSizes

void CConversionManager::AssignChildSizes(const std::vector<SCENode *> &nodes, CLanguageModel::Context context) {
  
  AssignSizes(nodes, context, CDasherModel::NORMALIZATION, GetLongParameter(LP_UNIFORM));
  
}

void CConversionManager::CConvNode::PopulateChildren() {
  DASHER_ASSERT(mgr()->m_pNCManager);
  
  // Do the conversion and build the tree (lattice) if it hasn't been
  // done already.
  //
  
  
  if(bisRoot && !pSCENode) {
    mgr()->BuildTree(this);
  }
  
  
  if(pSCENode && !pSCENode->GetChildren().empty()) {
    const std::vector<SCENode *> &vChildren = pSCENode->GetChildren();
    //    RecursiveDumpTree(pSCENode, 1);
    mgr()->AssignChildSizes(vChildren, iContext);
    
    int iIdx(0);
    int iCum(0);
    
    //    int parentClr = pNode->Colour();
    // TODO: Fixme
    int parentClr = 0;
    
    // Finally loop through and create the children
    
    for (std::vector<SCENode *>::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
      //      std::cout << "Current scec: " << pCurrentSCEChild << std::endl;
      SCENode *pCurrentSCEChild(*it);
      DASHER_ASSERT(pCurrentSCEChild != NULL);
      unsigned int iLbnd(iCum);
      unsigned int iHbnd(iCum + pCurrentSCEChild->NodeSize);
      //CDasherModel::NORMALIZATION);//
      
      iCum = iHbnd;
      
      // TODO: Parameters here are placeholders - need to figure out
      // what's right
      
      //  std::cout << "#" << pCurrentSCEChild->pszConversion << "#" << std::endl;
      
      CConvNode *pNewNode = mgr()->makeNode(offset()+1, mgr()->AssignColour(parentClr, pCurrentSCEChild, iIdx), mgr()->GetLabel(pCurrentSCEChild->pszConversion));

      // TODO: Reimplement ----

      // FIXME - handle context properly
      //      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      // -----

      pNewNode->bisRoot = false;
      pNewNode->pSCENode = pCurrentSCEChild;

      pNewNode->iContext = mgr()->m_pLanguageModel->CloneContext(this->iContext);

      if(pCurrentSCEChild ->Symbol !=-1)
        mgr()->m_pLanguageModel->EnterSymbol(pNewNode->iContext, pCurrentSCEChild->Symbol); // TODO: Don't use symbols?

      pNewNode->Reparent(this, iLbnd, iHbnd);

      ++iIdx;
    }
    
  }
  else {//End of conversion -> default to alphabet
    
    //Phil//
    // TODO: Placeholder algorithm here
    // TODO: Add an 'end of conversion' node?
    //ACL 1/12/09 Note that this adds one to the m_iOffset of the created node
    // (whereas code that was once here did not, but was otherwise identical...)
    CDasherNode *pNewNode = m_pMgr->m_pNCManager->GetAlphabetManager()->GetRoot(this, false, offset() + 1);
    pNewNode->Reparent(this, 0, CDasherModel::NORMALIZATION);
  }
}

int CConversionManager::CConvNode::ExpectedNumChildren() {
  if(bisRoot && !pSCENode) mgr()->BuildTree(this);
  if (pSCENode && !pSCENode->GetChildren().empty()) return pSCENode->GetChildren().size();
  return 1;//Alphabet root
}

void CConversionManager::BuildTree(CConvNode *pRoot) {
  // Build the string to convert.
  std::string strCurrentString;
  // Search backwards but stop at any conversion node.
  for (CDasherNode *pNode = pRoot->Parent();
       pNode && pNode->mgr() == this;
       pNode = pNode->Parent()) {
    
    // TODO: Need to make this the edit text rather than the display text
    strCurrentString =
    m_pAlphabet->GetText(pNode->GetAlphSymbol())
    + strCurrentString;
  }
  // Handle/store the result.
  SCENode *pStartTemp;
  Convert(strCurrentString, &pStartTemp);
  
  // Store all conversion trees (SCENode trees) in the pUserData->pSCENode
  // of each Conversion Root.
  
  pRoot->pSCENode = pStartTemp;
}

void CConversionManager::CConvNode::SetFlag(int iFlag, bool bValue) {
  CDasherNode::SetFlag(iFlag, bValue);
  switch(iFlag) {
    case NF_COMMITTED:
      if(bValue){
        
        if(!pSCENode)
          return;
        
        symbol s =pSCENode ->Symbol;
        
        
        if(s!=-1)
          mgr()->m_pLanguageModel->LearnSymbol(mgr()->m_iLearnContext, s);
      }
      break;
  }
}
