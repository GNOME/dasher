// PinyinParser.h
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

#ifndef __PinyinParser_h__
#define __PinyinParser_h__

#include "SCENode.h"

#include <expat.h>
#include <iostream>
#include <string>
#include <vector>

// TODO:
// 1. Need a destructor
// 2. This leaks memory badly
// 3. Generally make things sensible!
// 4. Load the conversion data from a configurable location, store in a sensible binary format

class CPinyinParser {
 public:
  /// Constructor
  CPinyinParser();

  bool Convert(const std::string &strPhrase, SCENode **pRoot);

 private:
  static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
  static void XML_EndElement(void *userData, const XML_Char * name);
  
  std::vector<std::string> *ParseGroupName(const std::string &strName);
  SCENode *AddList(std::vector<std::string> *pList, SCENode *pTail);

  class CTrieNode {
  public:
    CTrieNode(char cSymbol) {
      m_pChild = NULL;
      m_pNext = NULL;
      m_pList = NULL;
      
      m_cSymbol = cSymbol;
    };
    
    void AddChild(CTrieNode *pNewChild) {
      pNewChild->SetNext(m_pChild);
      m_pChild = pNewChild;
    };
    
    void SetNext(CTrieNode *pNewNode) {
      m_pNext = pNewNode;
    };
    
    CTrieNode *GetNext() {
      return m_pNext;
    };
    
    char GetSymbol() {
      return m_cSymbol;
    };
    
    void SetList(std::vector<std::string> *pList) {
      m_pList = pList;
    }
    
    std::vector<std::string> *GetList() {
      return m_pList;
    }
    
    CTrieNode *LookupChild(char cSymbol) {
      CTrieNode *pCurrentChild = m_pChild;
      
      while(pCurrentChild) {
	if(pCurrentChild->GetSymbol() == cSymbol)
	  return pCurrentChild;
	pCurrentChild = pCurrentChild->GetNext();
      }
      
      return NULL;
    };
    
    void RecursivelyDump(int iDepth) {
      for(int i(0); i < iDepth; ++i)
	std::cout << " ";
      
      std::cout << m_cSymbol << std::endl;
      
      CTrieNode *pCurrentChild = m_pChild;
      
      while(pCurrentChild) {
	pCurrentChild->RecursivelyDump(iDepth + 1);
	pCurrentChild = pCurrentChild->GetNext();
      }
    }
    
  private:
    CTrieNode *m_pChild;
    CTrieNode *m_pNext;
    
    std::vector<std::string> *m_pList;
    
    char m_cSymbol;
  };
  
  class CLatticeNode {
  public:
    char m_cSymbol;
    // It actually makes more sense here to work backwards, so store pointers to parent
    CLatticeNode *m_pParent;
    std::vector<std::string> *m_pList;
  };

  std::vector<std::string> *pCurrentList;
  std::string g_strCurrentGroup;
  std::string g_strLastGroup;
  CTrieNode *g_pRoot;
};

#endif
