/*
 *  ExpansionPolicy.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 03/06/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#ifndef __ExpansionPolicy_h__
#define __ExpansionPolicy_h__

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include "DasherNode.h"

class CNodeCreationManager;

namespace Dasher {
  class CDasherModel;
class CExpansionPolicy
{
public:
  ///dMaxCost should be the value returned by pushNode from the call for the node most closely enclosing pNode (that was pushed!)
  ///for the first (outermost) node, i.e. when no enclosing node has been passed, (+ive) INFINITY should be passed in.
	virtual double pushNode(CDasherNode *pNode, int iDasherMinY, int iDasherMaxY, bool bExpand, double dMaxCost)=0;
  ///Return TRUE if another frame should be forced.
  virtual bool apply(CNodeCreationManager *pMgr, CDasherModel *model)=0;
};

class NoExpansions : public CExpansionPolicy
{
public:
	double pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dMaxCost) {return dMaxCost;}
  bool apply(CNodeCreationManager *pMgr, CDasherModel *model) {return false;}
};

///A policy that expands/collapses nodes to maintain a given node budget.
///Also ascribes uniform costs, according to size within the range 0-4096.
class BudgettingPolicy : public CExpansionPolicy
{
public:
  BudgettingPolicy(unsigned int iNodeBudget);
  ///sets cost according to getCost(pNode,iMin,iMax);
  ///then assures node is cheaper (less important) than its parent;
  ///then adds to relevant queue
  double pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dParentCost);
  bool apply(CNodeCreationManager *pMgr, CDasherModel *pModel);
protected:
  virtual double getCost(CDasherNode *pNode, int iDasherMinY, int iDasherMaxY);
  ///return the intersection of the ranges (y1-y2) and (iMin-iMax)
  int getRange(int y1, int y2, int iMin, int iMax);
  std::vector<std::pair<double,CDasherNode *> > sExpand, sCollapse;
  unsigned int m_iNodeBudget;
};

///limits expansion to a few nodes (per instance i.e. per frame)
///(collapsing is at present unlimited, have to test this...)
class AmortizedPolicy : public BudgettingPolicy
{
public:
  AmortizedPolicy(unsigned int iNodeBudget);
	AmortizedPolicy(unsigned int iNodeBudget, unsigned int iMaxExpands);
  bool apply(CNodeCreationManager *pMgr, CDasherModel *pModel);
  double pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dParentCost);
private:
	unsigned int m_iMaxExpands;
  void trim();
};
}
#endif /*defined __ExpansionPolicy_h__*/
