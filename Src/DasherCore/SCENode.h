// SCENode.h
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

#ifndef __SCENODE_H__
#define __SCENODE_H__

/*Common Node Definition for Chinese Pinyin (possibly also Japanese) 
  Conversion Library and Dasher ConversionManager*/

#include <vector>

/// \ingroup Model
/// \{
class SCENode {
 public:
  SCENode();

  ~SCENode();

  void Ref() {
    ++m_iRefCount;
  };

  void Unref() {
    --m_iRefCount;
    
    if(m_iRefCount == 0) {
      delete this;
    }
  };
  
  const std::vector<SCENode *> &GetChildren() const {
    return m_vChildren;
  }
  void AddChild(SCENode *pChild) {
    m_vChildren.push_back(pChild);
    pChild->Ref();
  }

  char *pszConversion;

  //int IsHeadAndCandNum;
  //int CandIndex;
  int Symbol;
  //unsigned int SumPYProbStore;
  
  //int IsComplete;
  //int AcCharCount;  /*accumulative character count*/
  
  int NodeSize;
  
  //unsigned int HZFreq;
  //float HZProb;
 private:
  int m_iRefCount;

  std::vector<SCENode *> m_vChildren;
};
/// \}

#endif

