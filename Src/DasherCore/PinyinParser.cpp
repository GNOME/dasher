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
#include <iostream>

CPinyinParser::CPinyinParser() {
  g_strLastGroup = "";
  pCurrentList = NULL;

  g_pRoot = new CTrieNode('0');

  FILE *Input;
  if((Input = fopen("Data/alphabets/alphabet.chineseRuby.xml", "r")) == (FILE *) 0) {
    // could not open file
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
	if(pThis->pCurrentList)
	  pThis->pCurrentList->push_back(*(atts + 1));
      }
      atts += 2;
    }
  }
}

void 
CPinyinParser::XML_EndElement(void *userData, const XML_Char * name) {
}

std::vector<std::string> *CPinyinParser::ParseGroupName(const std::string &strName) {
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

    CTrieNode *pCurrentNode = g_pRoot;

    for(std::string::iterator it = strSymbol.begin(); it != strSymbol.end(); ++it) {
      CTrieNode *pChild = pCurrentNode->LookupChild(*it);

      if(!pChild) {
	pChild = new CTrieNode(*it);
	pCurrentNode->AddChild(pChild);
      }

      pCurrentNode = pChild;
    }

    std::vector<std::string> *pList;

    // TODO: It seems like we're getting double instances of some here

    // Handle same symbol with different tones
    if(strShortName == g_strLastGroup) 
      pList = pCurrentList; 
    else
      pList = new std::vector<std::string>;

    g_strLastGroup = strShortName;

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
  pCurrentList->push_back(tLPair(g_pRoot, NULL));

  for(std::string::const_iterator it = strPhrase.begin(); it != strPhrase.end(); ++it) {
    std::vector<tLPair> *pNewList = new std::vector<tLPair>;

    for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {

      // First see if we can directly continue:
      CTrieNode *pCurrentChild = it2->first->LookupChild(*it);

      if(pCurrentChild) {
	CLatticeNode *pNewLNode = new CLatticeNode(pCurrentChild->GetSymbol(), it2->second, NULL);
	pNewList->push_back(tLPair(pCurrentChild, pNewLNode));
      }

      // Now see if we can start a new symbol here
      pCurrentChild = it2->first->LookupChild('1');

      if(pCurrentChild) {
	CTrieNode *pCurrentChild2 = g_pRoot->LookupChild(*it);
	if(pCurrentChild2) {
	  CLatticeNode *pNewLNode = new CLatticeNode('|', it2->second, pCurrentChild->GetList());
	  CLatticeNode *pNewLNode2 = new CLatticeNode(pCurrentChild->GetSymbol(), pNewLNode, NULL);
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

  // Now trace back through the remaining paths
  for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {

    CLatticeNode *pCurrentNode = it2->second;

    // Only print conversions which terminate at the end of a sylable
    if(it2->first->LookupChild('1')) {
      SCENode *pTail = NULL;

      pTail = AddList(it2->first->LookupChild('1')->GetList(), pTail);

      while(pCurrentNode) {
	if(pCurrentNode->GetList())
	  pTail = AddList(pCurrentNode->GetList(), pTail);

	pCurrentNode = pCurrentNode->GetParent();
      }
      
      
      // TODO: Compact lattice
      SCENode *pCurrentRoot = pTail;

      while(pCurrentRoot->GetNext())
	pCurrentRoot = pCurrentRoot->GetNext();

      pCurrentRoot->SetNext(*pRoot);
      *pRoot = pTail;
    }

    if(it2->second)
      it2->second->Unref();

  }

  return (*pRoot != NULL);
}

SCENode *CPinyinParser::AddList(std::vector<std::string> *pList, SCENode *pTail) {
  SCENode *pNewTail = NULL;

  for(std::vector<std::string>::iterator it = pList->begin(); it != pList->end(); ++it) {
    SCENode *pNewNode = new SCENode;
    
    pNewNode->SetChild(pTail);
    pNewNode->SetNext(pNewTail);
    
    pNewNode->pszConversion = new char[it->size() + 1];
    strcpy(pNewNode->pszConversion, it->c_str());

    pNewTail = pNewNode;
  }

  // TODO: Fix reference counting here
  return pNewTail;
}
