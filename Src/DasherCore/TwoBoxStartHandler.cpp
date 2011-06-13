#include "TwoBoxStartHandler.h"
#include "Event.h"

using namespace Dasher;

CTwoBoxStartHandler::CTwoBoxStartHandler(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface)
: CStartHandler(pInterface), CSettingsUserObserver(pCreator), m_bFirstBox(true), m_iBoxEntered(std::numeric_limits<long>::max()) {
}

bool CTwoBoxStartHandler::DecorateView(CDasherView *pView) {
  if (!GetBoolParameter(BP_DASHER_PAUSED)) return false;
  
  int iHeight = pView->Screen()->GetHeight();
  int iWidth = pView->Screen()->GetWidth();

  int iMousePosDist = GetLongParameter(LP_MOUSEPOSDIST);

  int lineWidth = m_iBoxEntered == std::numeric_limits<long>::max() ? 2 : 4; //out/in box

  if (m_bFirstBox) {
    pView->Screen()->DrawRectangle(8, iHeight / 2 - iMousePosDist + 50, iWidth-16, iHeight / 2 - iMousePosDist - 50, -1, 119, lineWidth);
  } else {
    pView->Screen()->DrawRectangle(8, iHeight / 2 + iMousePosDist + 50, iWidth-16, iHeight / 2 + iMousePosDist - 50, -1, 120, lineWidth);
  }
  return true;
}

void CTwoBoxStartHandler::Timer(int iTime, dasherint iDasherX, dasherint iDasherY, CDasherView *pView) { 
  if (!GetBoolParameter(BP_DASHER_PAUSED)) return;
  
  int iBoxMin, iBoxMax;
  if(m_bFirstBox) {
    iBoxMax = pView->Screen()->GetHeight() / 2 - (int)GetLongParameter(LP_MOUSEPOSDIST) + 50;
    iBoxMin = iBoxMax - 100;
  }
  else {
    iBoxMin = pView->Screen()->GetHeight() / 2 + (int)GetLongParameter(LP_MOUSEPOSDIST) - 50;
    iBoxMax = iBoxMin + 100;
  }

  screenint iNewScreenX, iNewScreenY;
  pView->Dasher2Screen(iDasherX, iDasherY, iNewScreenX, iNewScreenY);

  if((iNewScreenY >= iBoxMin) && (iNewScreenY <= iBoxMax)) {
    if(m_iBoxEntered == std::numeric_limits<long>::max()) {
      m_iBoxEntered = iTime;
    }
    else if (iTime - m_iBoxEntered > 2000) {
      m_iBoxStart = iTime;

      if(m_bFirstBox)
        m_bFirstBox=false;
      else
        m_pInterface->Unpause(iTime);
      m_iBoxEntered = std::numeric_limits<long>::max();
    }
  } else {
    //not in box
    if(!m_bFirstBox && (iTime - m_iBoxStart > 2000))
      m_bFirstBox=true;
    
    m_iBoxEntered = std::numeric_limits<long>::max();
  }
}

void CTwoBoxStartHandler::HandleEvent(int iParameter) {
  if (iParameter==BP_DASHER_PAUSED)
    m_bFirstBox = true;
}
