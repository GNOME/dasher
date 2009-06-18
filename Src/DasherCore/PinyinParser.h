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
#include "Alphabet/Alphabet.h"

#include <expat.h>
#include <iostream>
#include <set>
#include <string>

// TODO:
// 1. Need a destructor
// 2. This leaks memory badly
// 3. Generally make things sensible!
// 4. Load the conversion data from a configurable location, store in a sensible binary format

class CPinyinParser {
 public:
  /// Constructor
  CPinyinParser(const std::string &strAlphabetPath);
  ~CPinyinParser();

  bool Convert(const std::string &pystr, SCENode **pRoot);


 private:
  static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
  static void XML_EndElement(void *userData, const XML_Char * name);
  
  std::set<std::string> *ParseGroupName(const std::string &strName);

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
    
    void SetList(std::set<std::string> *pList) {
      m_pList = pList;
    }
    
    std::set<std::string> *GetList() {
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

    void RecursiveDelete() {
      if(m_pChild)
	m_pChild->RecursiveDelete();
      
      if(m_pNext)
	m_pNext->RecursiveDelete();

      delete this;
    }
    
    //For debugging/testing
    void PrintSymbol(){
      if(m_cSymbol)
	std::cout<<m_cSymbol<<std::endl;
    }

    CTrieNode * GetChild(){
      return m_pChild;
    }
    
    std::set<std::string> *m_pList;
 
 private:
    CTrieNode *m_pChild;
    CTrieNode *m_pNext;
       
    char m_cSymbol;
  };

  std::set<std::string> *pCurrentList;
  std::string m_strCurrentGroup;
  std::string m_strLastGroup;
  CTrieNode *m_pRoot;
};

#endif
