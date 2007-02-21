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
  void GetNext(long long int *iID, int *iType, std::string *strName);

  CDasherModule *GetModule(long long int iID);
  std::string GetName(long long int iID);
  int GetType(long long int iID);

 private:
  CDasherModule *m_pModule;

  int m_iCurrentPos;
};
/// @}

#endif
