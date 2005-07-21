#ifndef __dashercomponent_h__
#define __dashercomponent_h__

#include "Event.h"
#include "EventHandler.h"
#include "SettingsStore.h"

namespace Dasher {
  class CDasherComponent;
}

class Dasher::CDasherComponent {
 public:
  CDasherComponent(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore)
    :m_pEventHandler(pEventHandler), m_pSettingsStore(pSettingsStore) {

    m_pEventHandler->RegisterListener(this);

  };

  ~CDasherComponent() {
    m_pEventHandler->UnregisterListener(this);
  };

  void InsertEvent(Dasher::CEvent * pEvent) {
    m_pEventHandler->InsertEvent(pEvent);
  };

  virtual void HandleEvent(Dasher::CEvent * pEvent) = 0;

  bool GetBoolParameter(int iParameter) const {
    return m_pSettingsStore->GetBoolParameter(iParameter);
  }
  long GetLongParameter(int iParameter) const {
    return m_pSettingsStore->GetLongParameter(iParameter);
  }
  std::string GetStringParameter(int iParameter)const {
    return m_pSettingsStore->GetStringParameter(iParameter);
  }
  void SetBoolParameter(int iParameter, bool bValue) const {
    m_pSettingsStore->SetBoolParameter(iParameter, bValue);
  }
  void SetLongParameter(int iParameter, long lValue) const {
    m_pSettingsStore->SetLongParameter(iParameter, lValue);
  }
  void SetStringParameter(int iParameter, std::string & sValue) const {
    m_pSettingsStore->SetStringParameter(iParameter, sValue);
  }

 protected:
  Dasher::CEventHandler *m_pEventHandler;
  CSettingsStore *m_pSettingsStore;
};

#endif
