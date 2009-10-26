// TwoPushDynamicFilter.cpp
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

#include "TwoPushDynamicFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_TWO_PUSH_OUTER, T_LONG, 1024, 2048, 2048, 128, _("Offset for outer (second) button")},
  {LP_TWO_PUSH_UP, T_LONG, 256, 2048, 2048/*divisor*/, 128/*step*/, _("Distance for 1st button UP")},
  {LP_TWO_PUSH_DOWN, T_LONG, 256, 2048, 2048, 128, _("Distance for 1st button DOWN")},
  {LP_TWO_PUSH_TOLERANCE, T_LONG, 50, 1000, 1, 10, _("Tolerance for inaccurate timing of button pushes (in ms)")},
  /* TRANSLATORS: The time for which a button must be held before it counts as a 'long' (rather than short) press. */
  {LP_HOLD_TIME, T_LONG, 100, 10000, 1000, 100, _("Long press time")},
  /* TRANSLATORS: Backoff = reversing in Dasher to correct mistakes. This allows a single button to be dedicated to activating backoff, rather than using multiple presses of other buttons, and another to be dedicated to starting and stopping. 'Button' in this context is a physical hardware device, not a UI element.*/
  {BP_BACKOFF_BUTTON,T_BOOL, -1, -1, -1, -1, _("Enable backoff and start/stop buttons")},
  {BP_SLOW_START,T_BOOL, -1, -1, -1, -1, _("Slow startup")},
  {LP_SLOW_START_TIME, T_LONG, 0, 10000, 1000, 100, _("Slow startup time")},
  {LP_DYNAMIC_SPEED_INC, T_LONG, 1, 100, 1, 1, _("Percentage by which to automatically increase speed")},
  {LP_DYNAMIC_SPEED_FREQ, T_LONG, 1, 1000, 1, 1, _("Time after which to automatically increase speed (secs)")},
  {LP_DYNAMIC_SPEED_DEC, T_LONG, 1, 99, 1, 1, _("Percentage by which to decrease speed upon reverse")},
  {LP_DYNAMIC_BUTTON_LAG, T_LONG, 0, 1000, 1, 25, _("Lag before user actually pushes button (ms)")}, 
};

CTwoPushDynamicFilter::CTwoPushDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CDynamicFilter(pEventHandler, pSettingsStore, pInterface, 14, 1, _("Two-push Dynamic Mode (New One Button)")), m_dNatsSinceFirstPush(-std::numeric_limits<double>::infinity()) {
  
  Dasher::CParameterNotificationEvent oEvent(LP_TWO_PUSH_OUTER);//and all the others too!
  HandleEvent(&oEvent);
}

void GuideLine(CDasherView *pView, const myint iDasherY, const int iColour)
{
  myint iDasherX = -100;
  CDasherScreen::point p[2];
  CDasherScreen *pScreen(pView->Screen());
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);

  iDasherX = -1000;

  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);

  pScreen->Polyline(p, 2, 3, iColour);
}  

