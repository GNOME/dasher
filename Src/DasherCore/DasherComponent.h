#ifndef __dashercomponent_h__
#define __dashercomponent_h__

#include "SettingsStore.h"
namespace Dasher {
  class CEvent;
  class CEventHandler;
  class CDasherComponent;
};

/// \ingroup Core
/// @{
class Dasher::CDasherComponent {
 public:
  CDasherComponent(Dasher::CEventHandler* pEventHandler, CSettingsStore* pSettingsStore);
  virtual ~CDasherComponent();

  void InsertEvent(Dasher::CEvent * pEvent);
  virtual void HandleEvent(Dasher::CEvent * pEvent) {};

  bool GetBoolParameter(int iParameter) const;
  void SetBoolParameter(int iParameter, bool bValue) const;

  long GetLongParameter(int iParameter) const;
  void SetLongParameter(int iParameter, long lValue) const;

  std::string GetStringParameter(int iParameter) const;
  void        SetStringParameter(int iParameter, const std::string & sValue) const;

  ParameterType   GetParameterType(int iParameter) const;
  std::string     GetParameterName(int iParameter) const;

 protected:
  Dasher::CEventHandler *m_pEventHandler;
  CSettingsStore *m_pSettingsStore;
};
/// @}


#endif
