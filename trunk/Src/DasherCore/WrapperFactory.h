#ifndef __WRAPPER_FACTORY_H__
#define __WRAPPER_FACTORY_H__

#include "../DasherCore/ModuleFactory.h"

/// \ingroup Core
/// @{
class CWrapperFactory : public CModuleFactory {
 public:
  CWrapperFactory(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherModule *pModule);
  ~CWrapperFactory();

  void Start();
  bool IsMore();
  void GetNext(ModuleID_t *iID, int *iType, std::string *strName);

  CDasherModule *GetModule(ModuleID_t iID);
  std::string GetName(ModuleID_t iID);
  int GetType(ModuleID_t iID);

 private:
  CDasherModule *m_pModule;

  int m_iCurrentPos;
};
/// @}

#endif
