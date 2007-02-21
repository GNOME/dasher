// FrameRate.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FrameRate_h__
#define __FrameRate_h__

#include "../Common/Common.h"

using namespace Dasher;

const double LN2 = log(2.0);

/// \ingroup Model
/// \{

/// keeps track of framerate
/// computes the Steps parameter
/// computes RXmax - which controls the maximum rate of zooming in
class CFrameRate {
public:
  CFrameRate();
  void Initialise();

  /// Get the minimum size of the target viewport
  ////// TODO: Eventually fix this so that it uses integer maths internally. 

  // dFactor is a temporary change to the frame rate, allowing for
  // slow start and the like
  myint MinSize(myint t, double dFactor = 1.0) const {
    if(dFactor == 1.0)
      return static_cast < myint > (t / m_dRXmax);
    else
      return static_cast < myint > (t / pow(m_dRXmax, dFactor));
  };

  int Steps() const {
    return m_iSteps;
  }; 

  double Framerate() const {
    return m_dFr;
  };

  void Reset(unsigned long Time);
  void NewFrame(unsigned long Time);
  void SetBitrate(double TargetRate);
  void SetMaxBitrate(double MaxRate);

private:
  double m_dFr;
  double m_dMaxbitrate;         // the maximum rate of entering information
  double m_dRXmax;              // the maximum zoomin per frame
  int m_iFrames, m_iTime, m_iTime2, m_iSamples;
  int m_iSteps;                 // the 'Steps' parameter. See djw thesis.
};
/// \}

inline CFrameRate::CFrameRate() {
  // TODO: This looks obsolete - need to rationalise this
  // maxbitrate should be user-defined and/or adaptive. Currently it is not.
#if defined(_WIN32_WCE)
  m_dMaxbitrate = 5;
#else
  m_dMaxbitrate = 5.5;
#endif

  Initialise();
}

inline void CFrameRate::Initialise(void) {
  m_dRXmax = 2;                 // only a transient effect
  m_iFrames = 0;
  m_iSamples = 1;

  // we dont know the framerate yet - play it safe by setting it high
  m_dFr = 1 << 5;

  // start off very slow until we have sampled framerate adequately
  m_iSteps = 2000;
  m_iTime = 0;                  // Hmmm, User must reset framerate before starting.
}

inline void CFrameRate::NewFrame(unsigned long Time)
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
    // sampleing period
    if(m_iTime2 - m_iTime) {
      m_dFr = m_iFrames * 1000.0 / (m_iTime2 - m_iTime);
      m_iTime = m_iTime2;
      m_iFrames = 0;
    }

    // Update auxiliary variablesq
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

inline void CFrameRate::Reset(unsigned long Time) {
  m_iFrames = 0;
  m_iTime = Time;
}

// TODO: Need to clarify the exact relation between these two values -
// at the moment the max bitrate is all that is used
inline void CFrameRate::SetBitrate(double TargetRate) {
  m_dMaxbitrate = TargetRate;
}

inline void CFrameRate::SetMaxBitrate(double MaxRate) {
  m_dMaxbitrate = MaxRate;
}

#endif /* #ifndef __FrameRate_h__ */
