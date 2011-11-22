// FrameRate.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FrameRate_h__
#define __FrameRate_h__

#include <cmath>
#include "../Common/Common.h"
#include "SettingsStore.h"
#include "DasherModel.h"

namespace Dasher {
/// \ingroup Model
/// \{

/// keeps the framerate (LP_FRAMERATE / 100.0) up-to-date,
/// computes the Steps parameter,
/// computes RXmax - which controls the maximum rate of zooming in
class CFrameRate : public CSettingsUserObserver  {
public:
  CFrameRate(CSettingsUser *pCreator);

  //Responds to a change to LP_FRAMERATE or LP_MAX_BITRATE
  // by recomputing the Steps() parameter.
  virtual void HandleEvent(int iParameter);

  ///The number of frames, in which we will attempt to bring
  /// the target location (under the cursor, or in dynamic button
  /// modes) to the crosshair. See DJW thesis.
  int Steps() const {
    return m_iSteps;
  }; 

  ///
  /// Reset the framerate class
  /// TODO: Need to check semantics here
  /// Called from CDasherInterfaceBase::UnPause;
  ///
  void Reset_framerate(unsigned long Time) {
    m_iFrames = 0;
    m_iTime = Time;
  }

  void RecordFrame(unsigned long Time);
  
private:
  ///number of frames that have been sampled
  int m_iFrames;
  ///time at which first sampled frame was rendered
  unsigned long m_iTime;
  ///number of frames over which we will compute average framerate
  int m_iSamples;

  int m_iSteps;
  
  double m_dBitsAtLimX;
};
/// \}
}
#endif /* #ifndef __FrameRate_h__ */

