#include "FrameRate.h"
namespace Dasher {
CFrameRate::CFrameRate(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore) :
  CDasherComponent(pEventHandler, pSettingsStore) {

  //what follows was once the Initialise() method, but
  // (ACL 21/05/09) is no longer called from elsewhere, hence inlining
  m_dRXmax = 2;                 // only a transient effect
  m_iFrames = 0;
  m_iSamples = 1;

  // start off very slow until we have sampled framerate adequately
  m_iSteps = 2000;
  m_iTime = 0;                  // Hmmm, User must reset framerate before starting.

  //Set bitrate and framerate
  m_dFr = GetLongParameter(LP_FRAMERATE) / 100.0;
  m_dMaxbitrate = GetLongParameter(LP_MAX_BITRATE) * GetLongParameter(LP_BOOSTFACTOR) / 10000.0;
}

void CFrameRate::NewFrame(unsigned long Time)
{
  m_iFrames++;

  // Update values once enough samples have been collected
  if(m_iFrames == m_iSamples) {
    m_iTime2 = Time;

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
      SetLongParameter(LP_FRAMERATE, m_iFrames * 100000.0 / (m_iTime2 - m_iTime));
      m_iTime = m_iTime2;
      m_iFrames = 0;
    }

    // Update auxiliary variables - even if we didn't recalc the framerate
    //   (means we reach sensible values more quickly after first loading)
    m_dRXmax = exp(m_dMaxbitrate * LN2 / m_dFr);
    
    // Note that m_iSteps is smoothed here - 50:50 interpolation with
    // previous value
    m_iSteps = m_iSteps / 2 + (int)(-log(0.2) * m_dFr / LN2 / m_dMaxbitrate) / 2;

    // If the framerate slows to < 4 then we end up with steps < 1 ! 
    if(m_iSteps == 0)
      m_iSteps = 1;

    DASHER_TRACEOUTPUT("Fr %f Steps %d Samples %d Time2 %d rxmax %f\n", m_dFr, m_iSteps, m_iSamples, m_iTime2, m_dRXmax);
  }
}

void CFrameRate::HandleEvent(Dasher::CEvent *pEvent) {

  if(pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {
    case LP_MAX_BITRATE: // Delibarate fallthrough
    case LP_BOOSTFACTOR:
      BitrateChanged(GetLongParameter(LP_MAX_BITRATE) * GetLongParameter(LP_BOOSTFACTOR) / 10000.0);
      break;
    case LP_FRAMERATE:
      FramerateChanged(GetLongParameter(LP_FRAMERATE) / 100.0);
    }
  }
}

}
