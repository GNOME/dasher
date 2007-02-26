#ifndef __DASHER_MODULE_H__
#define __DASHER_MODULE_H__

#include "../Common/ModuleSettings.h"
#include "DasherComponent.h"

#define VC7

#ifdef VC7
typedef __int64 ModuleID_t;
#else
typedef long long int ModuleID_t;
#endif

/// \ingroup Core
/// @{
class CDasherModule : public Dasher::CDasherComponent {
 public:
  CDasherModule(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, ModuleID_t iID, int iType, const char *szName);

  virtual ModuleID_t GetID();
  virtual int GetType();
  virtual const char *GetName();

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount) {
    return false;
  };

  // Basic Reference counting

  ///
  /// Add a reference to the module
  ///

  virtual void Ref();

  /// 
  /// Remove a reference to the module
  ///

  virtual void Unref();

 private:
  ModuleID_t m_iID;
  int m_iType;
  int m_iRefCount;
  const char *m_szName;
};
/// @}

#endif
