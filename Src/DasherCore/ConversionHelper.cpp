// ConversionHelper.cpp
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

#include "ConversionHelper.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"
#include "DasherNode.h"

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdlib.h>

//Note the new implementation in Mandarin Dasher may not be compatible with the previous implementation of Japanese Dasher
//Need to reconcile (a small project)

using namespace Dasher;

CConversionHelper::CConversionHelper(CNodeCreationManager *pNCManager, CAlphabet *pAlphabet, CLanguageModel *pLanguageModel) :
  CConversionManager(pNCManager, pAlphabet), m_pLanguageModel(pLanguageModel) {
	  colourStore[0][0]=66;//light blue
	  colourStore[0][1]=64;//very light green
	  colourStore[0][2]=62;//light yellow
	  colourStore[1][0]=78;//light purple
	  colourStore[1][1]=81;//brownish
	  colourStore[1][2]=60;//red

  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();

}

CConversionManager::CConvNode *CConversionHelper::GetRoot(CDasherNode *pParent, int iLower, int iUpper, int iOffset) {
  CConvNode *pConvNode = CConversionManager::GetRoot(pParent, iLower, iUpper, iOffset);
  //note that pConvNode is actually a CConvHNode, created by this CConversionHelper:
  // the overridden CConversionManager::GetRoot creates the node the node it returns
  // by calling makeNode(), which we override to create a CConvHNode, and then just
  // fills in some of the fields; however, short of duplicating the code of
  // CConversionManager::GetRoot here, we can't get the type to reflect that...
	
  pConvNode->pLanguageModel = m_pLanguageModel;

  // context of a conversion node (e.g. ^) is the context of the
  // letter (e.g. e) before it (as the ^ entails replacing the e with
  // a single accented character e-with-^)
  pConvNode->iContext = pParent->CloneAlphContext(m_pLanguageModel);
  return pConvNode;
}

// TODO: This function needs to be significantly tidied up
// TODO: get rid of pSizes

void CConversionHelper::AssignChildSizes(const std::vector<SCENode *> &nodes, CLanguageModel::Context context) {

  AssignSizes(nodes, context, m_pNCManager->GetLongParameter(LP_NORMALIZATION), m_pNCManager->GetLongParameter(LP_UNIFORM));

}

void CConversionHelper::CConvHNode::PopulateChildren() {
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
      int iLbnd(iCum);
      int iHbnd(iCum + pCurrentSCEChild->NodeSize);
		//m_pNCManager->GetLongParameter(LP_NORMALIZATION));//

      iCum = iHbnd;

      // TODO: Parameters here are placeholders - need to figure out
      // what's right


      CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
      pDisplayInfo->iColour = mgr()->AssignColour(parentClr, pCurrentSCEChild, iIdx);
      pDisplayInfo->bShove = true;
      pDisplayInfo->bVisible = true;

      //  std::cout << "#" << pCurrentSCEChild->pszConversion << "#" << std::endl;

      pDisplayInfo->strDisplayText = pCurrentSCEChild->pszConversion;

      CConvNode *pNewNode = mgr()->makeNode(this, iLbnd, iHbnd, pDisplayInfo);

      // TODO: Reimplement ----

      // FIXME - handle context properly
      //      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      // -----

      pNewNode->bisRoot = false;
      pNewNode->pSCENode = pCurrentSCEChild;
      pNewNode->pLanguageModel = pLanguageModel;
      pNewNode->m_iOffset = m_iOffset + 1;

      if(pLanguageModel) {
        CLanguageModel::Context iContext;
        iContext = pLanguageModel->CloneContext(this->iContext);
        
        if(pCurrentSCEChild ->Symbol !=-1)
          pNewNode->pLanguageModel->EnterSymbol(iContext, pCurrentSCEChild->Symbol); // TODO: Don't use symbols?
        
        
        pNewNode->iContext = iContext;
      }

      DASHER_ASSERT(GetChildren().back()==pNewNode);

      ++iIdx;
    }

  }
  else {//End of conversion -> default to alphabet

      //Phil//
      // TODO: Placeholder algorithm here
      // TODO: Add an 'end of conversion' node?
    //ACL 1/12/09 Note that this adds one to the m_iOffset of the created node
    // (whereas code that was here did not, but was otherwise identical to
    // superclass method...?)
    CConvNode::PopulateChildren();
  }
}
int CConversionHelper::CConvHNode::ExpectedNumChildren() {
  if(bisRoot && !pSCENode) mgr()->BuildTree(this);  
  if (pSCENode && !pSCENode->GetChildren().empty()) return pSCENode->GetChildren().size();
  return CConvNode::ExpectedNumChildren();
}

void CConversionHelper::BuildTree(CConvHNode *pRoot) {
  // Build the string to convert.
  std::string strCurrentString;
  // Search backwards but stop at any conversion node.
  for (CDasherNode *pNode = pRoot->Parent();
       pNode && pNode->mgrId() != 2;
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

CConversionHelper::CConvHNode::CConvHNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CConversionHelper *pMgr)
: CConvNode(pParent, iLbnd, iHbnd, pDispInfo, pMgr) {
}


CConversionHelper::CConvHNode *CConversionHelper::makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo) {
  return new CConvHNode(pParent, iLbnd, iHbnd, pDispInfo, this);
}

void CConversionHelper::CConvHNode::SetFlag(int iFlag, bool bValue) {
  CDasherNode::SetFlag(iFlag, bValue);
  switch(iFlag) {
  case NF_COMMITTED:
    if(bValue){

      if(!pSCENode)
	return;

      symbol s =pSCENode ->Symbol;


      if(s!=-1)
	pLanguageModel->LearnSymbol(mgr()->m_iLearnContext, s);
    }
    break;
  }
}
