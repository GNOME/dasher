#include "../Common/Common.h"
#include "StylusFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

CStylusFilter::CStylusFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *pDasherModel, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, pDasherModel, iID, szName) {
}


void CStylusFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {
  if(iId == 100)
    m_pInterface->Unpause(iTime);
}

void CStylusFilter::KeyUp(int iTime, int iId, CDasherModel *pModel) {
  if(iId == 100)
    m_pInterface->PauseAt(0, 0);
}
