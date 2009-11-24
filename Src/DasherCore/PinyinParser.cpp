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
#include "Alphabet/Alphabet.h"
#include "DasherTypes.h"

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

  //RepairPYTrainingFile();
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

    //Changed for with tones!!!
    if(isdigit(strName[i2-1]))
      strSymbol = strName.substr(i1+1, i2-i1-1);
    else
      strSymbol = strName.substr(i1+1, i2-i1-1);

    std::string strShortName = strSymbol;

    strSymbol += '9';

    std::string strTone = strName.substr(i2-1,1);

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


bool CPinyinParser::Convert(const std::string &pystr, SCENode **pRoot) {
  *pRoot = NULL;


  //    std::cout<<"full string is "<<pystr<<std::endl;
  
  CTrieNode * pCurrentNode = m_pRoot;
  CTrieNode * pCurrentChild;

  for(std::string::const_iterator it = pystr.begin();it!=pystr.end();++it){
    pCurrentChild = pCurrentNode->LookupChild(*it);
    
    if(!pCurrentChild)
      return 0;
    else
      pCurrentNode = pCurrentChild;
  }
  pCurrentNode = pCurrentNode->LookupChild('9');
  if(!pCurrentNode)
    return 0;
  else{

    if(!pCurrentNode->m_pList)
      return 0;

    *pRoot = new SCENode;
    (*pRoot)->pszConversion = "";

    for(std::set<std::string>::iterator it = pCurrentNode->m_pList->begin(); it != pCurrentNode->m_pList->end(); ++it) {
      SCENode *pNewNode = new SCENode;
        
      pNewNode->pszConversion = new char[it->size() + 1];
      strcpy(pNewNode->pszConversion, it->c_str());

      //            std::string strChar(pNewNode->pszConversion);
      // std::cout<<"Mandarin Char: "<<strChar<<std::endl;
      // std::vector<int> CHSym;
      // m_pCHAlphabet->GetSymbols(&CHSym, &strChar, 0);
      // pNewNode->Symbol = CHSym[0];
      
      (*pRoot)->AddChild(pNewNode);
    }

    //Test code: will make program crash
    //    SCENode * pTemp = *pRoot;
    // if(pTemp->GetChild()->GetChild())
    //  std::cout<<"We have trouble in PYParser."<<std::endl;
    return 1;    
    
  }
}
