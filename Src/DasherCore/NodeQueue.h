/*
 *  NodeQueue.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 03/06/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#ifndef __NodeQueue_h__
#define __NodeQueue_h__

#include <vector>
#include <queue>
#include <limits>
#include "DasherNode.h"

namespace Dasher {
class NodeQueue
{
public:
	virtual void pushNodeToExpand(CDasherNode *pNode)=0;
	virtual bool hasNodesToExpand()=0;
	virtual CDasherNode *nodeToExpand()=0;
	virtual void popNodeToExpand()=0;
	virtual void pushNodeToCollapse(CDasherNode *pNode)=0;
	virtual bool hasNodesToCollapse()=0;
	virtual CDasherNode *nodeToCollapse()=0;
	virtual void popNodeToCollapse()=0;
};

class NoNodeQueue : public NodeQueue
{
public:
	void pushNodeToExpand(CDasherNode *pNode) {}
	bool hasNodesToExpand() {return false;}
	CDasherNode *nodeToExpand() {return NULL;}
	void popNodeToExpand() {}
	void pushNodeToCollapse(CDasherNode *pNode) {}
	bool hasNodesToCollapse() {return false;}
	CDasherNode *nodeToCollapse() {return NULL;}
	void popNodeToCollapse() {}
};

class NodeComparator
{
public:
	int operator() (CDasherNode * &x, CDasherNode * &y)
	{
		return x->m_dCost < y->m_dCost;
	}
};
	
class ReverseNodeComparator
{
public:
	int operator() (CDasherNode * &x, CDasherNode * &y)
	{
		return x->m_dCost > y->m_dCost;
	}
};

class UnlimitedNodeQueue : public NodeQueue
{
public:
	void pushNodeToExpand(CDasherNode *pNode) {sExpand.push(pNode);}
	bool hasNodesToExpand() {return sExpand.size()>0;}
	CDasherNode *nodeToExpand() {return sExpand.top();}
	void popNodeToExpand() {sExpand.pop();}
	void pushNodeToCollapse(CDasherNode *pNode) {sDelete.push(pNode);}
	bool hasNodesToCollapse() {return sDelete.size()>0;}
	CDasherNode *nodeToCollapse() {return sDelete.top();}
	void popNodeToCollapse() {sDelete.pop();}
protected:
	std::priority_queue<CDasherNode *,std::vector<CDasherNode * >,NodeComparator> sExpand;
	std::priority_queue<CDasherNode *, std::vector<CDasherNode *>,ReverseNodeComparator> sDelete;
};

//limits expansion to a few nodes (per instance i.e. per frame)
  //(collapsing is at present unlimited, have to test this...)
class LimitedNodeQueue : public UnlimitedNodeQueue
{
public:
	LimitedNodeQueue(int _budget) : budget(_budget) {}
	bool hasNodesToExpand() {return budget>0 && UnlimitedNodeQueue::hasNodesToExpand();}
	CDasherNode *nodeToExpand() {return budget>0 ? UnlimitedNodeQueue::nodeToExpand() : NULL;}
	void popNodeToExpand() {if (budget>0) {UnlimitedNodeQueue::popNodeToExpand(); budget--;}}
private:
	int budget;
};
}
#endif /*defined __NodeQueue_h__*/
