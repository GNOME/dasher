#ifndef __CONVERSION_HELPER_H__
#define __CONVERSION_HELPER_H__

#include <string>
#include <vector>
#include "SCENode.h"
#include "LanguageModelling/LanguageModel.h"
#include "LanguageModelling/PPMLanguageModel.h"



//THESE DEFINITIONS ARE SHARED BETWEEN PYCONVERSIONHELPER AND CONVERSIONMANAGER
#define MAX_CARE_CAND 100
#define MAX_CARE_PHRASE 20
#define MAX_HZ_NUM 50
#define MAX_CM_NUM 10 



//trial change
namespace Dasher{
  class CDasherNode;  //trial forward declaration
}

/// \ingroup Model
/// @{
class CConversionHelper {
 public:
  CConversionHelper() {
    // TODO: Move all this further up the class hierarchy
    colourStore[0][0]=66;//light blue
    colourStore[0][1]=64;//very light green
    colourStore[0][2]=62;//light yellow
    colourStore[1][0]=78;//light purple
    colourStore[1][1]=81;//brownish
    colourStore[1][2]=60;//red
  };

  virtual bool Convert(const std::string &strSource, SCENode ** pRoot, int * childCount, int CMid) = 0;

  virtual void AssignSizes(SCENode * pStart, Dasher::CLanguageModel::Context context, long normalization, int uniform, int iNChildren)=0;

  //TODO: figure out why this function cannot return a CLanguageModel
  virtual Dasher::CLanguageModel * GetLanguageModel()=0;
 
/*   virtual bool GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid)=0; */
/*   virtual void BuildDataBase()=0; */
  virtual void ClearData(int CMid)=0; 
  
/*   virtual std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * GetDP(int CMid)=0;//get data pointer */
  
  virtual int AssignColour(int parentClr, SCENode * pNode, int childIndex) {
    int which = -1;
    
    for (int i=0; i<2; i++)
      for(int j=0; j<3; j++)
	if (parentClr == colourStore[i][j])
	  which = i;
    
    if(which == -1)
      return colourStore[0][childIndex%3];
    else if(which == 0)
      return colourStore[1][childIndex%3];
    else 
      return colourStore[0][childIndex%3]; 
  };
  
  std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > >vContextData;

 private:
  int colourStore[2][3]; 
};
/// @}
#endif
