#ifndef __MODULE_MANAGER_H__
#define __MODULE_MANAGER_H__

#include "DasherModule.h"
#include "ModuleFactory.h"

#include <map>
#include <string>
#include <vector>

class CModuleManager {
 public:
  void RegisterFactory(CModuleFactory *pFactory);
  CDasherModule *GetModule(long long int iID);
  CDasherModule *GetModuleByName(const std::string strName);

  void ListModules(int iType, std::vector<std::string> &vList);

 private:
  std::map<long long int, CModuleFactory *> m_mapModules;
};

#endif
