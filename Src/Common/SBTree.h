/*
 *  SBTree.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 17/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include <stdlib.h>

class SBTree {
public:
    SBTree(int iValue);
    ~SBTree();
	
    void Add(int iValue);
    SBTree* Delete(int iValue);
	
    int GetCount() {
		return m_iCount;
    };
	
    int GetOffset(int iOffset);
		
private:
	void SetRightMost(SBTree* pNewTree);
    int m_iValue;
    SBTree *m_pLeft;
    SBTree *m_pRight;
    int m_iCount;
};
