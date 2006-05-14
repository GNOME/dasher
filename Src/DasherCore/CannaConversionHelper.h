#ifndef __CANNA_CONVERSION_HELPER_H__
#define __CANNA_CONVERSION_HELPER_H__

#include "ConversionHelper.h"

class CCannaConversionHelper : public CConversionHelper {
 public:
  CCannaConversionHelper();
  ~CCannaConversionHelper();

  virtual bool Convert(const std::string &strSource, std::vector<std::vector<std::string> > &vResult);

 private:
  int context_id;
  bool IsInit;   
};

#endif
