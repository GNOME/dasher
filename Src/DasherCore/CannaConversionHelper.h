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

  unsigned int iUTF8Decode(const std::string &strChar) {
    int iLength = strChar.size();

    unsigned int iCode = 0;

    switch(iLength) {
    case 0:
      // Invalid - return 0;
      iCode = 0;
      break;
    case 1:
      iCode =  static_cast<int>(strChar[0]) & 127;
      break;
    case 2:
      iCode = static_cast<int>(strChar[1]) & 63 + ((static_cast<int>(strChar[0]) & 31) << 6);
      break;
    case 3:
      iCode = static_cast<int>(strChar[2]) & 63 + ((static_cast<int>(strChar[1]) & 63) << 6) + 
	((static_cast<int>(strChar[0]) & 15) << 12);
      break;
    case 4:
      // Invalid (as far as we're concerned) - return 0;
      iCode = 0;
      break;
    }

    return iCode;
  }

  int context_id;
  bool IsInit;   
};
/// @}
#endif
