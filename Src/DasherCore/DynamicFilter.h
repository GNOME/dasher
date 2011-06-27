// DynamicFilter.h
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

#ifndef __DynamicFilter_h__
#define __DynamicFilter_h__

#include "InputFilter.h"
#include "SettingsStore.h"
#include "FrameRate.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
///Abstract superclass for filters which produce continuous movement
/// i.e. steadily towards a target (perhaps moving). These filters
/// need to monitor the framerate (using CFrameRate) to maintain a steady
/// speed of movement. Also implements Slow Start following a call to the
/// Unpause method.
class CDynamicFilter : public CInputFilter, public CSettingsUser {
 public:
  CDynamicFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName);

  virtual bool supportsPause() {return true;}

 protected:
  bool OneStepTowards(CDasherModel *pModel, myint y1, myint y2, unsigned long iTime, double dSpeedMul);
  double SlowStartSpeedMul(unsigned long iTime);

  /// Starts moving.  Clears BP_DASHER_PAUSED.
  /// (But does nothing if BP_DASHER_PAUSED is currently set).
  /// \param Time Time in ms, used to keep a constant frame rate and
  /// initialize slow start.
  void Unpause(unsigned long iTime);
 private:
  CFrameRate *m_pFramerate;
  //Time at which Unpause() was called, used for Slow Start.
  unsigned long m_iStartTime;
};
}
#endif
