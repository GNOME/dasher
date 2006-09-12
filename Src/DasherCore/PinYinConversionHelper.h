#ifndef __PINYIN_CONVERSION_HELPER_H__
#define __PINYIN_CONVERSION_HELPER_H__

#include "ConversionHelper.h"



class CPinYinConversionHelper : public CConversionHelper {
 public:

  CPinYinConversionHelper();
  
  virtual bool Convert(const std::string &strSource, SCENode ** pRoot, int * HZCount, int CMid);

  virtual bool GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid);

  virtual void BuildDataBase();
  virtual void ClearData(int CMid);

  virtual std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * GetDP(int CMid);// get data pointer

  virtual int AssignColour(int parentClr, SCENode * pNode, int childIndex);


  //CONTEXT DATA
  std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > >vContextData;  
//THE DATA IS CONSTRUCTED OF UNITS OF SINGLE CELLS STORING CONTEXT SEQUENCE
  //AND A CORRESPONDING SCORE 

  //LEVEL 1 : HZ INDEX : NUMBER OF CHARACTERS CONVERTED
  //LEVEL 2 : CAND INDEX : NUMBER OF CANDIDATES WITH EACH POSITION
  //LEVEL 3 : SUB HZ INDEX :(COULD BE REDUNDANT) CORRESPONDES TO
  //          INDEX OF CHARACTERS WHICH WERE CONSISTED IN PHRASES
  //LEVEL 4 : SUB CAND INDEX : WHICH CAND WAS IN THE PHRASE 
  //LEVLE 5 : CELL: STORING PHASES IN HZ INDEX AND ASSIGNED SCORE
  //          IN THE WAY: 1.SCORE 2.Z 3.Y 4.X FOR PHRASE XYZ
  //          PREVIOUSLY PROCESSED
  
 private:

  int colourStore[2][3]; 
			 
};

#endif
