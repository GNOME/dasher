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
    x -= iMarginWidth;
    if (GetLongParameter(LP_NONLINEAR_X)>0 && x >= m_iXlogThres) {
      double dx = (x - m_iXlogThres) / static_cast<double>(GetLongParameter(LP_MAX_Y));
      dx =  (exp(dx * m_dXlogCoeff) - 1) / m_dXlogCoeff;
      x = myint( dx * GetLongParameter(LP_MAX_Y)) + m_iXlogThres;
    }
    return x;
  }

  inline myint CDasherViewSquare::xmap(myint x) const
  {
    if(GetLongParameter(LP_NONLINEAR_X) && x >= m_iXlogThres) {
      double dx = log(1+ (x-m_iXlogThres)*m_dXlogCoeff/GetLongParameter(LP_MAX_Y))/m_dXlogCoeff;
      dx = (dx*GetLongParameter(LP_MAX_Y)) + m_iXlogThres;
      x= myint(dx>0 ? ceil(dx) : floor(dx));
    }
    return x + iMarginWidth;
  }

  inline myint CDasherViewSquare::ymap(myint y) const {
    if (GetBoolParameter(BP_NONLINEAR_Y)) {
      if(y > m_Y2)
        return m_Y2 + (y - m_Y2) / m_Y1;
      else if(y < m_Y3)
        return m_Y3 + (y - m_Y3) / m_Y1;
    }
    return y;
  }

  inline myint CDasherViewSquare::iymap(myint ydash) const {
    if (GetBoolParameter(BP_NONLINEAR_Y)) {
      if(ydash > m_Y2)
        return (ydash - m_Y2) * m_Y1 + m_Y2;
      else if(ydash < m_Y3)
        return (ydash - m_Y3) * m_Y1 + m_Y3;
    }
    return ydash;
  }
}
