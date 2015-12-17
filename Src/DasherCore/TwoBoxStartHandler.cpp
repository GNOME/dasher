#include "TwoBoxStartHandler.h"
#include "DefaultFilter.h"
#include "DasherView.h"

using namespace Dasher;

CTwoBoxStartHandler::CTwoBoxStartHandler(CDefaultFilter *pCreator)
: CStartHandler(pCreator), CSettingsUser(pCreator), m_bFirstBox(true), m_iBoxEntered(std::numeric_limits<long>::max()) {
}

bool CTwoBoxStartHandler::DecorateView(CDasherView *pView) {
  if (!m_pFilter->isPaused()) return false;
  
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

void CTwoBoxStartHandler::Timer(unsigned long iTime, dasherint iDasherX, dasherint iDasherY, CDasherView *pView) { 
  if (!m_pFilter->isPaused()) return;
  
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

  if ((iNewScreenY >= iBoxMin) && (iNewScreenY <= iBoxMax) 
	  && (iNewScreenX >= 8) && (iNewScreenX <= pView->Screen()->GetWidth() - 16)
	  && pView->Screen()->IsWindowUnderCursor()) {
    if(m_iBoxEntered == std::numeric_limits<long>::max()) {
      m_iBoxEntered = iTime;
    }
    else if (iTime - m_iBoxEntered > 2000) {
      m_iBoxStart = iTime;

      if(m_bFirstBox)
        m_bFirstBox=false;
      else
        m_pFilter->run(iTime);
      m_iBoxEntered = std::numeric_limits<long>::max();
    }
  } else {
    //not in box
    if(!m_bFirstBox && (iTime - m_iBoxStart > 2000))
      m_bFirstBox=true;
    
    m_iBoxEntered = std::numeric_limits<long>::max();
  }
}

void CTwoBoxStartHandler::onPause() {
    m_bFirstBox = true;
}
