#ifndef __dashercomponent_h__
#define __dashercomponent_h__

namespace Dasher {
  class CDasherComponent;
  class CEvent;
  class CEventHandler;
}

class Dasher::CEvent;
class Dasher::CEventHandler;
#include "SettingsStore.h"



class Dasher::CDasherComponent {
 public:
  CDasherComponent(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);

  virtual ~CDasherComponent();

  void InsertEvent(Dasher::CEvent * pEvent);

  virtual void HandleEvent(Dasher::CEvent * pEvent) = 0;

  bool GetBoolParameter(int iParameter) const;
  long GetLongParameter(int iParameter) const;
  std::string GetStringParameter(int iParameter) const;
  void SetBoolParameter(int iParameter, bool bValue) const;
  void SetLongParameter(int iParameter, long lValue) const;
  void SetStringParameter(int iParameter, std::string & sValue) const;

  ParameterType   GetParameterType(int iParameter) const;
  std::string     GetParameterName(int iParameter) const;

 protected:
  Dasher::CEventHandler *m_pEventHandler;
  CSettingsStore *m_pSettingsStore;
};

#endif
