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
  
  virtual void HandleEvent(int iParameter);

  ///The maximum amount by which one frame may zoom in. Used as a hard
  /// upper-bound on the speed of movement (however far to RHS the cursor is),
  /// calculated from the most-recent (instantaneous) framerate rather than
  /// the decaying average used for the Steps() parameter.
  double GetMaxBitsPerFrame() {
    return m_dFrameBits;
  }

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

  bool OneStepTowards(CDasherModel *pModel, myint y1, myint y2, unsigned long iTime, double dSpeedMul);
  
private:
  double m_dFrameBits;              // the maximum zoomin per frame
  ///number of frames that have been sampled
  int m_iFrames;
  ///time at which first sampled frame was rendered
  unsigned long m_iTime;
  ///number of frames over which we will compute average framerate
  int m_iSamples;

  int m_iSteps;

  ///updates m_dRXMax and m_iSteps
  /// \param dFrNow current (non-decaying-average) framerate (if available!)
  void UpdateSteps(double dFrNow);
};
/// \}
}
#endif /* #ifndef __FrameRate_h__ */

