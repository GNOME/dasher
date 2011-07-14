// CircleStartHandler.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "../Common/Common.h"

#include "CircleStartHandler.h"
#include "DefaultFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"
#include "DasherInput.h"

using namespace Dasher;

CCircleStartHandler::CCircleStartHandler(CDefaultFilter *pCreator)
: CStartHandler(pCreator), CSettingsUserObserver(pCreator), m_iEnterTime(std::numeric_limits<long>::max()), m_iScreenRadius(-1), m_pView(NULL) {
}

CCircleStartHandler::~CCircleStartHandler() {
  if (m_pView) m_pView->Observable<CDasherView*>::Unregister(this);
}

void CCircleStartHandler::ComputeScreenLoc(CDasherView *pView) {
  if (pView != m_pView) {
    if (m_pView) m_pView->Observable<CDasherView*>::Unregister(this);
    (m_pView=pView)->Observable<CDasherView*>::Register(this);
  } else if (m_iScreenRadius!=-1) return;

  pView->Dasher2Screen(CDasherModel::ORIGIN_X, CDasherModel::ORIGIN_Y, m_screenCircleCenter.x, m_screenCircleCenter.y);
  //compute radius against orientation. It'd be simpler to use
  // Math.min(screen width, screen height) * LP_CIRCLE_PERCENT / 100
  // - should we?
  screenint iEdgeX, iEdgeY;
  pView->Dasher2Screen(CDasherModel::ORIGIN_X, CDasherModel::ORIGIN_Y + (CDasherModel::MAX_Y*GetLongParameter(LP_CIRCLE_PERCENT))/100, iEdgeX, iEdgeY);

  const Opts::ScreenOrientations iDirection(pView->GetOrientation());

  if((iDirection == Opts::TopToBottom) || (iDirection == Opts::BottomToTop)) {
    m_iScreenRadius = iEdgeX - m_screenCircleCenter.x;
  }
  else {
    m_iScreenRadius = iEdgeY - m_screenCircleCenter.y;
  }
}

bool CCircleStartHandler::DecorateView(CDasherView *pView) {
  ComputeScreenLoc(pView);

  const bool bAboutToChange = m_bInCircle && m_iEnterTime != std::numeric_limits<long>::max();
  int fillColor, lineColor, lineWidth;
  if (GetBoolParameter(BP_DASHER_PAUSED)) {
    lineColor=2; lineWidth=1;
    fillColor = bAboutToChange ? 241 : 242;
  } else {
    lineColor=240; fillColor=-1; //don't fill
    lineWidth = bAboutToChange ? 3 : 1;
  }

  pView->Screen()->DrawCircle(m_screenCircleCenter.x, m_screenCircleCenter.y, m_iScreenRadius, fillColor, lineColor, lineWidth);

  return true;
}

void CCircleStartHandler::Timer(int iTime, dasherint mouseX, dasherint mouseY,CDasherView *pView) {
  ComputeScreenLoc(pView);
  screenint x,y;
  pView->Dasher2Screen(mouseX, mouseY, x, y);
  x-=m_screenCircleCenter.x; y-=m_screenCircleCenter.y;
  const bool inCircleNow = x*x + y*y <= (m_iScreenRadius * m_iScreenRadius);

  if (inCircleNow) {
    if (m_bInCircle) {
      //still in circle...check they aren't still in there after prev. activation
      if (m_iEnterTime != std::numeric_limits<long>::max() && iTime - m_iEnterTime > 1000) {
        //activate!
        if (GetBoolParameter(BP_DASHER_PAUSED))
          m_pFilter->Unpause(iTime);
        else
          m_pFilter->m_pInterface->Stop();
        //note our HandleEvent method will then set
        //   m_iEnterTime = std::numeric_limits<long>::max()
        // thus preventing us from firing until user leaves circle and enters again
      }
    } else {// !m_bInCircle
      //just entered circle
      m_bInCircle=true;
      m_iEnterTime = iTime;
    }
  } else {
    //currently outside circle
    m_bInCircle=false;
  }
}

void CCircleStartHandler::HandleEvent(int iParameter) {
  switch (iParameter) {
    case LP_CIRCLE_PERCENT:
      //recompute geometry.
      m_iScreenRadius = -1;
      break;
    case BP_DASHER_PAUSED:
      m_iEnterTime = std::numeric_limits<long>::max();
      //In one-dimensional mode, we have that (un)pausing can _move_ the circle, thus,
      // clicking (or using any other start mechanism) can cause the circle to appear
      // around the mouse. If this happens, you should have to leave and re-enter
      // the circle before the start handler does anything. The following ensures this.
      m_bInCircle = true;
      break;
  }
}

void CCircleStartHandler::HandleEvent(CDasherView *pNewView) {
  //need to recompute geometry...
  m_iScreenRadius = -1; //even if it's the same view
  ComputeScreenLoc(pNewView);
}
