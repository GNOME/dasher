// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherModel.h"

namespace Dasher {

  /// Draw the crosshair

  inline void CDasherViewSquare::Crosshair(myint sx) {
    myint iDasherMinX;
    myint iDasherMinY;
    myint iDasherMaxX;
    myint iDasherMaxY;
    VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

    myint x[2];
    myint y[2];

    // Vertical bar of crosshair

    x[0] = sx;
    y[0] = iDasherMinY;

    x[1] = sx;
    y[1] = iDasherMaxY;

    DasherPolyline(x, y, 2, 1, 5);

    // Horizontal bar of crosshair

    x[0] = 12 * sx / 14;
    y[0] = GetLongParameter(LP_MAX_Y) / 2;

    x[1] = 17 * sx / 14;
    y[1] = GetLongParameter(LP_MAX_Y) / 2;

    DasherPolyline(x, y, 2, 1, 5);
  }

  inline double CDasherViewSquare::ixmap(double x) const
  {
      if(x < m_dXmpb * m_dXmpc)
        return x / m_dXmpc;
      else
        return m_dXmpb - m_dXmpa + m_dXmpa * exp((x / m_dXmpc - m_dXmpb) / m_dXmpa);
  }

  inline double CDasherViewSquare::xmap(double x) const
  {
      if(x < m_dXmpb)
        return m_dXmpc * x;
      else
        return m_dXmpc * (m_dXmpa * log((x + m_dXmpa - m_dXmpb) / m_dXmpa) + m_dXmpb);
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
