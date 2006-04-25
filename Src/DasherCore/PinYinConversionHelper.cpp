#include "PinYinConversionHelper.h"

bool CPinYinConversionHelper::Convert(const std::string &strSource, std::vector<std::vector<std::string> > &vResult) {

  std::vector<std::string> vPhrase;

  vPhrase.push_back("hello");
  vPhrase.push_back("foo");
  vPhrase.push_back("fish");

  vResult.push_back(vPhrase);
  
  return true;
}
