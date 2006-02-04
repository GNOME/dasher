#ifndef __MODULE_MANAGER_H__
#define __MODULE_MANAGER_H__

#include "DasherModule.h"
#include "ModuleFactory.h"

#include <map>

class CModuleManager {
 public:
  void RegisterFactory(CModuleFactory *pFactory);
  CDasherModule *GetModule(long long int iID);

 private:
  std::map<long long int, CModuleFactory *> m_mapModules;
};

#endif
