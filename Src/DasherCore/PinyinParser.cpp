// PinyinParser.cpp
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

#include "PinyinParser.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

CPinyinParser::CPinyinParser(const std::string &strAlphabetPath) {
  m_strLastGroup = "";
  pCurrentList = NULL;

  m_pRoot = new CTrieNode('0');

  FILE *Input;
  if((Input = fopen(strAlphabetPath.c_str(), "r")) == (FILE *) 0) {
    std::cerr << "Could not open Pin Yin alphabet file - conversion will not work" << std::endl;
    return;
  }

  XML_Parser Parser = XML_ParserCreate(NULL);
  XML_SetUserData(Parser, this);
  XML_SetElementHandler(Parser, XML_StartElement, XML_EndElement);

  const unsigned int BufferSize = 1024;
  char Buffer[BufferSize];
  int Done;
  do {
    size_t len = fread(Buffer, 1, sizeof(Buffer), Input);
    Done = len < sizeof(Buffer);
    if(XML_Parse(Parser, Buffer, len, Done) == XML_STATUS_ERROR) {
      break;
    }
  } while(!Done);
}

CPinyinParser::~CPinyinParser() {
  if(m_pRoot)
    m_pRoot->RecursiveDelete();
}

void 
CPinyinParser::XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts) {

  CPinyinParser *pThis = static_cast<CPinyinParser *>(userData);

  if(!strcmp("group", name)) {
     while(*atts != 0) {
      if(!strcmp("name", *atts)) {
	pThis->pCurrentList = pThis->ParseGroupName(*(atts + 1));
      }
      atts += 2;
    }
  }
  else if(!strcmp("s", name)) {
    while(*atts != 0) {
      if(!strcmp("t", *atts)) {
	if(pThis->pCurrentList) {
	  pThis->pCurrentList->insert(*(atts + 1));
	}
      }
      atts += 2;
    }
  }
}

void 
CPinyinParser::XML_EndElement(void *userData, const XML_Char * name) {
}

std::set<std::string> *CPinyinParser::ParseGroupName(const std::string &strName) {
  int i1 = strName.find('(');
  int i2 = strName.find(')');

  if((i1 > 0) && (i2 > 0)) {

    std::string strSymbol;

    
    if(isdigit(strName[i2-1]))
      strSymbol = strName.substr(i1+1, i2-i1-2);
    else
      strSymbol = strName.substr(i1+1, i2-i1-1);

    std::string strShortName = strSymbol;

    strSymbol += '1';

    std::string strTone = strName.substr(i2-1,1);
    int iTone = atoi(strTone.c_str());

    CTrieNode *pCurrentNode = m_pRoot;

    for(std::string::iterator it = strSymbol.begin(); it != strSymbol.end(); ++it) {
      CTrieNode *pChild = pCurrentNode->LookupChild(*it);

      if(!pChild) {
	pChild = new CTrieNode(*it);
	pCurrentNode->AddChild(pChild);
      }

      pCurrentNode = pChild;
    }

    std::set<std::string> *pList;

    // TODO: It seems like we're getting double instances of some here

    // Handle same symbol with different tones
    if(strShortName == m_strLastGroup) 
      pList = pCurrentList; 
    else
      pList = new std::set<std::string>;

    m_strLastGroup = strShortName;

    pCurrentNode->SetList(pList);
    return pList;
  }
  else {
    return NULL;
  }
}

