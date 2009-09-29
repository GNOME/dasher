/*
 *  SBTree.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 17/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "SBTree.h"

SBTree::SBTree(int iValue) {
	m_iValue = iValue;
	m_pLeft = NULL;
	m_pRight = NULL;
	m_iCount = 1;
}

SBTree::~SBTree() {
	if(m_pLeft)
		delete m_pLeft;
	
	if(m_pRight)
		delete m_pRight;
}

void SBTree::Add(int iValue) {
	++m_iCount;
	
	if(iValue > m_iValue) {
		if(m_pRight)
			m_pRight->Add(iValue);
		else
			m_pRight = new SBTree(iValue);
	}
	else {
		if(m_pLeft)
			m_pLeft->Add(iValue);
		else
			m_pLeft = new SBTree(iValue);
	}
}

SBTree* SBTree::Delete(int iValue) {
	// Hmm... deleting is awkward in binary trees
	
	if(iValue == m_iValue) {
		if(!m_pLeft) {
			SBTree *pOldRight = m_pRight;
			m_pRight = NULL;
			delete this;
			return pOldRight;
		}
		else {
			SBTree *pOldLeft = m_pLeft;
			pOldLeft->SetRightMost(m_pRight);
			m_pLeft = NULL;
			m_pRight = NULL;
			delete this;
			return pOldLeft;
		}
	}
	else if(iValue > m_iValue) {
		--m_iCount;
		m_pRight = m_pRight->Delete(iValue);
	}
	else {
		--m_iCount;
		m_pLeft = m_pLeft->Delete(iValue);
	}
	
	return this;
}

void SBTree::SetRightMost(SBTree* pNewTree) {
	if(pNewTree)
		m_iCount += pNewTree->GetCount();
	
	if(m_pRight)
		m_pRight->SetRightMost(pNewTree);
	else
		m_pRight = pNewTree;
}

int SBTree::GetOffset(int iOffset) {
	if(m_pLeft && (m_pLeft->GetCount() > iOffset))
		return m_pLeft->GetOffset(iOffset);
	else if((m_pLeft && (m_pLeft->GetCount() == iOffset)) || (!m_pLeft && (iOffset == 0)))
		return m_iValue;
	else if(m_pLeft)
		return m_pRight->GetOffset(iOffset - m_pLeft->GetCount() - 1);
	else
		return m_pRight->GetOffset(iOffset - 1);
}