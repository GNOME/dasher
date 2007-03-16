#include "SCENode.h"

SCENode::SCENode() {
  m_pNext = 0;
  m_pChild = 0;
  
  m_iRefCount = 1;
}

SCENode::~SCENode() {
  if(m_pNext)
    m_pNext->Unref();
  
  if(m_pChild)
    m_pChild->Unref();
}

void SCENode::SetNext(SCENode *pNext) {
  if(m_pNext)
    m_pNext->Unref();
  
  m_pNext = pNext;
  m_pNext->Ref();
};

void SCENode::SetChild(SCENode *pChild) {
  if(m_pChild)
    m_pChild->Unref();
  
  m_pChild = pChild;
  m_pChild->Ref();
};

