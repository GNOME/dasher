#include "../Common/Common.h"
#include "StylusFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

CStylusFilter::CStylusFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, iID, szName) {
}


void CStylusFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {
  if(iId == 100)
    m_pInterface->Unpause(iTime);
}

void CStylusFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel) {
  if(iId == 100)
    m_pInterface->PauseAt(0, 0);
}
