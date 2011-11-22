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

CDynamicFilter::CDynamicFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName)
: CInputFilter(pInterface, iID, szName), CSettingsUser(pCreator),
  m_bPaused(true), m_pFramerate(pFramerate), m_dLastBits(-1) {
}

bool CDynamicFilter::OneStepTowards(CDasherModel *pModel, myint X, myint Y, unsigned long iTime, double dSpeedMul) {
  if (dSpeedMul<=0.0) return false; //going nowhere
  m_pFramerate->RecordFrame(iTime); //Hmmm, even if we don't do anything else?

  //The maximum number of bits we should allow to be entered in this frame:
  // (after adjusting for slow start, turbo mode, control node slowdown, etc.)
  double dBits = m_pFramerate->GetMaxBitsPerFrame()*dSpeedMul;

  //Compute max expansion, i.e. the minimum size we should allow the range 0..MAX_Y
  // to be shrunk down to, for this frame. We cache the most-recent result to
  // avoid an exp() (and a division): in the majority of cases this doesn't change
  // between frames, but only does so when the maxbitrate changes, or dspeedmul
  // changes (e.g. continuously during slow start, or when entering/leaving turbo
  // mode or a control node).
  if (dBits != m_dLastBits) m_iLastMinSize = static_cast<myint>(CDasherModel::MAX_Y / exp(m_dLastBits = dBits));
  //However, note measurements on iPhone suggest even one exp() per frame is not
  // a significant overhead; so the caching may be unnecessary, but it's easy.
  
  //If we wanted to take things further we could generalize this cache to cover
  // exp()s done in the dynamic button modes too, and thus to allow them to adjust
  // lag, guide markers, etc., according to the dSpeedMul in use. (And/or
  // to do slow-start more efficiently by interpolating cache values.)
  pModel->ScheduleOneStep(X, Y,
                          static_cast<int>(m_pFramerate->Steps() / dSpeedMul),
                          m_iLastMinSize);
  return true;
}

double CDynamicFilter::FrameSpeedMul(CDasherModel *pModel, unsigned long iTime) {
  CDasherNode *n = pModel->Get_node_under_crosshair();
  double d = n ? n->SpeedMul() : 1.0;
  if(GetBoolParameter(BP_SLOW_START)) {
    if ((iTime - m_iStartTime) < GetLongParameter(LP_SLOW_START_TIME))
      d *= 0.1 * (1 + 9 * ((iTime - m_iStartTime) / static_cast<double>(GetLongParameter(LP_SLOW_START_TIME))));
  }
  //else, no slow start, or finished.
  return d;
}

void CDynamicFilter::run(unsigned long Time) {
  if (!isPaused()) return; //already running, no need to / can't really do anything
  
  m_bPaused = false;

  m_pFramerate->Reset_framerate(Time);
  m_iStartTime = Time;
}
