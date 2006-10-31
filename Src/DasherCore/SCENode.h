
#ifndef __SCENODE_H__
#define __SCENODE_H__

/*Common Node Definition for Chinese Pinyin (possibly also Japanese) 
  Conversion Library and Dasher ConversionManager*/

typedef struct _SCENode SCENode;



struct _SCENode {

  char *pszConversion;
  SCENode *pNext;
  SCENode *pChild;

  int IsHeadAndCandNum;
  int CandIndex;
  int IsComplete;
  int AcCharCount;  /*accumulative character count*/
  
};


#endif

