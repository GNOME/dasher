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
    virtual ~CExpansionPolicy() { };
  ///dMaxCost should be the value returned by pushNode from the call for the node most closely enclosing pNode (that was pushed!)
  ///for the first (outermost) node, i.e. when no enclosing node has been passed, (+ive) INFINITY should be passed in.
	virtual double pushNode(CDasherNode *pNode, int iDasherMinY, int iDasherMaxY, bool bExpand, double dMaxCost)=0;
  ///Return TRUE if another frame should be forced.
  virtual bool apply()=0;
  ///Expand node immediately (do not wait for a call to apply()) - subclasses may use
  /// to implement their apply() methods, but public so the view can call directly for nodes
  /// which must be expanded during rendering. (Delegates to CDasherModel.)
  void ExpandNode(CDasherNode *pNode);
protected:
  CExpansionPolicy(CDasherModel *pModel) : m_pModel(pModel) {}
private:
  CDasherModel *m_pModel;
};

class NoExpansions : public CExpansionPolicy
{
public:
  NoExpansions() = default;
	double pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dMaxCost) override {return dMaxCost;}
  bool apply() override {return false;}
};

///A policy that expands/collapses nodes to maintain a given node budget.
///Also ascribes uniform costs, according to size within the range 0-4096.
class BudgettingPolicy : public CExpansionPolicy
{
public:
  BudgettingPolicy(CDasherModel *pModel, unsigned int iNodeBudget);
  ///sets cost according to getCost(pNode,iMin,iMax);
  ///then assures node is cheaper (less important) than its parent;
  ///then adds to relevant queue
  double pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dParentCost) override;
  bool apply() override;
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
  AmortizedPolicy(CDasherModel *pModel, unsigned int iNodeBudget);
	AmortizedPolicy(CDasherModel *pModel, unsigned int iNodeBudget, unsigned int iMaxExpands);
  ~AmortizedPolicy() override = default;
  bool apply() override;
  double pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dParentCost) override;
private:
	unsigned int m_iMaxExpands;
  void trim();
};
}
#endif /*defined __ExpansionPolicy_h__*/
