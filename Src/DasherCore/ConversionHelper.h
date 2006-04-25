#ifndef __CONVERSION_HELPER_H__
#define __CONVERSION_HELPER_H__

#include <string>
#include <vector>

class CConversionHelper {
 public:
  virtual bool Convert(const std::string &strSource, std::vector<std::vector<std::string> > &vResult) = 0;
};

#endif
