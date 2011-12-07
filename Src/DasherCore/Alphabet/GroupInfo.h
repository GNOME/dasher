#ifndef __GROUP_INFO_H__
#define __GROUP_INFO_H__

#include <string>
/// \ingroup Alphabet
/// \{
struct SGroupInfo {
  SGroupInfo *pChild;
  SGroupInfo *pNext;
  std::string strLabel;
  ///lowest index of symbol that is in group
  int iStart;
  //one more than the highest index of a symbol in the group.
  // (iStart+1==iEnd => single character)
  int iEnd;
  int iColour;
  bool bVisible;
  int iNumChildNodes;
  void RecursiveDelete() {
    for(SGroupInfo *t=this; t; ) {
      SGroupInfo *next = t->pNext;
      t->pChild->RecursiveDelete();
      delete t;
      t = next;
    }
  }
};
/// \}

#endif
