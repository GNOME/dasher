
#include "../Common/Common.h"

#include "DasherComponent.h"
#include "EventHandler.h"

using namespace Dasher;

CDasherComponent::CDasherComponent(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore)
  :m_pEventHandler(pEventHandler), m_pSettingsStore(pSettingsStore) {
  if (m_pEventHandler != NULL)
    m_pEventHandler->RegisterListener(this);

};
CDasherComponent::~CDasherComponent() {
  if (m_pEventHandler != NULL)
    m_pEventHandler->UnregisterListener(this);
};

void CDasherComponent::InsertEvent(CEvent * pEvent) {
  m_pEventHandler->InsertEvent(pEvent);
};

bool CDasherComponent::GetBoolParameter(int iParameter) const {
  return m_pSettingsStore->GetBoolParameter(iParameter);
}
long CDasherComponent::GetLongParameter(int iParameter) const {
  return m_pSettingsStore->GetLongParameter(iParameter);
}
std::string CDasherComponent::GetStringParameter(int iParameter)const {
  return m_pSettingsStore->GetStringParameter(iParameter);
}
void CDasherComponent::SetBoolParameter(int iParameter, bool bValue) const {
  m_pSettingsStore->SetBoolParameter(iParameter, bValue);
}
void CDasherComponent::SetLongParameter(int iParameter, long lValue) const {
  m_pSettingsStore->SetLongParameter(iParameter, lValue);
}
void CDasherComponent::SetStringParameter(int iParameter, const std::string & sValue) const {
  m_pSettingsStore->SetStringParameter(iParameter, sValue);
}

ParameterType CDasherComponent::GetParameterType(int iParameter) const {
  if (m_pSettingsStore != NULL)
    return m_pSettingsStore->GetParameterType(iParameter);
  return ParamInvalid;
}
std::string CDasherComponent::GetParameterName(int iParameter) const {
  if (m_pSettingsStore != NULL)
    return m_pSettingsStore->GetParameterName(iParameter);
  return "";
}
