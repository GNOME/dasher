#ifndef __CONVERSION_HELPER_H__
#define __CONVERSION_HELPER_H__

#include <string>
#include <vector>
#include <SCENode.h>



//THESE DEFINITIONS ARE SHARED BETWEEN PYCONVERSIONHELPER AND CONVERSIONMANAGER
#define MAX_CARE_CAND 100
#define MAX_CARE_PHRASE 20
#define MAX_HZ_NUM 50
#define MAX_CM_NUM 10 



//trial change
namespace Dasher{
  class CDasherNode;  //trial forward declaration
}

class CConversionHelper {
 public:
 virtual bool Convert(const std::string &strSource, SCENode ** pRoot, int * childCount, int CMid) = 0;

 virtual bool GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid)=0;
 virtual void BuildDataBase()=0;
 virtual void ClearData(int CMid)=0;

 virtual std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * GetDP(int CMid)=0;//get data pointer


 virtual int AssignColour(int parentClr, SCENode * pNode, int childIndex)=0;

 std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > >vContextData;

};

#endif
