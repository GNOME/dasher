#ifndef __GROUP_INFO_H__
#define __GROUP_INFO_H__

#include <string>
/// \ingroup Alphabet
/// \{
struct SGroupInfo {
  SGroupInfo *pChild;
  SGroupInfo *pNext;
  std::string strLabel;
  int iStart;
  int iEnd;
  int iColour;
  bool bVisible;
  int iNumChildNodes;
};
/// \}

#endif
