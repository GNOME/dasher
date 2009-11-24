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

void CConversionHelper::AssignChildSizes(SCENode **pNode, CLanguageModel::Context context, int iNChildren) {

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


  iNChildren = 0;
  SCENode *pChild(*pNode);

  while(pChild) {
    pChild = pChild->GetNext();
    ++iNChildren;
  }

  //  std::cout<<"iNChildren: "<<iNChildren<<std::endl;
  AssignSizes(pNode, context, m_pNCManager->GetLongParameter(LP_NORMALIZATION), m_pNCManager->GetLongParameter(LP_UNIFORM), iNChildren);


}

void CConversionHelper::CConvHNode::PopulateChildren() {
  DASHER_ASSERT(mgr()->m_pNCManager);

  // Do the conversion and build the tree (lattice) if it hasn't been
  // done already.
  //


  if(bisRoot) {
    mgr()->BuildTree(this);
  }

  SCENode *pCurrentSCEChild;

  if(pSCENode){

    //    RecursiveDumpTree(pSCENode, 1);
    pCurrentSCEChild = pSCENode->GetChild();

  }
  else {
    //    if(m_pRoot && !pCurrentDataNode->bType)
    //  pCurrentSCEChild = m_pRoot[0];
    //else
      pCurrentSCEChild = 0;
  }

  if(pCurrentSCEChild) {
    //    std::cout<<"Populating character nodes!"<<std::endl;
    //    std::cout << "Current SCE Child: " << pCurrentSCEChild << std::endl;

    // TODO: Reimplement (in subclass) -----

//     if(m_iHZCount>1)
//       if(!m_bPhrasesProcessed[pCurrentSCEChild->AcCharCount-1])
//    	if(pCurrentSCEChild->AcCharCount<m_iHZCount)
// 	  ProcessPhrase(pCurrentSCEChild->AcCharCount-1);

    // -----

    //int *iSize;

    //    iSize = new int[pCurrentSCEChild->IsHeadAndCandNum];




    mgr()->AssignChildSizes(&pCurrentSCEChild, iContext, pCurrentSCEChild->IsHeadAndCandNum);

    int iIdx(0);
    int iCum(0);

    //    int parentClr = pNode->Colour();
    // TODO: Fixme
    int parentClr = 0;

    // Finally loop through and create the children

    do {
      //      std::cout << "Current scec: " << pCurrentSCEChild << std::endl;

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

      pCurrentSCEChild = pCurrentSCEChild->GetNext();
      ++iIdx;
    }while(pCurrentSCEChild);

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
