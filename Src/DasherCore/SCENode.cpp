// SCENode.cpp
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

#include "SCENode.h"

SCENode::SCENode() {
  m_pNext = 0;
  m_pChild = 0;
  
  m_iRefCount = 1;
}

SCENode::~SCENode() {
  // TODO: Delete string?

  if(m_pNext)
    m_pNext->Unref();
  
  if(m_pChild)
    m_pChild->Unref();
}

void SCENode::SetNext(SCENode *pNext) {
  if(m_pNext)
    m_pNext->Unref();
  
  m_pNext = pNext;

  if(m_pNext)
    m_pNext->Ref();
};

void SCENode::SetChild(SCENode *pChild) {
  if(m_pChild)
    m_pChild->Unref();
  
  m_pChild = pChild;

  if(m_pChild)
    m_pChild->Ref();
};

