#include "ModuleManager.h"

void CModuleManager::RegisterFactory(CModuleFactory *pFactory) {
  pFactory->Start();

  while(pFactory->IsMore()) {
    long long int iID;
    int iType;
    std::string strName;

    pFactory->GetNext(&iID, &iType, &strName);

    m_mapModules[iID] = pFactory;
  }
}

CDasherModule *CModuleManager::GetModule(long long int iID) {
  // TODO: Error checking here
  
  return m_mapModules[iID]->GetModule(iID);
}
