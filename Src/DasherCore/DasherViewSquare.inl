// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherModel.h"

namespace Dasher {

  inline screenint CDasherViewSquare::dasherx2screen(myint sx) const {
    double x = double (sx) / double (GetLongParameter(LP_MAX_Y));
    x = xmap(x);
    return CanvasX - int (x * CanvasX);

  }

  inline Cint32 CDasherViewSquare::dashery2screen(myint y1, myint y2, screenint & s1, screenint & s2) const {
//    if(GetBoolParameter(BP_KEYBOARD_MODE) == false) {
      y1 = m_ymap.map(y1);
      y2 = m_ymap.map(y2);
//  } 

if(y1 > (myint)GetLongParameter(LP_MAX_Y)) {
      return 0;
    } if(y2 < 0) {
      return 0;
    }

    if(y1 < 0)                  // "highest" legal coordinate to draw is 0.
      {
        y1 = 0;
      }

    // Is this square actually on the screen? Check bottom
    if(y2 > (myint)GetLongParameter(LP_MAX_Y))
      y2 = GetLongParameter(LP_MAX_Y);

    Cint32 iSize = Cint32(y2 - y1);
    DASHER_ASSERT(iSize >= 0);

    s1 = screenint(y1 * CanvasY / (myint)GetLongParameter(LP_MAX_Y));
    s2 = screenint(y2 * CanvasY / (myint)GetLongParameter(LP_MAX_Y));

    DASHER_ASSERT(s2 >= s1);
    return iSize;

  }

  inline screenint CDasherViewSquare::dashery2screen(myint y) const {
//    if(GetBoolParameter(BP_KEYBOARD_MODE) == false) {
      y = m_ymap.map(y);
  //  }

 y = (y * CanvasY / (myint)GetLongParameter(LP_MAX_Y));

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

    DasherPolyline(x, y, 2, 1, 5);

    // Horizontal bar of crosshair

    x[0] = 12 * sx / 14;
    y[0] = GetLongParameter(LP_MAX_Y) / 2;

    x[1] = 17 * sx / 14;
    y[1] = GetLongParameter(LP_MAX_Y) / 2;

    DasherPolyline(x, y, 2, 1, 5);
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
