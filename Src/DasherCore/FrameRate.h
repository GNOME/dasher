// FrameRate.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FrameRate_h__
#define __FrameRate_h__

#include "../Common/MSVC_Unannoy.h"

// keeps track of framerate
// computes the Steps parameter
// computes RXmax - which controls the maximum rate of zooming in

const double LN2= log(2);

class CFrameRate {
	public:
		CFrameRate() ;
		~CFrameRate() {};
		const double Rxmax() const {return m_dRXmax;}
		const int Steps() const {return m_iSteps;}
		const double Framerate() const {return m_dFr;}
		void Reset(unsigned long Time);
		void NewFrame(unsigned long Time);
		
		// TODO: These two shouldn't be the same thing:
		void SetBitrate(double TargetRate);
		void SetMaxBitrate(double MaxRate);
	private:
		double m_dFr;
		double m_dMaxbitrate;    // the maximum rate of entering information
		double m_dRXmax;         // the maximum zoomin per frame
		int m_iFrames,m_iTime,m_iTime2,m_iSamples;
		int m_iSteps;            // the 'Steps' parameter. See djw thesis.
};
	
inline CFrameRate::CFrameRate() {

 // maxbitrate should be user-defined and/or adaptive. Currently it is not.
#if defined(_WIN32_WCE)
	m_dMaxbitrate=5;
#else
	m_dMaxbitrate=5.5;
#endif

	m_dRXmax=2;  // only a transient effect
	m_iFrames=0;
	m_iSamples=1;

	// we dont know the framerate yet - play it safe by setting it high
	m_dFr=1<<5;

	// start off very slow until we have sampled framerate adequately
	m_iSteps=2000;
	m_iTime=0; // Hmmm, User must reset framerate before starting.
}

inline void CFrameRate::NewFrame(unsigned long Time) 
	// compute framerate if we have sampled enough frames
{
	
	m_iFrames++;

	if (m_iFrames==m_iSamples) {
		m_iTime2=Time;
		if (m_iTime2-m_iTime < 50) 
			m_iSamples++;      // increase sample size
		else if (m_iTime2-m_iTime > 80) {
			m_iSamples--;
			if (m_iSamples <2)
				m_iSamples=2;
		}
		if (m_iTime2-m_iTime) {
				m_dFr=m_iFrames*1000.0/(m_iTime2-m_iTime);
				m_iTime=m_iTime2;
				m_iFrames=0;

			}
		m_dRXmax=exp(m_dMaxbitrate*LN2/m_dFr);
		m_iSteps=m_iSteps/2+(int)(-log(0.2)*m_dFr/LN2/m_dMaxbitrate)/2;
	//	dchar debug[256];
	//		_stprintf(debug,TEXT("fr %f Steps %d samples %d time2 %d rxmax %f\n"),fr,Steps,samples,time2,RXmax);
	//	OutputDebugString(debug);
	
	}
}

inline void CFrameRate::Reset(unsigned long Time)
{
	m_iFrames=0;
	m_iTime=Time;
}


inline void CFrameRate::SetBitrate(double TargetRate)
{
	m_dMaxbitrate = TargetRate;
}


inline void CFrameRate::SetMaxBitrate(double MaxRate)
{
	m_dMaxbitrate = MaxRate;
}



#endif /* #ifndef __FrameRate_h__ */