bool CTwoPushDynamicFilter::DecorateView(CDasherView *pView)
{  
  //outer guides (yellow rects)
  for (int i=0; i<2; i++)
  {
    screenint x1, y1, x2, y2;
    CDasherScreen *pScreen(pView->Screen());
  
    pView->Dasher2Screen(-100, m_aaiGuideAreas[i][0], x1, y1);
    pView->Dasher2Screen(-1000, m_aaiGuideAreas[i][1], x2, y2);
  
    pScreen->DrawRectangle(x1, y1, x2, y2, 62/*pale yellow*/, -1, Opts::Nodes1, false, true, 0);
  }

  //inner guides (red lines)
  GuideLine(pView, 2048 - GetLongParameter(LP_TWO_PUSH_UP), 1);
  GuideLine(pView, 2048 + GetLongParameter(LP_TWO_PUSH_DOWN), 1);

  //outer guides (at center of rects) - red lines
  GuideLine(pView, 2048 - GetLongParameter(LP_TWO_PUSH_OUTER), 1);
  GuideLine(pView, 2048 + GetLongParameter(LP_TWO_PUSH_OUTER), 1);

  //moving markers - green if active, else yellow
  if (m_bDecorationChanged && isRunning() && m_dNatsSinceFirstPush > -std::numeric_limits<double>::infinity())
  {
    for (int i = 0; i < 2; i++)
    {
      GuideLine(pView, m_aiMarker[i], (i == m_iActiveMarker) ? 240 : 61/*orange*/);
    }
  }
  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

void CTwoPushDynamicFilter::HandleEvent(Dasher::CEvent * pEvent)
{
  if(pEvent->m_iEventType == EV_PARAM_NOTIFY)
  {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter)
    {
      case LP_TWO_PUSH_OUTER: //deliberate fallthrough
      case LP_TWO_PUSH_UP: //deliberate fallthrough
      case LP_TWO_PUSH_DOWN:
      {
//cout << "Initializing - outer " << GetLongParameter(LP_TWO_PUSH_OUTER) << " up " << GetLongParameter(LP_TWO_PUSH_UP) << " down " << GetLongParameter(LP_TWO_PUSH_DOWN) << "\n";
	DASHER_ASSERT (GetLongParameter(LP_TWO_PUSH_UP) < GetLongParameter(LP_TWO_PUSH_OUTER));
	DASHER_ASSERT (GetLongParameter(LP_TWO_PUSH_DOWN) < GetLongParameter(LP_TWO_PUSH_OUTER));
	DASHER_ASSERT (GetLongParameter(LP_TWO_PUSH_UP) > GetLongParameter(LP_TWO_PUSH_DOWN));
		//TODO, that means short gap at the top - allow other way around also?

	double dOuter = GetLongParameter(LP_TWO_PUSH_OUTER);
	m_dLogUpMul = log(dOuter / (double)GetLongParameter(LP_TWO_PUSH_UP));
	m_dLogDownMul = log(dOuter / (double)GetLongParameter(LP_TWO_PUSH_DOWN));
//cout << "bitsUp " << m_dLogUpMul << " bitsDown " << m_dLogDownMul << "\n";
      } //and fallthrough
      case LP_TWO_PUSH_TOLERANCE:
      case LP_MAX_BITRATE:
      case LP_BOOSTFACTOR: // Deliberate fallthrough
      {
	double dMaxRate = GetLongParameter(LP_MAX_BITRATE) * GetLongParameter(LP_BOOSTFACTOR) / 10000.0;
	double dPressBits = dMaxRate * (double) GetLongParameter(LP_TWO_PUSH_TOLERANCE) / 1000.0;
//cout << "Max Bitrate changed - now " << dMaxRate << " user accuracy " << dPressBits;
	m_dMinShortTwoPushTime = m_dLogUpMul - dPressBits;
//cout << "bits; minShort " << m_dMinShortTwoPushTime;
	m_dMaxShortTwoPushTime = m_dLogUpMul + dPressBits;
	m_dMinLongTwoPushTime = m_dLogDownMul - dPressBits;
	if (m_dMaxShortTwoPushTime > m_dMinLongTwoPushTime)
          m_dMaxShortTwoPushTime = m_dMinLongTwoPushTime = (m_dLogUpMul + m_dLogDownMul)/2.0;
	m_dMaxLongTwoPushTime = m_dLogDownMul + dPressBits;
//TODO, what requirements do we actually need to make to ensure sanity (specifically, that computed m_aiTarget's are in range)?
//cout << " maxShort " << m_dMaxShortTwoPushTime << " minLong " << m_dMinLongTwoPushTime << " maxLong " << m_dMaxLongTwoPushTime << "\n";
	m_bDecorationChanged = true;
     }  //and fallthrough again
     case LP_DYNAMIC_BUTTON_LAG:
     {
       double dMaxRate = GetLongParameter(LP_MAX_BITRATE) * GetLongParameter(LP_BOOSTFACTOR) / 10000.0;
       m_dLagBits = dMaxRate * GetLongParameter(LP_DYNAMIC_BUTTON_LAG)/1000.0;
  //cout << " lag (" << m_dLagBits[0] << ", " << m_dLagBits[1] << ", " << m_dLagBits[2] << ", " << m_dLagBits[3] << ")";
       m_aaiGuideAreas[0][0] = 2048 - GetLongParameter(LP_TWO_PUSH_UP)*exp(m_dMaxShortTwoPushTime);
       m_aaiGuideAreas[0][1] = 2048 - GetLongParameter(LP_TWO_PUSH_UP)*exp(m_dMinShortTwoPushTime);
       m_aaiGuideAreas[1][0] = 2048 + GetLongParameter(LP_TWO_PUSH_DOWN)*exp(m_dMinLongTwoPushTime);
       m_aaiGuideAreas[1][1] = 2048 + GetLongParameter(LP_TWO_PUSH_DOWN)*exp(m_dMaxLongTwoPushTime);
	     break;
     }
    }
  }

};

void CTwoPushDynamicFilter::KeyDown(int Time, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY) {
  if (iId == 100 && !GetBoolParameter(BP_BACKOFF_BUTTON))
    //mouse click - will be ignored by superclass method.
    //simulate press of button 2...
    CDynamicFilter::KeyDown(Time, 2, pDasherView, pModel, pUserLog);
  else
    CInputFilter::KeyDown(Time, iId, pDasherView, pModel, pUserLog, bPos, iX, iY);
}

void CTwoPushDynamicFilter::KeyUp(int Time, int iId, CDasherView *pDasherView, CDasherModel *pModel, bool bPos, int iX, int iY) {
  if (iId == 100 && !GetBoolParameter(BP_BACKOFF_BUTTON))
  //mouse click - will be ignored by superclass method.
  //simulate press of button 2...
    CDynamicFilter::KeyUp(Time, 2, pDasherView, pModel);
  else
    CInputFilter::KeyUp(Time, iId, pDasherView, pModel, bPos, iX, iY);
}

