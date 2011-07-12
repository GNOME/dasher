// DynamicFilter.cpp
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

#include "DasherInterfaceBase.h"
#include "DynamicFilter.h"

using namespace Dasher;

CDynamicFilter::CDynamicFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName) : CInputFilter(pInterface, iID, szName), CSettingsUser(pCreator), m_pFramerate(pFramerate) {
}

bool CDynamicFilter::OneStepTowards(CDasherModel *pModel, myint y1, myint y2, unsigned long iTime, double dSpeedMul) {
  if (dSpeedMul<=0.0) return false; //going nowhere
  m_pFramerate->RecordFrame(iTime); //Hmmm, even if we don't do anything else?

  double dRXMax = m_pFramerate->GetMaxZoomFactor();
  // Adjust for slow start etc. TODO: can we fix to use integer math (or at least no pow?)
  if (dSpeedMul!=1.0) dRXMax=pow(dRXMax, dSpeedMul);
  
  pModel->OneStepTowards(y1, y2, static_cast<int>(m_pFramerate->Steps() / dSpeedMul), static_cast<myint>(GetLongParameter(LP_MAX_Y)/dRXMax));
  return true;
}

double CDynamicFilter::SlowStartSpeedMul(unsigned long iTime) {
  if(GetBoolParameter(BP_SLOW_START)) {
    if ((iTime - m_iStartTime) < GetLongParameter(LP_SLOW_START_TIME))
      return 0.1 * (1 + 9 * ((iTime - m_iStartTime) / static_cast<double>(GetLongParameter(LP_SLOW_START_TIME))));
  }
  //no slow start, or finished.
  return 1.0;
}

void CDynamicFilter::Unpause(unsigned long Time) {
  if (!GetBoolParameter(BP_DASHER_PAUSED)) return; //already running, no need to / can't really do anything
  
  SetBoolParameter(BP_DASHER_PAUSED, false);

  m_pFramerate->Reset_framerate(Time);
  m_iStartTime = Time;
}
