#include "WrapperFactory.h"

CWrapperFactory::CWrapperFactory(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherModule *pModule) {
  m_pModule = pModule;
  m_pModule->Ref();

  m_iCurrentPos = 0;
}

CWrapperFactory::~CWrapperFactory() {
  m_pModule->Unref();
  m_pModule = 0;
}

void CWrapperFactory::Start() {
  m_iCurrentPos = 0;
}

bool CWrapperFactory::IsMore() {
  return (m_iCurrentPos < 1);
}

void CWrapperFactory::GetNext(long long int *iID, int *iType, std::string *strName) {
  if(m_iCurrentPos == 0) {
    *iID = m_pModule->GetID();
    *iType = m_pModule->GetType();
    *strName = "Unknown Module";
  }
  ++m_iCurrentPos;
}

CDasherModule *CWrapperFactory::GetModule(long long int iID) {
  if(iID == m_pModule->GetID())
    return m_pModule;
  else
    return 0;
}

std::string CWrapperFactory::GetName(long long int iID) {
  if(iID == m_pModule->GetID())
    return m_pModule->GetName();
  else
    return "Not implemented"; // TODO: Is this the right thing to do here?
}

int CWrapperFactory::GetType(long long int iID) {
  if(iID == m_pModule->GetID())
    return m_pModule->GetType();
  else
    return 0; // TODO: Is this the right thing to do here?
}
