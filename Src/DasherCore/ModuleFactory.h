#ifndef __MODULE_FACTORY_H__
#define __MODULE_FACTORY_H__

#include "DasherModule.h"

#include <string>

// TODO: Implement generic implementation of iterators

class CModuleFactory {
 public:
  virtual void Start() = 0;
  virtual bool IsMore() = 0;
  virtual void GetNext(long long int *iID, int *iType, std::string *strName) = 0;

  virtual CDasherModule *GetModule(long long int iID) = 0;
};

#endif
