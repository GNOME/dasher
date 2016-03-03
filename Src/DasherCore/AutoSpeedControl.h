#ifndef __AUTO_SPEED_CONTROL_H__
#define __AUTO_SPEED_CONTROL_H__

#include "DasherTypes.h"
#include "DasherView.h"
#include "SettingsStore.h"

#include <deque>

/// \defgroup AutoSpeed Auto speed control
/// @{
namespace Dasher {
  class CAutoSpeedControl : private CSettingsUser {
 public:
  CAutoSpeedControl(CSettingsUser *pCreateFrom);
  
  ///
  /// AUTO-SPEED-CONTROL
  /// This is the main speed control function and drives all of auto speed control.
  /// \param iDasherX non-linear Dasher x coord
  /// \param iDasherY non-linear Dasher y coord
  /// \param dFrameRate The current frame rate
  /// \param pView The current Dasher view class
  ///
  void SpeedControl(myint iDasherX, myint iDasherY, CDasherView *pView);

 private:

  ///
  /// AUTO-SPEED-CONTROL
  /// Calculates the running variance of the angle between the +ve x-axis and the line joining 
  /// the cross hair to the mouse position.
  ///
  inline double Variance();

  ///
  /// AUTO-SPEED-CONTROL
  /// Updates the exclusion radius for auto speed control.
  ///
  inline double UpdateMinRadius();

  ///
  /// AUTO-SPEED-CONTROL
  /// Applies changes to the max bit rate depending on the running variance
  /// of the angle between the +ve x-axis and the line joining 
  /// the cross hair to the mouse position.
  ///
  double UpdateBitrate();

  ///
  /// AUTO-SPEED-CONTROL
  /// Adapts the number of samples taken so that auto speed control
  /// is invariant to clock rate and user ability (!!!).
  ///
  
  inline int UpdateSampleSize(double dFrameRate);

  ///
  /// AUTO-SPEED-CONTROL
  /// Updates the *variances* of the two populations of mixture-of-2-Gaussians
  /// distribution of radii. These are used to calculate the exclusion radius.
  /// \param r radius
  /// \param dFrameRate The current frame rate
  ///
  void UpdateSigmas(double r, double dFrameRate);

  double m_dBitrate; //  stores max bit rate internally
  double m_dSampleScale, m_dSampleOffset; // internal, control sample size
  unsigned int m_nSpeedCounter;  // keep track of how many samples
  unsigned int m_nSpeedSamples;  // upper limit on #samples
  double m_dSpeedMax, m_dSpeedMin; // bit rate always within this range
  double m_dTier1, m_dTier2, m_dTier3, m_dTier4; // variance tolerance tiers 
  double m_dChange1, m_dChange2, m_dChange3, m_dChange4; // fractional changes to bit rate
  double m_dMinRRate; // controls rate at which min. r adapts HIGHER===SLOWER!
  double m_dSensitivity; // not used, control sensitivity of auto speed control
  std::deque<double> m_dequeAngles; // store angles for statistics
  
  //variables for adaptive radius calculations...
  double m_dSigma1, m_dSigma2, m_dMinRadius;
};
}
/// @}

#endif
