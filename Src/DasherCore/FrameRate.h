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
#include "Event.h"
#include "Parameters.h"
#include "DasherComponent.h"

const double LN2 = log(2.0);

namespace Dasher {
/// \ingroup Model
/// \{

/// keeps track of framerate
/// computes the Steps parameter
/// computes RXmax - which controls the maximum rate of zooming in
class CFrameRate : public CDasherComponent {
public:
  CFrameRate(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore);
  
  virtual void HandleEvent(Dasher::CEvent *pEvent);

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

  ///
  /// Get the current framerate
  ///
  double Framerate() const {
    return m_dFr;
  };

  double Bitrate() const {
    return m_dMaxbitrate;
  }

  virtual void BitrateChanged(double dMaxbitrate) {m_dMaxbitrate = dMaxbitrate;}
  virtual void FramerateChanged(double dFr) {m_dFr = dFr;}
  ///
  /// Reset the framerate class
  /// TODO: Need to check semantics here
  /// Called from CDasherInterfaceBase::UnPause;
  ///
  void Reset_framerate(unsigned long Time) {
    m_iFrames = 0;
    m_iTime = Time;
  }


  void NewFrame(unsigned long Time);
  
private:
  double m_dFr;                 // current frame rate (cache of LP_FRAMERATE/100.0)
  double m_dMaxbitrate;         // the maximum rate of entering information (cache)
  double m_dRXmax;              // the maximum zoomin per frame
  int m_iFrames, m_iTime, m_iTime2, m_iSamples;
  int m_iSteps;                 // the 'Steps' parameter. See djw thesis.
};
/// \}
}
#endif /* #ifndef __FrameRate_h__ */

