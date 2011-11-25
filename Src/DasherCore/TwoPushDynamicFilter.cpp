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
  {LP_TWO_PUSH_LONG, T_LONG, 128, 1024, 2048/*divisor*/, 128/*step*/, _("Distance between down markers (long gap)")},
  {LP_TWO_PUSH_SHORT, T_LONG, 10, 90, 100, 1, _("Distance between up markers, as percentage of long gap")},
  {LP_TWO_PUSH_TOLERANCE, T_LONG, 50, 1000, 1, 10, _("Tolerance for inaccurate timing of button pushes (in ms)")},
  {BP_TWO_PUSH_RELEASE_TIME, T_BOOL, -1, -1, -1, -1, _("Use push and release times of single press rather than push times of two presses")},
  /* TRANSLATORS: Backoff = reversing in Dasher to correct mistakes. This allows a single button to be dedicated to activating backoff, rather than using multiple presses of other buttons, and another to be dedicated to starting and stopping. 'Button' in this context is a physical hardware device, not a UI element.*/
  {BP_BACKOFF_BUTTON,T_BOOL, -1, -1, -1, -1, _("Enable backoff and start/stop buttons")},
  {BP_SLOW_START,T_BOOL, -1, -1, -1, -1, _("Slow startup")},
  {LP_SLOW_START_TIME, T_LONG, 0, 10000, 1000, 100, _("Slow startup time")},
  {LP_DYNAMIC_SPEED_INC, T_LONG, 1, 100, 1, 1, _("Percentage by which to automatically increase speed")},
  {LP_DYNAMIC_SPEED_FREQ, T_LONG, 1, 1000, 1, 1, _("Time after which to automatically increase speed (secs)")},
  {LP_DYNAMIC_SPEED_DEC, T_LONG, 1, 99, 1, 1, _("Percentage by which to decrease speed upon reverse")},
  {LP_DYNAMIC_BUTTON_LAG, T_LONG, 0, 1000, 1, 25, _("Lag before user actually pushes button (ms)")}, 
};

