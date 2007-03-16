#ifndef __SCENODE_H__
#define __SCENODE_H__

/*Common Node Definition for Chinese Pinyin (possibly also Japanese) 
  Conversion Library and Dasher ConversionManager*/


/// \ingroup Model
/// \{
class SCENode {
 public:
  SCENode();

  ~SCENode();

  void Ref() {
    ++m_iRefCount;
  };

  void Unref() {
    --m_iRefCount;
    
    if(m_iRefCount == 0) {
      delete this;
    }
  };
  
  SCENode *GetNext() {
    return m_pNext;
  };

  void SetNext(SCENode *pNext);

  SCENode *GetChild() {
    return m_pChild;
  };

  void SetChild(SCENode *pChild);

  char *pszConversion;

  int IsHeadAndCandNum;
  int CandIndex;
  int Symbol;
  
  int IsComplete;
  int AcCharCount;  /*accumulative character count*/
  
  int NodeSize;
  
  unsigned int HZFreq;
  float HZProb;
 private:
  int m_iRefCount;

  SCENode *m_pNext;
  SCENode *m_pChild;
};
/// \}

#endif

