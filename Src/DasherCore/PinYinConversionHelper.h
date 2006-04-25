#ifndef __PINYIN_CONVERSION_HELPER_H__
#define __PINYIN_CONVERSION_HELPER_H__

#include "ConversionHelper.h"

class CPinYinConversionHelper : public CConversionHelper {
 public:
  virtual bool Convert(const std::string &strSource, std::vector<std::vector<std::string> > &vResult);
};

#endif