CTwoPushDynamicFilter::CTwoPushDynamicFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate)
  : CDynamicButtons(pCreator, pInterface, pFramerate, 14, _("Two-push Dynamic Mode (New One Button)")), CSettingsObserver(pCreator), m_dNatsSinceFirstPush(-std::numeric_limits<double>::infinity()) {
  
  HandleEvent(LP_TWO_PUSH_OUTER);//and all the others too!
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

long CTwoPushDynamicFilter::downDist() {
  return GetLongParameter(LP_TWO_PUSH_OUTER) - GetLongParameter(LP_TWO_PUSH_LONG);
}

long CTwoPushDynamicFilter::upDist() {
  return GetLongParameter(LP_TWO_PUSH_OUTER) - (GetLongParameter(LP_TWO_PUSH_LONG) * GetLongParameter(LP_TWO_PUSH_SHORT))/100;
}

bool CTwoPushDynamicFilter::DecorateView(CDasherView *pView, CDasherInput *pInput) {
  if (isRunning()) {
    //outer guides (yellow rects)
    for (int i=0; i<2; i++) {
      screenint x1, y1, x2, y2;
      CDasherScreen *pScreen(pView->Screen());
    
      pView->Dasher2Screen(-100, m_aaiGuideAreas[i][0], x1, y1);
      pView->Dasher2Screen(-1000, m_aaiGuideAreas[i][1], x2, y2);
    
      pScreen->DrawRectangle(x1, y1, x2, y2, 62/*pale yellow*/, -1, 0);
    }
  }

  //inner guides (red lines).
  GuideLine(pView, 2048 - upDist(), 1);
  GuideLine(pView, 2048 + downDist(), 1);

  //outer guides (at center of rects) - red lines
  GuideLine(pView, 2048 - GetLongParameter(LP_TWO_PUSH_OUTER), 1);
  GuideLine(pView, 2048 + GetLongParameter(LP_TWO_PUSH_OUTER), 1);

  //moving markers - green if active, else yellow
  if (m_bDecorationChanged && isRunning() && m_dNatsSinceFirstPush > -std::numeric_limits<double>::infinity()) {
    for (int i = 0; i < 2; i++) {
      GuideLine(pView, m_aiMarker[i], (i == m_iActiveMarker) ? 240 : 61/*orange*/);
    }
  }
  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

void CTwoPushDynamicFilter::HandleEvent(int iParameter) {
  switch (iParameter) {
  case LP_TWO_PUSH_OUTER: //fallthrough
  case LP_TWO_PUSH_LONG: //fallthrough
  case LP_TWO_PUSH_SHORT: {
    //TODO, short gap always at the top - allow other way around also?
    double dOuter = GetLongParameter(LP_TWO_PUSH_OUTER);
    m_dLogUpMul = log(dOuter / upDist());
    m_dLogDownMul = log(dOuter / downDist());
//cout << "bitsUp " << m_dLogUpMul << " bitsDown " << m_dLogDownMul << std::endl;
  } //and fallthrough
  case LP_TWO_PUSH_TOLERANCE: //fallthrough
  case LP_DYNAMIC_BUTTON_LAG:
    //recompute rest in Timer
    m_dLastBitRate=-numeric_limits<double>::infinity();
  }
}

void CTwoPushDynamicFilter::updateBitrate(double dBitrate) {
  if (dBitrate==m_dLastBitRate) return;
  m_dLastBitRate = dBitrate;

  double dPressBits = dBitrate * (double) GetLongParameter(LP_TWO_PUSH_TOLERANCE) / 1000.0;
//cout << "Max Bitrate changed - now " << dBitrate << " user accuracy " << dPressBits;
  m_dMinShortTwoPushTime = m_dLogUpMul - dPressBits;
  m_dMaxShortTwoPushTime = m_dLogUpMul + dPressBits;
  m_dMinLongTwoPushTime = m_dLogDownMul - dPressBits;
  if (m_dMaxShortTwoPushTime > m_dMinLongTwoPushTime)
    m_dMaxShortTwoPushTime = m_dMinLongTwoPushTime = (m_dLogUpMul + m_dLogDownMul)/2.0;
  m_dMaxLongTwoPushTime = m_dLogDownMul + dPressBits;
  //TODO, what requirements do we actually need to make to ensure sanity (specifically, that computed m_aiTarget's are in range)?
//cout << "bits; minShort " << m_dMinShortTwoPushTime << " maxShort " << m_dMaxShortTwoPushTime << " minLong " << m_dMinLongTwoPushTime << " maxLong " << m_dMaxLongTwoPushTime << std::endl;
  m_bDecorationChanged = true;

  m_dLagBits = dBitrate * GetLongParameter(LP_DYNAMIC_BUTTON_LAG)/1000.0;

  const long down(downDist()), up(upDist());
  
  //the boundaries of the guide areas (around the outer markers) are
  // then computed from the number of bits _since_ the inner marker:
  m_aaiGuideAreas[0][0] = 2048 - up*exp(m_dMaxShortTwoPushTime);
  m_aaiGuideAreas[0][1] = 2048 - up*exp(m_dMinShortTwoPushTime);
  m_aaiGuideAreas[1][0] = 2048 + down*exp(m_dMinLongTwoPushTime);
  m_aaiGuideAreas[1][1] = 2048 + down*exp(m_dMaxLongTwoPushTime);
//cout << "Short " << m_aaiGuideAreas[0][0] << " to " << m_aaiGuideAreas[0][1] << ", Long " << m_aaiGuideAreas[1][0] << " to " << m_aaiGuideAreas[1][1];
}

void CTwoPushDynamicFilter::KeyDown(unsigned long Time, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if (iId == 100 && !GetBoolParameter(BP_BACKOFF_BUTTON))
    //mouse click - will be ignored by superclass method.
    //simulate press of button 2...
    iId=2;
  CDynamicButtons::KeyDown(Time, iId, pView, pInput, pModel);
}

void CTwoPushDynamicFilter::KeyUp(unsigned long Time, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if (iId == 100 && !GetBoolParameter(BP_BACKOFF_BUTTON))
    //mouse click - will be ignored by superclass method.
    //simulate press of button 2...
    iId=2;
  if (GetBoolParameter(BP_TWO_PUSH_RELEASE_TIME)
      && isRunning() && iId==m_iHeldId
      && m_dNatsSinceFirstPush!=-numeric_limits<double>::infinity())
    ActionButton(Time, iId, 0, pModel);
  //just records that the key has been released
  CDynamicButtons::KeyUp(Time, iId, pView, pInput, pModel);
}

void CTwoPushDynamicFilter::ActionButton(unsigned long iTime, int iButton, int iType, CDasherModel *pModel) {
  // Types:
  // 0 = ordinary click
  // 1 = long click
  
  if (iType != 0) {
    reverse(iTime);
    return;
  }
  if (m_dNatsSinceFirstPush == -std::numeric_limits<double>::infinity()) {
    //no button pushed (recently)
    m_dNatsSinceFirstPush = pModel->GetNats();
    //note, could be negative if overall reversed since last ResetNats (Offset)
//cout << "First push - got " << m_dNatsSinceFirstPush << std::endl;
  } else {
//cout << "Second push - event type " << iType << " logGrowth " << pModel->GetNats() << std::endl;
    if (m_iActiveMarker == -1)
      reverse(iTime);
    else {
      ApplyOffset(pModel,m_aiTarget[m_iActiveMarker]);
      pModel->ResetNats();
      //don't really have to reset there, but seems as good a place as any
      m_dNatsSinceFirstPush = -std::numeric_limits<double>::infinity(); //"waiting for first push"
    }
  }
}

bool doSet(int &var, const int val) {
  if (var == val) return false;
  var = val;
  return true;
}

void CTwoPushDynamicFilter::TimerImpl(unsigned long iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, CExpansionPolicy **pol) {
  DASHER_ASSERT(isRunning());
  const double dSpeedMul(FrameSpeedMul(m_pDasherModel, iTime));
  updateBitrate(GetLongParameter(LP_MAX_BITRATE)*dSpeedMul/100.0);
  if (m_dNatsSinceFirstPush > -std::numeric_limits<double>::infinity()) {
    // first button has been pushed
    double dLogGrowth(m_pDasherModel->GetNats() - m_dNatsSinceFirstPush), dOuter(GetLongParameter(LP_TWO_PUSH_OUTER)),
           dUp(upDist()), dDown(downDist());
    
    //to move to point currently at outer marker: set m_aiTarget to dOuter==exp( log(dOuter/dUp) ) * dUp
              // (note that m_dLogUpMul has already been set to log(dOuter/dUp)...)
    //to move to point that _was_ at inner marker: set to exp(dLogGrowth) * dUp
    //to move to midpoint - weighting both equally - set to exp( (log(dOuter/dup)+dLogGrowth)/2.0 ) * dUp
    //we move to a WEIGHTED average, with the weights being given by dUp, dDown, and dOuter...
    double dUpBits = (m_dLogUpMul * dOuter + dLogGrowth * dUp) / (dOuter + dUp);
    double dDownBits = (m_dLogDownMul * dOuter + dLogGrowth * dDown) / (dOuter + dDown);
    
    double dUpDist = exp( dUpBits ) * dUp;
    double dDownDist = exp( dDownBits ) * dDown;
    // (note it's actually slightly more complicated even than that, we have to add in m_dLagBits too!)
    
    m_aiTarget[0] = dUpDist * exp(m_dLagBits);
    m_aiTarget[1] = -dDownDist * exp(m_dLagBits);
    m_bDecorationChanged |= doSet(m_aiMarker[0], 2048 - exp(m_dLagBits + dLogGrowth) * dUp);
    m_bDecorationChanged |= doSet(m_aiMarker[1], 2048 + exp(m_dLagBits + dLogGrowth) * dDown);
    if (dLogGrowth > m_dMaxLongTwoPushTime) {
//cout << " growth " << dLogGrowth << " - reversing" << std::endl;
      //button pushed, but then waited too long.
      reverse(iTime);
    } else if (dLogGrowth >= m_dMinShortTwoPushTime && dLogGrowth <= m_dMaxShortTwoPushTime)
      m_bDecorationChanged |= doSet(m_iActiveMarker, 0 /*up*/);
    else if (dLogGrowth >= m_dMinLongTwoPushTime)
      m_bDecorationChanged |= doSet(m_iActiveMarker, 1 /*down*/);
    else m_bDecorationChanged |= doSet(m_iActiveMarker, -1 /*in middle (neither/both) or too short*/);
  }
  OneStepTowards(m_pDasherModel, 100, 2048, iTime, dSpeedMul);
}

void CTwoPushDynamicFilter::run(unsigned long iTime) {
  m_dNatsSinceFirstPush = -std::numeric_limits<double>::infinity();
  CDynamicButtons::run(iTime);
}

bool CTwoPushDynamicFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
}

bool CTwoPushDynamicFilter::GetMinWidth(int &iMinWidth) {
  iMinWidth = 1024;
  return true;
}
