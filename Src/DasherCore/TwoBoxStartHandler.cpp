#include "TwoBoxStartHandler.h"
#include "Event.h"

using namespace Dasher;

CTwoBoxStartHandler::CTwoBoxStartHandler(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface)
  : CStartHandler(pEventHandler, pSettingsStore, pInterface) {
  m_bInBox = false;
}

bool CTwoBoxStartHandler::DecorateView(CDasherView *pView) {
  int iHeight = pView->Screen()->GetHeight();
  int iWidth = pView->Screen()->GetWidth();

  int iMousePosDist = GetLongParameter(LP_MOUSEPOSDIST);
  int iDrawMousePosBox = GetLongParameter(LP_MOUSE_POS_BOX);

  switch (iDrawMousePosBox) {
  case 1:
    pView->Screen()->DrawRectangle(8, iHeight / 2 - iMousePosDist + 50, iWidth-16, iHeight / 2 - iMousePosDist - 50, 0, 119, Opts::Nodes1,true, false, 4);
    return true;
    break;
  case 2:
    pView->Screen()->DrawRectangle(8, iHeight / 2 + iMousePosDist + 50, iWidth-16, iHeight / 2 + iMousePosDist - 50, 0, 120, Opts::Nodes1,true, false, 4);
    return true;
    break;
  default:
    return false;
    break;
  }
}

void CTwoBoxStartHandler::Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) { 
  myint iDasherX;
  myint iDasherY;
  m_pDasherView->GetCoordinates(iDasherX, iDasherY);
  
  screenint iNewScreenX;
  screenint iNewScreenY;
  m_pDasherView->Dasher2Screen(iDasherX, iDasherY, iNewScreenX, iNewScreenY);

  int iBoxMax(-1);
  int iBoxMin(0);

  if(GetLongParameter(LP_MOUSE_POS_BOX) == 1) {
    iBoxMax = m_pDasherView->Screen()->GetHeight() / 2 - (int)GetLongParameter(LP_MOUSEPOSDIST) + 50;
    iBoxMin = iBoxMax - 100;
  }
  else if(GetLongParameter(LP_MOUSE_POS_BOX) == 2) {
    iBoxMin = m_pDasherView->Screen()->GetHeight() / 2 + (int)GetLongParameter(LP_MOUSEPOSDIST) - 50;
    iBoxMax = iBoxMin + 100;
  }

  if((iNewScreenY >= iBoxMin) && (iNewScreenY <= iBoxMax)) {
    if(!m_bInBox) {
      m_iBoxEntered = iTime;
    }
    else {
      if(iTime - m_iBoxEntered > 2000) {

        m_iBoxStart = iTime;

        if(GetLongParameter(LP_MOUSE_POS_BOX) == 1)
          SetLongParameter(LP_MOUSE_POS_BOX, 2);
        else if(GetLongParameter(LP_MOUSE_POS_BOX) == 2) {
          SetLongParameter(LP_MOUSE_POS_BOX, -1);
	  m_pInterface->Unpause(iTime);
        }
      }
    }

    m_bInBox = true;
  }
  else {
    if((GetLongParameter(LP_MOUSE_POS_BOX) == 2) && (iTime - m_iBoxStart > 2000))
      SetLongParameter(LP_MOUSE_POS_BOX, 1);
    
    m_bInBox = false;
  }
}

void CTwoBoxStartHandler::HandleEvent(Dasher::CEvent * pEvent) {
    if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
   
    switch (pEvt->m_iParameter) {
    case BP_DASHER_PAUSED:
      if(GetBoolParameter(BP_DASHER_PAUSED))
	SetLongParameter(LP_MOUSE_POS_BOX, 1);
      else
	SetLongParameter(LP_MOUSE_POS_BOX, -1);
      break;
    }
  }
}
