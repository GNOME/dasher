#include "ModuleManager.h"

void CModuleManager::RegisterFactory(CModuleFactory *pFactory) {
  pFactory->Start();

  while(pFactory->IsMore()) {
    ModuleID_t iID;
    int iType;
    std::string strName;

    pFactory->GetNext(&iID, &iType, &strName);

    m_mapModules[iID] = pFactory;
  }
}

CDasherModule *CModuleManager::GetModule(ModuleID_t iID) {
  // TODO: Error checking here
  
  return m_mapModules[iID]->GetModule(iID);
}

CDasherModule *CModuleManager::GetModuleByName(const std::string strName) {
  for(std::map<ModuleID_t, CModuleFactory *>::iterator it(m_mapModules.begin()); it != m_mapModules.end(); ++it) {
    if(strName == (it->second)->GetName(it->first))
      return (it->second)->GetModule(it->first);
  }

  return 0;
}

void CModuleManager::ListModules(int iType, std::vector<std::string> &vList) {
  for(std::map<ModuleID_t, CModuleFactory *>::iterator it(m_mapModules.begin()); it != m_mapModules.end(); ++it) {
    if((it->second)->GetType(it->first) == iType)
      vList.push_back((it->second)->GetName(it->first));
  }
}
