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

  inline myint CDasherViewSquare::ixmap(myint x) const
  {
    double dx = x / static_cast<double>(GetLongParameter(LP_MAX_Y));
    if(dx < m_dXmpb * m_dXmpc)
      dx /= m_dXmpc;
    else
      dx =m_dXmpb - m_dXmpa + m_dXmpa * exp((dx / m_dXmpc - m_dXmpb) / m_dXmpa);
    return myint(dx * GetLongParameter(LP_MAX_Y));
  }

  inline myint CDasherViewSquare::xmap(myint x) const
  {
    double dx = x / static_cast<double>(GetLongParameter(LP_MAX_Y));
    if(dx < m_dXmpb)
      dx *= m_dXmpc;
    else
      dx = m_dXmpc * (m_dXmpa * log((dx + m_dXmpa - m_dXmpb) / m_dXmpa) + m_dXmpb);
    return myint(dx * GetLongParameter(LP_MAX_Y));
  }

  inline myint CDasherViewSquare::ymap(myint y) const {
    if(y > m_Y2)
      return m_Y2 + (y - m_Y2) / m_Y1;
    else if(y < m_Y3)
      return m_Y3 + (y - m_Y3) / m_Y1;
    else
      return y;
  }

  inline myint CDasherViewSquare::iymap(myint ydash) const {
    if(ydash > m_Y2)
      return (ydash - m_Y2) * m_Y1 + m_Y2;
    else if(ydash < m_Y3)
      return (ydash - m_Y3) * m_Y1 + m_Y3;
    else
      return ydash;
  }
}
