
#ifndef __SCENODE_H__
#define __SCENODE_H__

/*Common Node Definition for Chinese Pinyin (possibly also Japanese) 
  Conversion Library and Dasher ConversionManager*/

typedef struct _SCENode SCENode;

/// \ingroup Model
/// \{
struct _SCENode {

  char *pszConversion;
  SCENode *pNext;
  SCENode *pChild;

  int IsHeadAndCandNum;
  int CandIndex;
  int Symbol;

  int IsComplete;
  int AcCharCount;  /*accumulative character count*/

  int NodeSize;

  uint HZFreq;
  float HZProb;
};
/// \}

#endif