bool CPinyinParser::Convert(const std::string &strPhrase, SCENode **pRoot) {
  *pRoot = NULL;

  // TODO: Need to implement real latice here with merges

  typedef std::pair<CTrieNode *, CLatticeNode *> tLPair;

  std::vector<tLPair> *pCurrentList = new std::vector<tLPair>;
  pCurrentList->push_back(tLPair(m_pRoot, NULL));

  for(std::string::const_iterator it = strPhrase.begin(); it != strPhrase.end(); ++it) {
    std::vector<tLPair> *pNewList = new std::vector<tLPair>;

    for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {
      // First see if we can directly continue:
      CTrieNode *pCurrentChild = it2->first->LookupChild(*it);

      if(pCurrentChild) {
	int iNewDepth;

	if(it2->second)
	  iNewDepth = it2->second->GetDepth();
	else
	  iNewDepth = 0;

	CLatticeNode *pNewLNode = new CLatticeNode(pCurrentChild->GetSymbol(), it2->second, NULL, iNewDepth, 0);
	pNewList->push_back(tLPair(pCurrentChild, pNewLNode));
      }

       // Now see if we can start a new symbol here
      pCurrentChild = it2->first->LookupChild('1');

      if(pCurrentChild) {
	CTrieNode *pCurrentChild2 = m_pRoot->LookupChild(*it);
	if(pCurrentChild2) {
	  int iOldDepth;

	  if(it2->second)
	    iOldDepth = it2->second->GetDepth();
	  else
	    iOldDepth = 0;

	  CLatticeNode *pNewLNode = new CLatticeNode('|', it2->second, pCurrentChild->GetList(), iOldDepth, 0);
	  CLatticeNode *pNewLNode2 = new CLatticeNode(pCurrentChild->GetSymbol(), pNewLNode, NULL, iOldDepth + 1, 0);
	  pNewList->push_back(tLPair(pCurrentChild2, pNewLNode2));

	  // Unref the initial count on the intermediate node
	  pNewLNode->Unref();
	}
      }

      if(it2->second)
	it2->second->Unref();
    }

    delete pCurrentList;
    pCurrentList = pNewList;
  }

  // Need to also look for terminating symbols at the end of the phrase
  {
    std::vector<tLPair> *pNewList = new std::vector<tLPair>;

    for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {

      // First see if we can directly continue:
      CTrieNode *pCurrentChild = it2->first->LookupChild('1');

      if(pCurrentChild) { 
	  int iOldDepth;

	  if(it2->second)
	    iOldDepth = it2->second->GetDepth();
	  else
	    iOldDepth = 0;

	CLatticeNode *pNewLNode = new CLatticeNode('|', it2->second, pCurrentChild->GetList(), iOldDepth, 0);
	CLatticeNode *pNewLNode2 = new CLatticeNode(pCurrentChild->GetSymbol(), pNewLNode, NULL, iOldDepth + 1, 0);

	pNewList->push_back(tLPair(pCurrentChild, pNewLNode2));

	pNewLNode->Unref();
      }

      if(it2->second)
	it2->second->Unref();
    }

    delete pCurrentList;
    pCurrentList = pNewList;

  }

  CLatticeNode *pPreviousNode = NULL;

  int iMinDepth = 1000000;


  for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {
    if(it2->second && (it2->second->GetDepth() < iMinDepth))
      iMinDepth = it2->second->GetDepth();
  }

  // Now trace back through the remaining paths
  for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {

    pPreviousNode = NULL;

    CLatticeNode *pCurrentNode = it2->second;

    int iPromotion;

    if(pCurrentNode && (pCurrentNode->GetDepth() == iMinDepth))
      iPromotion = 0;
    else
      iPromotion = 1;

    while(pCurrentNode) {
      if(pCurrentNode->GetPriority() > iPromotion)
	pCurrentNode->SetPriority(iPromotion);

      // Check to see whether child has already been added (this could be made more efficient)

      if(pPreviousNode) {
	bool bFound = false;

	CLatticeNode *pCurrentCh = pCurrentNode->GetChild();
	
	while(pCurrentCh) {
	  if(pCurrentCh == pPreviousNode) {
	    bFound = true;
	  }

	  pCurrentCh = pCurrentCh->GetNext();
	}

	if(!bFound) {
	  pPreviousNode->SetNext(pCurrentNode->GetChild());
	  pCurrentNode->SetChild(pPreviousNode);
	}
      }

      pPreviousNode = pCurrentNode;
      pCurrentNode = pCurrentNode->GetParent();
    }
  }
  
  // TODO: Put reference counting back in here

  // pPreviousNode should now be the root (assuming it exists)

  if(pPreviousNode) {
    *pRoot = pPreviousNode->RecursiveAddList(NULL);
  }

  for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2)
    if(it2->second)
      it2->second->Unref();


  return (*pRoot != NULL);
}

SCENode *CPinyinParser::CLatticeNode::RecursiveAddList(SCENode *pOldTail) {
  // pOldTail  is the tail from which to aggregate

  CLatticeNode *pCurrentChild = m_pChild;

  SCENode *pTail;

  if(m_pList)
    pTail = NULL;
  else
    pTail = pOldTail;

  while(pCurrentChild) {
    pTail = pCurrentChild->RecursiveAddList(pTail);
    
    pCurrentChild = pCurrentChild->GetNext();
  }

  // pTail now points towards the aggregated list of children, so that becomes the child of the new nodes:

  if(m_pList) {
     SCENode *pNewTail = pOldTail;

     for(std::set<std::string>::iterator it = m_pList->begin(); it != m_pList->end(); ++it) {
       SCENode *pNewNode = new SCENode;
       
       pNewNode->SetChild(pTail);
       pNewNode->SetNext(pNewTail);
       pNewNode->SetPriority(m_iPriority);
       
       pNewNode->pszConversion = new char[it->size() + 1];
       strcpy(pNewNode->pszConversion, it->c_str());
       
       pNewTail = pNewNode;
     }

     pTail = pNewTail;
  }
  
  // With no list the just passign through should be okay here.
  
  return pTail;
}
