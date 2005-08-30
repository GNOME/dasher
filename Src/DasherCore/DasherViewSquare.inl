// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherModel.h"

namespace Dasher {
inline double CDasherViewSquare::UpdateBitrate()
{
  ////////////////////////////////////////////////
  //
  //  Change max bitrate based on variance of angle 
  //  in dasher space.
  //

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
        
inline double CDasherViewSquare::Variance()
{      
  ///////////////////////////////////////////////
  //
  //  Finds variance for automatic speed control
  //

  double dSum1, dSum2, avgcos, avgsin;
  dSum1 = dSum2 = 0.0;
  DOUBLE_DEQUE::iterator i;
  for(i = m_dequeAngles.begin(); i != m_dequeAngles.end(); i++) {
    dSum1 += cos(*i);
    dSum2 += sin(*i);
  }
  avgcos = dSum1 / (1.0 * m_dequeAngles.size());
  avgsin = dSum2 / (1.0 * m_dequeAngles.size());
  return -log(avgcos * avgcos + avgsin * avgsin);

}

inline int CDasherViewSquare::UpdateSampleSize()
{
//  METHOD 2: The number of samples depends on the clock rate of the
//  machine (framerate) and the user's speed (bitrate).

  double dFramerate = DasherModel()->Framerate();
  double dSpeedSamples = 0.0;
  double dBitrate = m_dBitrate; 
  if(dBitrate < 1.0)// for the purposes of this function
    dBitrate = 1.0; // we don't care exactly how slow we're going
                    // *really* low speeds are ~ equivalent?
  dSpeedSamples = dFramerate * (m_dSampleScale / dBitrate + m_dSampleOffset);
  
  m_nSpeedSamples = int(round(dSpeedSamples));
  return m_nSpeedSamples;
}

inline double CDasherViewSquare::UpdateMinRadius() 
{
  /////////////////////////////////////////////////////////////
  //
  //  double UpdateMinRadius() - return adaptive min radius for
  //  auto-speed control. Calculated by DJCM's
  //  mixture-of-2-centred-gaussians model.

  m_dMinRadius = sqrt( log( (m_dSigma2 * m_dSigma2) / (m_dSigma1 * m_dSigma1) ) / 
                ( 1 / (m_dSigma1 * m_dSigma1) - 1 / (m_dSigma2 * m_dSigma2)) );
  return m_dMinRadius;
}

inline void CDasherViewSquare::UpdateSigmas(double r)
{
  double dSamples = 60.0 * DasherModel()->Framerate() / m_dBitrate;//double(m_nSpeedSamples);
  if(r > m_dMinRadius)
    m_dSigma1 = m_dSigma1 - (m_dSigma1 - r * r) / dSamples;
  else 
    m_dSigma2 = m_dSigma2 - (m_dSigma2 - r * r) / dSamples;
}

inline void CDasherViewSquare::SpeedControl(myint iDasherX, myint iDasherY)
{
  /////////////////////////////////////////////////////////////////
  //
  //  AUTOMATIC SPEED CONTROL, CEH 7/05: Analyse variance of angle
  //  mouse position makes with +ve x-axis in Dasher-space
  //

  if(GetBoolParameter(BP_AUTO_SPEEDCONTROL) && !GetBoolParameter(BP_DASHER_PAUSED)) {
    
//  Coordinate transforms:    
    iDasherX = myint(xmap(iDasherX / static_cast < double >(DasherModel()->DasherY())) * DasherModel()->DasherY());
    iDasherY = m_ymap.map(iDasherY);

    myint iDasherOX = myint(xmap(DasherModel()->DasherOX() / static_cast < double >(DasherModel()->DasherY())) * DasherModel()->DasherY());
    myint iDasherOY = m_ymap.map(DasherModel()->DasherOY());

    double x = -(iDasherX - iDasherOX) / double(iDasherOX); //  FIXME - for the purposes of adaptive min radius
    double y = -(iDasherY - iDasherOY) / double(iDasherOY); //  this normalisation works well????
    
    double theta = atan2(y, x);
    double r = sqrt(x * x + y * y);
    m_dBitrate = GetLongParameter(LP_MAX_BITRATE) / 100.0; //  stored as long(round(true bitrate * 100))

    UpdateSigmas(r);

//  Data collection:
    
    if(r > m_dMinRadius && abs(theta) < 1.25) { //FIXME - should we ignore backwards AND vertical data?
      m_nSpeedCounter++;
      m_dequeAngles.push_back(theta);
      while(m_dequeAngles.size() > m_nSpeedSamples) {
	    m_dequeAngles.pop_front();
      }
      
    }

    if(m_nSpeedCounter > m_nSpeedSamples) 
    {
      //do speed control every so often!
      
      UpdateSampleSize();
      UpdateMinRadius();
      UpdateBitrate();
      long lBitrateTimes100 =  long(round(m_dBitrate * 100)); //Dasher settings want long numerical parameters
      SetLongParameter(LP_MAX_BITRATE, lBitrateTimes100);
      m_nSpeedCounter = 0;	  
    
    }	
  
  }  
  
}

  inline double CDasherViewSquare::xmax(double x, double y) const {
    // DJCM -- define a function xmax(y) thus:
    // xmax(y) = a*[exp(b*y*y)-1] 
    // then:  if(x<xmax(y) [if the mouse is to the RIGHT of the line xmax(y)]
    // set x=xmax(y).  But set xmax=c if(xmax>c).
    // I would set a=1, b=1, c=16, to start with. 

    int a = 1, b = 1, c = 100;
    double xmax = a * (exp(b * y * y) - 1);
    //cout << "xmax = " << xmax << endl;

    if(xmax > c)
      xmax = c;

    return xmax;
  }

  inline screenint CDasherViewSquare::dasherx2screen(myint sx) const {
    double x = double (sx) / double (DasherModel()->DasherY());
    x = xmap(x);
    return CanvasX - int (x * CanvasX);

  }

  inline Cint32 CDasherViewSquare::dashery2screen(myint y1, myint y2, screenint & s1, screenint & s2) const {
//    if(GetBoolParameter(BP_KEYBOARD_MODE) == false) {
      y1 = m_ymap.map(y1);
      y2 = m_ymap.map(y2);
//  } 

if(y1 > DasherModel()->DasherY()) {
      return 0;
    } if(y2 < 0) {
      return 0;
    }

    if(y1 < 0)                  // "highest" legal coordinate to draw is 0.
      {
        y1 = 0;
      }

    // Is this square actually on the screen? Check bottom
    if(y2 > DasherModel()->DasherY())
      y2 = DasherModel()->DasherY();

    Cint32 iSize = Cint32(y2 - y1);
    DASHER_ASSERT(iSize >= 0);

    s1 = screenint(y1 * CanvasY / DasherModel()->DasherY());
    s2 = screenint(y2 * CanvasY / DasherModel()->DasherY());

    DASHER_ASSERT(s2 >= s1);
    return iSize;

  }

  inline screenint CDasherViewSquare::dashery2screen(myint y) const {
//    if(GetBoolParameter(BP_KEYBOARD_MODE) == false) {
      y = m_ymap.map(y);
  //  }

 y = (y * CanvasY / DasherModel()->DasherY());

    // Stop overflow when converting to screen coords
    if(y > myint(INT_MAX))
      return INT_MAX;
    else if(y < myint(INT_MIN))
      return INT_MIN;
    return int (y);
  }

  /// Draw the crosshair

  inline void CDasherViewSquare::Crosshair(myint sx) {
    myint x[2];
    myint y[2];

    // Vertical bar of crosshair

    x[0] = sx;
    y[0] = DasherVisibleMinY();

    x[1] = sx;
    y[1] = DasherVisibleMaxY();

    if(GetBoolParameter(BP_COLOUR_MODE) == true) {
      DasherPolyline(x, y, 2, 1, 5);
    }
    else {
      DasherPolyline(x, y, 2, 1, -1);
    }

    // Horizontal bar of crosshair

    x[0] = 12 * sx / 14;
    y[0] = DasherModel()->DasherY() / 2;

    x[1] = 17 * sx / 14;
    y[1] = DasherModel()->DasherY() / 2;

    if(GetBoolParameter(BP_COLOUR_MODE) == true) {
      DasherPolyline(x, y, 2, 1, 5);
    }
    else {
      DasherPolyline(x, y, 2, 1, -1);
    }
  }

  inline double CDasherViewSquare::ixmap(double x) const
    // invert x non-linearity
  {
//    if(GetBoolParameter(BP_KEYBOARD_MODE) == false) {
      if(x < m_dXmpb * m_dXmpc)
        return x / m_dXmpc;
      else
        return m_dXmpb - m_dXmpa + m_dXmpa * exp((x / m_dXmpc - m_dXmpb) / m_dXmpa);
//    }
 //   else {
 //     return x;
 //   }
  }

  inline double CDasherViewSquare::xmap(double x) const
    // x non-linearity
  {
//    if(GetBoolParameter(BP_KEYBOARD_MODE) == false) {
      if(x < m_dXmpb)
        return m_dXmpc * x;
      else
        return m_dXmpc * (m_dXmpa * log((x + m_dXmpa - m_dXmpb) / m_dXmpa) + m_dXmpb);
 //   }
 //   else {
 //     return x;
  //  }
  }


  inline myint CDasherViewSquare::Cymap::map(myint y) const {
    if(y > m_Y2)
      return m_Y2 + (y - m_Y2) / m_Y1;
    else if(y < m_Y3)
      return m_Y3 + (y - m_Y3) / m_Y1;
    else
      return y;
  }

  inline myint CDasherViewSquare::Cymap::unmap(myint ydash) const {
    if(ydash > m_Y2)
      return (ydash - m_Y2) * m_Y1 + m_Y2;
    else if(ydash < m_Y3)
      return (ydash - m_Y3) * m_Y1 + m_Y3;
    else
      return ydash;
  }
}
