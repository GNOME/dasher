#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../Common/Common.h"

#include "AutoSpeedControl.h"

#include <cmath>
#include <cfloat>
#include <cstring>

using namespace Dasher;

CAutoSpeedControl::CAutoSpeedControl(CSettingsUser *pCreateFrom)
  : CSettingsUser(pCreateFrom) {
  //scale #samples by #samples = m_dSamplesScale / (current bitrate) + m_dSampleOffset
  m_dSampleScale = 1.5;
  m_dSampleOffset = 1.3;
  m_dMinRRate = 80.0;
  m_dSensitivity = GetLongParameter(LP_AUTOSPEED_SENSITIVITY) / 100.0; //param only, no GUI!
  //tolerance for automatic speed control
  m_dTier1 = 0.0005;  //  should be arranged so that tier4 > tier3 > tier2 > tier1 !!!
  m_dTier2 = 0.01;
  m_dTier3 = 0.2;
  m_dTier4 = 0.31;
  //bitrate fractional changes for auto-speed control
  m_dChange1 = 1.1;
  m_dChange2 = 1.02;
  m_dChange3 = 0.97;
  m_dChange4 = 0.94;
  //cap bitrate at...
  m_dSpeedMax = 8.0;
  m_dSpeedMin = 0.1;
  //variance of two-centred-gaussians for adaptive radius
  m_dSigma1 = 0.5;
  m_dSigma2 = 0.05;
  //Initialise auto-speed control
  m_nSpeedCounter = 0;
  m_dBitrate = double(round(GetLongParameter(LP_MAX_BITRATE) / 100.0));

  UpdateMinRadius();
  UpdateSampleSize(GetLongParameter(LP_FRAMERATE) / 100.0);
}

  ////////////////////////////////////////////////
  ///
  ///  Change max bitrate based on variance of angle
  ///  in dasher space.
  ///
  /////////////////////////////////////////////////

inline double CAutoSpeedControl::UpdateBitrate()
{
  double var = Variance();
  if(var < m_dTier1)
  {
      m_dBitrate *= m_dChange1;
  }
  else if(var < m_dTier2)
  {
      m_dBitrate *= m_dChange2;
  }
  else if(var > m_dTier4) //Tier 4 comes before tier 3 because tier4 > tier3 !!!
  {
      m_dBitrate *= m_dChange4;
  }
  else if(var > m_dTier3)
  {
      m_dBitrate *= m_dChange3;
  }
  //else if( in the middle )
  //    nothing happens! ;

  //always keep bitrate values sane
  if(m_dBitrate > m_dSpeedMax)
  {
    m_dBitrate = m_dSpeedMax;
  }
  else if(m_dBitrate < m_dSpeedMin)
  {
    m_dBitrate = m_dSpeedMin;
  }

  return m_dBitrate;
}
  ///////////////////////////////////////////////
  ///
  ///  Finds variance for automatic speed control
  ///
  //////////////////////////////////////////////

inline double CAutoSpeedControl::Variance()
{
  double avgcos, avgsin;
  avgsin = avgcos = 0.0;
  // find average of cos(theta) and sin(theta)
  for(auto theta : m_dequeAngles) {
    avgcos += cos(theta);
    avgsin += sin(theta);
  }
  avgcos /= (1.0 * m_dequeAngles.size());
  avgsin /= (1.0 * m_dequeAngles.size());
  //return variance (see dasher/Doc/speedcontrol.tex)
  return -log(avgcos * avgcos + avgsin * avgsin);

}
//////////////////////////////////////////////////////////////////////
///
///  The number of samples depends on the clock rate of the
///  machine (framerate) and the user's speed (bitrate). See
///  speedcontrol.tex in dasher/Doc/ dir.
///
/////////////////////////////////////////////////////////////////////


inline int CAutoSpeedControl::UpdateSampleSize(double dFrameRate)
{
  // for the purposes of this function
  // we don't care exactly how slow we're going
  // *really* low speeds are ~ equivalent?
  double dBitrate = std::max(1.0,m_dBitrate);
  double dSpeedSamples = dFrameRate * (m_dSampleScale / dBitrate + m_dSampleOffset);

  m_nSpeedSamples = int(round(dSpeedSamples));

  return m_nSpeedSamples;
}
  /////////////////////////////////////////////////////////////
  ///
  ///  double UpdateMinRadius() - find adaptive min radius for
  ///  auto-speed control. Calculated by DJCM's
  ///  mixture-of-2-centred-gaussians model.
  ///
  ///////////////////////////////////////////////////////////

inline double CAutoSpeedControl::UpdateMinRadius()
{
  m_dMinRadius = sqrt( log( (m_dSigma2 * m_dSigma2) / (m_dSigma1 * m_dSigma1) ) /
                ( 1 / (m_dSigma1 * m_dSigma1) - 1 / (m_dSigma2 * m_dSigma2)) );
  return m_dMinRadius;
}

//////////////////////////////////////////////////////////////
///
///  NB: updates VARIANCES of two populations of
///  mixture-of-2-centred-Gaussians model!
///
//////////////////////////////////////////////////////////////

inline void CAutoSpeedControl::UpdateSigmas(double r, double dFrameRate)
{
  double dSamples = m_dMinRRate* dFrameRate / m_dBitrate;
  if(r > m_dMinRadius)
    m_dSigma1 = m_dSigma1 - (m_dSigma1 - r * r) / dSamples;
  else
    m_dSigma2 = m_dSigma2 - (m_dSigma2 - r * r) / dSamples;
}

/////////////////////////////////////////////////////////////////
///
///  AUTOMATIC SPEED CONTROL, CEH 7/05: Analyse variance of angle
///  mouse position makes with +ve x-axis in Dasher-space
///
////////////////////////////////////////////////////////////////


void CAutoSpeedControl::SpeedControl(myint iDasherX, myint iDasherY, CDasherView *pView) {
  if (GetBoolParameter(BP_AUTO_SPEEDCONTROL)) {

    //  Coordinate transforms:
    double r, theta;
    pView->Dasher2Polar(iDasherX, iDasherY, r, theta);

    m_dBitrate = GetLongParameter(LP_MAX_BITRATE) / 100.0; //  stored as long(round(true bitrate * 100))
    double dFrameRate = GetLongParameter(LP_FRAMERATE) / 100.0;
    UpdateSigmas(r, dFrameRate);

    //  Data collection:

    if (r > m_dMinRadius && fabs(theta) < 1.25) {
      m_nSpeedCounter++;
      m_dequeAngles.push_back(theta);
      while (m_dequeAngles.size() > m_nSpeedSamples) {
        m_dequeAngles.pop_front();
      }

    }
    m_dSensitivity = GetLongParameter(LP_AUTOSPEED_SENSITIVITY) / 100.0;
    if (m_nSpeedCounter > round(m_nSpeedSamples / m_dSensitivity)) {
      //do speed control every so often!
      UpdateSampleSize(dFrameRate);
      UpdateMinRadius();
      UpdateBitrate();
      long lBitrateTimes100 = long(round(m_dBitrate * 100)); //Dasher settings want long numerical parameters
      SetLongParameter(LP_MAX_BITRATE, lBitrateTimes100);
      m_nSpeedCounter = 0;
    }
  }
}