void CTwoPushDynamicFilter::ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) {
  // Types:
  // 0 = ordinary click
  // 1 = long click
  
  if (iType != 0) {
    reverse();
    return;
  }
  if (m_dNatsSinceFirstPush == -std::numeric_limits<double>::infinity()) //no button pushed (recently)
  {
    m_dNatsSinceFirstPush = pModel->GetNats();
    //note, could be negative if overall reversed since last ResetNats (Offset)
//cout << "First push - got " << m_dNatsSinceFirstPush << std::endl;
  }
  else
  {
//cout << "Second push - event type " << iType << " logGrowth " << pModel->GetNats() << "\n";
    if (m_iActiveMarker == -1)
      reverse();
    else
    {
      pModel->Offset(m_aiTarget[m_iActiveMarker]);
      pModel->ResetNats();
      //don't really have to reset there, but seems as good a place as any
      m_dNatsSinceFirstPush = -std::numeric_limits<double>::infinity(); //"waiting for first push"
    }
  }
}

bool doSet(int &var, const int val)
{
  if (var == val) return false;
  var = val;
  return true;
}

bool CTwoPushDynamicFilter::TimerImpl(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol)
{
  DASHER_ASSERT(isRunning());
  if (m_dNatsSinceFirstPush > -std::numeric_limits<double>::infinity()) // first button has been pushed
  {
    double dLogGrowth(m_pDasherModel->GetNats() - m_dNatsSinceFirstPush), dOuter(GetLongParameter(LP_TWO_PUSH_OUTER)),
           dUp(GetLongParameter(LP_TWO_PUSH_UP)), dDown(GetLongParameter(LP_TWO_PUSH_DOWN));
    
    //to move to point currently at outer marker: set m_aiTarget to dOuter==exp( log(dOuter/dUp) ) * dUp
              // (note that m_dLogUpMul has already been set to log(dOuter/dUp)...)
    //to move to point that _was_ at inner marker: set to exp(dLogGrowth) * dUp
    //to move to midpoint - weighting both equally - set to exp( (log(dOuter/dup)+dLogGrowth)/2.0 ) * dUp
    //we move to a WEIGHTED average, with the weights being given by dUp, dDown, and dOuter...
    double dUpBits = (m_dLogUpMul * dOuter + dLogGrowth * dUp) / (dOuter + dUp);
    double dDownBits = (m_dLogDownMul * dOuter + dLogGrowth * dDown) / (dOuter + dDown);
    
    // (note it's actually slightly more complicated even than that, we have to add in m_dLagBits too!)

    double dUpDist = exp( dUpBits ) * dUp;
    double dDownDist = exp( dDownBits ) * dDown;
    m_aiTarget[0] = dUpDist * exp(m_dLagBits);
    m_aiTarget[1] = -dDownDist * exp(m_dLagBits);
    m_bDecorationChanged |= doSet(m_aiMarker[0], 2048 - exp(m_dLagBits + dLogGrowth) * dUp);
    m_bDecorationChanged |= doSet(m_aiMarker[1], 2048 + exp(m_dLagBits + dLogGrowth) * dDown);
    if (dLogGrowth > m_dMaxLongTwoPushTime)
    {
//cout << " growth " << dLogGrowth << " - reversing\n";
      //button pushed, but then waited too long.
      reverse();
    }
    else if (dLogGrowth >= m_dMinShortTwoPushTime && dLogGrowth <= m_dMaxShortTwoPushTime)
      m_bDecorationChanged |= doSet(m_iActiveMarker, 0 /*up*/);
    else if (dLogGrowth >= m_dMinLongTwoPushTime)
      m_bDecorationChanged |= doSet(m_iActiveMarker, 1 /*down*/);
    else m_bDecorationChanged |= doSet(m_iActiveMarker, -1 /*in middle (neither/both) or too short*/);
  }
  m_pDasherModel->OneStepTowards(100, 2048, iTime, pAdded, pNumDeleted);
  return true;
}

void CTwoPushDynamicFilter::Activate() {
  SetBoolParameter(BP_SMOOTH_OFFSET, true);
}

void CTwoPushDynamicFilter::Deactivate() {
  SetBoolParameter(BP_SMOOTH_OFFSET, false);
}

void CTwoPushDynamicFilter::run() {
  m_dNatsSinceFirstPush = -std::numeric_limits<double>::infinity();
  SetBoolParameter(BP_SMOOTH_OFFSET, true);
  CDynamicFilter::run();
}

void CTwoPushDynamicFilter::pause() {
  SetBoolParameter(BP_SMOOTH_OFFSET, false);
  CDynamicFilter::pause();
}

void CTwoPushDynamicFilter::reverse() {
  //hmmmm. If we ever actually did Offset() while reversing,
  // we might want BP_SMOOTH_OFFSET on....
  SetBoolParameter(BP_SMOOTH_OFFSET, false);
  CDynamicFilter::reverse();
}

bool CTwoPushDynamicFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};

bool CTwoPushDynamicFilter::GetMinWidth(int &iMinWidth) {
  iMinWidth = 1024;
  return true;
}
