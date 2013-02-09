#include "FrameRate.h"

using namespace Dasher;

CFrameRate::CFrameRate(CSettingsUser *pCreator) :
  CSettingsUserObserver(pCreator) {

  //Sampling parameters...
  m_iFrames = 0;
  m_iSamples = 1;
  m_iTime = 0;

  //try and carry on from where we left off at last run
  HandleEvent(LP_X_LIMIT_SPEED);
  //Sets m_dBitsAtLimX and m_iSteps
}

void CFrameRate::RecordFrame(unsigned long Time)
{
  m_iFrames++;

  // Update values once enough samples have been collected
  if(m_iFrames == m_iSamples) {
    unsigned long m_iTime2 = Time;

    // If samples are collected in < 50ms, collect more
    if(m_iTime2 - m_iTime < 50)
      m_iSamples++; 
    // And if it's taking longer than > 80ms, collect fewer, down to a
    // limit of 2
    else if(m_iTime2 - m_iTime > 80) {
      m_iSamples--;
      if(m_iSamples < 2)
        m_iSamples = 2;
    }

    // Calculate the framerate and reset framerate statistics for next
    // sampling period
    if(m_iTime2 - m_iTime > 0) {
      double dFrNow = m_iFrames * 1000.0 / (m_iTime2 - m_iTime);
      //LP_FRAMERATE records a decaying average, smoothed 50:50 with previous value
      SetLongParameter(LP_FRAMERATE, long(GetLongParameter(LP_FRAMERATE) + (dFrNow*100))/2);
      m_iTime = m_iTime2;
      m_iFrames = 0;

    DASHER_TRACEOUTPUT("Fr %f Steps %d Samples %d Time2 %d\n", dFrNow, m_iSteps, m_iSamples, m_iTime2);

    }

  }
}

void CFrameRate::HandleEvent(int iParameter) {
  switch (iParameter) {
    case LP_X_LIMIT_SPEED:
      m_dBitsAtLimX = (log(static_cast<double>(CDasherModel::MAX_Y)) - log (2.*GetLongParameter(LP_X_LIMIT_SPEED)))/log(2.);
      //fallthrough
    case LP_MAX_BITRATE:
    case LP_FRAMERATE:
    //Calculate m_iSteps from the decaying-average framerate, as the number
    // of steps that, at the X limit, will cause LP_MAX_BITRATE bits to be
    // entered per second
    m_iSteps = std::max(1,(int)(GetLongParameter(LP_FRAMERATE)*m_dBitsAtLimX/GetLongParameter(LP_MAX_BITRATE)));
  }
}
