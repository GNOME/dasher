#ifndef __DASHER_MODULE_H__
#define __DASHER_MODULE_H__

#include "DasherComponent.h"

class CDasherModule : public Dasher::CDasherComponent {
 public:
  CDasherModule(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, long long int iID, int iType);

  virtual long long int GetID();
  virtual int GetType();

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
  long long int m_iID;
  int m_iType;
  int m_iRefCount;
};

#endif
