/*
 *  ExpansionPolicy.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 26/10/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "ExpansionPolicy.h"
#include "DasherModel.h"
#include <algorithm>

using namespace Dasher;
using namespace std;

void CExpansionPolicy::ExpandNode(CDasherNode *pNode) {
  m_pModel->ExpandNode(pNode);
}

bool Less(pair<double,CDasherNode *> x, pair<double, CDasherNode *> y) {return x.first < y.first;}
bool More(pair<double,CDasherNode *> x, pair<double, CDasherNode *> y) {return x.first > y.first;}
  
BudgettingPolicy::BudgettingPolicy(CDasherModel *pModel, unsigned int iNodeBudget) : CExpansionPolicy(pModel), m_iNodeBudget(iNodeBudget) {}

double BudgettingPolicy::pushNode(CDasherNode *pNode, int iMin, int iMax, bool bExpand, double dParentCost) {
  double dRes = getCost(pNode, iMin, iMax);
  if (dRes<dParentCost) {
    //obvious case: node is less important/costly than parent; will be collapsed first
    // (or expanded but only if parent is not collapsed) 
    vector<pair<double, CDasherNode*> > &target = (bExpand) ? sExpand : sCollapse;
    target.push_back(pair<double, CDasherNode *>(dRes,pNode));
  } else {
    //node has same or greater cost than parent. Take care of latter case...
    dRes = dParentCost;
    
    //Parent has children, so must have been enqueued to collapse;
    // thus, avoid enqueuing child node to collapse also: if costs are accurate
    // (i.e. in terms of the benefit/detriment of what's onscreen), then collapsing
    // parent will free up more nodes (by recursively collapsing child)
    if (bExpand) sExpand.push_back(pair<double,CDasherNode *>(dRes, pNode));
    
    //Of course, that also removes the possibility of collapsing the parent first,
    // then trying to collapse the child afterwards (=>freed pointer), if they get
    // sorted the wrong way round...
  }
  return dRes;
}

///Expand one level per frame; note this won't really take effect until the *next* frame!
bool BudgettingPolicy::apply() {
  //firstly, sort the nodes...
  sort(sExpand.begin(), sExpand.end(), Less);
  //sExpand now in < order: [0] < [1] < ... < [size()-1] - so last element will have highest (cost=)benefit
  sort(sCollapse.begin(), sCollapse.end(),More);
  //sCollapse now in > order: [0] > ... > [size()-1] - so last element will have lowest cost(=benefit)
  
  //did we expand anything? (if so, there may be more opportunities for expansion next frame)
  bool bReturnValue = false;

  //maintain record of the highest cost we've incurred by collapsing a node;
  // avoid expanding anything LESS beneficial than that, as (even if we've room)
  // it'll be better to instead wait until next frame (and possibly re-expand the
  // collapsed node! Sadly we can't rely on trading one-for-one as different nodes
  // may have different numbers of children...)
  double collapseCost = -std::numeric_limits<double>::infinity();
  
  //first, make sure we are within our budget (probably only in case the budget's changed)
  while (!sCollapse.empty()
         && currentNumNodeObjects() > m_iNodeBudget)
  {
    pair<double,CDasherNode *> node = sCollapse.back();
    DASHER_ASSERT(node.first >= collapseCost);
    collapseCost = node.first;
    node.second->Delete_children();    
    sCollapse.pop_back();
  }

  //ok, we're now within budget. However, we may still wish to "trade off" nodes
  // against each other, in case there are any unimportant (low-cost) nodes we could collapse
  // to make room to expand other more important (high-benefit) nodes.  
  while (!sExpand.empty() && sExpand.back().first > collapseCost)
  {
    if (currentNumNodeObjects()+sExpand.back().second->ExpectedNumChildren() < m_iNodeBudget)
    {
      ExpandNode(sExpand.back().second);
      sExpand.pop_back();
      bReturnValue = true;
      //...and loop.
    }
    else if (!sCollapse.empty()
             && sCollapse.back().first < sExpand.back().first)
    {
      //could be a beneficial trade - make room by performing collapse...
      pair<double,CDasherNode *> node = sCollapse.back();
      DASHER_ASSERT(node.first >= collapseCost);
      collapseCost = node.first;
      node.second->Delete_children();
      sCollapse.pop_back();
      //...and see how much room that makes
    }
    else break; //not enough room, nothing to collapse.
  }
  sExpand.clear();
  sCollapse.clear();
  return bReturnValue;
}

int BudgettingPolicy::getRange(int y1, int y2, int iMin, int iMax) {
  if (y1>iMax || y2 < iMin) return 0;
  return min(y2, iMax) - max(y1, iMin);
}

double BudgettingPolicy::getCost(CDasherNode *pNode, int iDasherMinY, int iDasherMaxY) {
  return getRange(iDasherMinY, iDasherMaxY, 0, 4096);
}

AmortizedPolicy::AmortizedPolicy(CDasherModel *pModel, unsigned int iNodeBudget) : BudgettingPolicy(pModel,iNodeBudget), m_iMaxExpands(std::max(1u,(500+iNodeBudget)/1000)) {}

AmortizedPolicy::AmortizedPolicy(CDasherModel *pModel, unsigned int iNodeBudget, unsigned int iMaxExpands) : BudgettingPolicy(pModel, iNodeBudget), m_iMaxExpands(iMaxExpands) {}

double AmortizedPolicy::pushNode(CDasherNode *node, int iMin, int iMax, bool bExpand, double dParentCost) {
  double dRes = BudgettingPolicy::pushNode(node,iMin,iMax,bExpand,dParentCost);
  if (bExpand && sExpand.size() > 2*m_iMaxExpands) trim();
  return dRes;
}

bool AmortizedPolicy::apply() {
  trim();
  return BudgettingPolicy::apply();
}

void AmortizedPolicy::trim() {
  if (sExpand.size() <= m_iMaxExpands) return;
  //ok - repeatedly find a pivot element, and place it dividing all elements into
  //those more than it (in lower indices) and those less than it (in higher indices),
  // until we have separated off the <m_iMaxExpands> elements with greatest benefit
#ifdef DEBUG_TRIM
  vector<pair<double,CDasherNode *> > backup = sExpand; //yep, copy the lot
#endif
  unsigned int start = 0, stop = sExpand.size()-1;
  while (true) {
    //at this point, we assume we know only that elements [start - stop] need examining....
    unsigned int low = start, high = stop;
    pair<double,CDasherNode *> &pivot = sExpand[low++];
    while (true) {
      while (low <= high && !Less(sExpand[low],pivot)) low++;
      //all elements (start-low) can stay on left of pivot.
      if (low <= high) {
        //element <low> needs to be on R of pivot.
        while (high >= low && !More(sExpand[high],pivot)) high--;
        if (high >= low) {
          //element <high> needs to be on L of pivot
          pair<double,CDasherNode *> temp = sExpand[low];
          sExpand[low++] = sExpand[high];
          sExpand[high--]=temp;
          continue;
        } //else, fall through to break -> place pivot
      }
      break;
    }
    //place pivot at index (low-1) - if that's not where it is already!
    low--;
    if (start!=low) {
      pair<double,CDasherNode *> temp = sExpand[start];
      sExpand[start]=sExpand[low];
      sExpand[low] = temp;
    }
    //if (low == m_iMaxExpands), elements [0 - low-1] are all <= pivot
    //if (low == m_iMaxExpands-1), elements [0 - low] are all <= pivot (as pivot<=pivot!)
    //finish in both cases.
    if (low < m_iMaxExpands-1)
      start=low+1;
    else if (low>m_iMaxExpands)
      stop=low-1;
    else break;
  }
  //truncate array
  sExpand.resize(m_iMaxExpands);
#ifdef DEBUG_TRIM
  //now compare with the brute-force method...
  sort(sExpand.begin(), sExpand.end(), Less);
  sort(backup.begin(), backup.end(), Less);
  backup.erase(backup.begin(), backup.end()-m_iMaxExpands);
  //now compare. note we _don't_ require the node pointers to be the same;
  // where the cut-off point falls within a group of nodes with the same cost,
  // the two vectors could have different nodes from that group.
  double dFirstCost = sExpand[sExpand.size()-1].first;
  for (vector<pair<double, CDasherNode *> >::iterator it1=sExpand.begin(), it2=backup.begin(); it1!=sExpand.end(); it1++, it2++)
    if (*it1 == *it2) continue;
    else if (it1->first == dFirstCost && it2->first==dFirstCost) continue;
    else cout << "trim not equal!\n";
#endif
}
