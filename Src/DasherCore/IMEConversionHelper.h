#ifndef __IME_CONVERSION_HELPER_H__
#define __IME_CONVERSION_HELPER_H__

#include <windows.h>
#include <imm.h>

#include "ConversionHelper.h"

class CIMEConversionHelper : public CConversionHelper {
 public:
  CIMEConversionHelper();
  ~CIMEConversionHelper();

  virtual bool Convert(const std::string &strSource, std::vector<std::vector<std::string> > &vResult);

private:
  bool IsInit;
  HIMC hIMC;
};

#endif
