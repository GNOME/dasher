#ifndef __MODULE_FACTORY_H__
#define __MODULE_FACTORY_H__

#include "DasherModule.h"

#include <string>

/// \ingroup Core
/// @{

/// \todo Implement generic implementation of iterators
class CModuleFactory {
 public:
  virtual void Start() = 0;
  virtual bool IsMore() = 0;
  virtual void GetNext(ModuleID_t *iID, int *iType, std::string *strName) = 0;

  virtual CDasherModule *GetModule(ModuleID_t iID) = 0;
  virtual std::string GetName(ModuleID_t iID) = 0;
  virtual int GetType(ModuleID_t iID) = 0;
};
/// @}

#endif
