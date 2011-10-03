#include "FrameRate.h"

using namespace Dasher;

CFrameRate::CFrameRate(CSettingsUser *pCreator) :
  CSettingsUserObserver(pCreator) {

  //Sampling parameters...
  m_iFrames = 0;
  m_iSamples = 1;
  m_iTime = 0;

  //try and carry on from where we left off at last run
  HandleEvent(LP_MAX_BITRATE);
  //calls UpdateSteps(), which sets m_dRXMax and m_iSteps
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
    double dFrNow;
    if(m_iTime2 - m_iTime > 0) {
      dFrNow = m_iFrames * 1000.0 / (m_iTime2 - m_iTime);
      //LP_FRAMERATE records a decaying average, smoothed 50:50 with previous value
      SetLongParameter(LP_FRAMERATE, long(GetLongParameter(LP_FRAMERATE) + (dFrNow*100))/2);
      m_iTime = m_iTime2;
      m_iFrames = 0;
    } else //best guess: use decaying average
      dFrNow = GetLongParameter(LP_FRAMERATE) / 100.0;

    UpdateSteps(dFrNow);

    DASHER_TRACEOUTPUT("Fr %f Steps %d Samples %d Time2 %d maxbits %f\n", dFrNow, m_iSteps, m_iSamples, m_iTime2, m_dFrameBits);

  }
}

void CFrameRate::HandleEvent(int iParameter) {

  switch (iParameter) {
  case LP_MAX_BITRATE:
    UpdateSteps(GetLongParameter(LP_FRAMERATE) / 100.0); //use the decaying average as current
    break;
  }
}

const double LN2 = log(2.0);
const double STEPS_COEFF = -log(0.2) / LN2;

void CFrameRate::UpdateSteps(double dFrNow) {
  const double dMaxbitrate = GetLongParameter(LP_MAX_BITRATE) / 100.0;
    // Update auxiliary variables - even if we didn't recalc the framerate
    //   (means we reach sensible values more quickly after first loading)
    m_dFrameBits = dMaxbitrate * LN2 / dFrNow;
    
    //Calculate m_iSteps from the decaying-average framerate, and ensure
    // it is at least 1 (else, if framerate slows to <4, we get 0 steps!)
    m_iSteps = std::max(1,(int)(STEPS_COEFF * (GetLongParameter(LP_FRAMERATE)/100.0) / dMaxbitrate));

}
