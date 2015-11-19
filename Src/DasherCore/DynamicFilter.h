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
  
  void pause() {m_bPaused = true;}
  
 protected:
  ///wraps Model's one-step method to compute the number of steps and minsize
  /// (from framerate) that the Model requires, from just the frame time and a
  /// multiplier to speed.
  /// \param dSpeedMul multiply normal speed of movement by this; 1.0 = normal speed,
  /// 0.0 = go nowhere. This allows for slow start, turbo mode, control nodes being
  /// more "viscous", etc. Values <=0.0 will result in no movement
  /// \return true if dSpeedMul>0.0, false if <=0.0.
  bool OneStepTowards(CDasherModel *pModel, myint X, myint Y, unsigned long iTime, double dSpeedMul);
  
  ///Calculates a multiplier by which to adjust our speed (for a given frame).
  /// Defalut implementation implements slow-start (i.e. a multiplier increasing
  /// from zero to one over the slow-start-time) as well as by checking the speedMul
  /// of the node under the cursor.
  virtual double FrameSpeedMul(CDasherModel *pModel, unsigned long iTime);

  ///Starts moving, i.e. just records that we are no longer paused,
  /// for the next call to Timer. Protected, to prevent external calls 
  /// so subclasses have control over all calls, as a call to run() may not be
  /// appropriate without performing other setup first.
  /// \param Time Time in ms, used to keep a constant frame rate and
  /// initialize slow start.

  virtual void run(unsigned long iTime);
  bool isPaused() {return m_bPaused;}
  
  CFrameRate * const m_pFramerate;
 private:
  //Time at which Unpause() was called, used for Slow Start.
  unsigned long m_iStartTime;
  bool m_bPaused;
};
}
#endif
