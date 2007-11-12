#ifndef __MODULE_MANAGER_H__
#define __MODULE_MANAGER_H__

#include "DasherModule.h"
#include "ModuleFactory.h"

#include <map>
#include <string>
#include <vector>

/// \ingroup Core
/// \{
class CModuleManager {
 public:
  ~CModuleManager();
  void RegisterFactory(CModuleFactory *pFactory);
  CDasherModule *GetModule(ModuleID_t iID);
  CDasherModule *GetModuleByName(const std::string strName);

  void ListModules(int iType, std::vector<std::string> &vList);

 private:
  std::map<ModuleID_t, CModuleFactory *> m_mapModules;
};
/// \}

#endif
