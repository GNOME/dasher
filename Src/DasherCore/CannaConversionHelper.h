#ifndef __CANNA_CONVERSION_HELPER_H__
#define __CANNA_CONVERSION_HELPER_H__

#include "ConversionHelper.h"
/// \ingroup Model
/// @{
class CCannaConversionHelper : public CConversionHelper {
 public:
  CCannaConversionHelper();
  ~CCannaConversionHelper();

  virtual bool Convert(const std::string &strSource, SCENode ** pRoot, int * childCount, int CMid);


  virtual void AssignSizes(SCENode * pStart, Dasher::CLanguageModel::Context context, long normalization, int uniform, int iNChildren);


  virtual Dasher::CLanguageModel * GetLanguageModel() { return NULL; };



/*   virtual bool GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid) { */
/*     return false; */
/*   }; */

/*   virtual void BuildDataBase() { */
/*   }; */

   virtual void ClearData(int CMid) { 
   }; 
  
/*   virtual std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * GetDP(int CMid) { */
/*     return NULL; */
/*   };//get data pointer */

 private:
  void ProcessCandidate(std::string strCandidate, SCENode *pRoot, SCENode *pTail);

  int context_id;
  bool IsInit;   
};
/// @}
#endif
