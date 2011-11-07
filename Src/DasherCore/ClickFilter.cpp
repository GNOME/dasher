#include "../Common/Common.h"
#include "ClickFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_MAXZOOM, T_LONG, 11, 400, 10, 1, _("Maximum Zoom")},

/* TRANSLATORS: In click mode, when you click with the mouse, you select
   a piece of y-axis to be zoomed to, based on the mouse coordinates. The
   "guides" are lines from the mouse position to the edges of the piece of
   y-axis. */
  {BP_DRAW_MOUSE_LINE, T_BOOL, -1, -1, -1, -1, _("Draw guides on screen to show area into which a click will zoom")},
/* TRANSLATORS: As dasher's on-screen coordinate space is not flat,
   the straight guide lines should in fact really be curved. This option
   draws the guides as correct, though possibly more confusing, curves. */
  {BP_CURVE_MOUSE_LINE, T_BOOL, -1, -1, -1, -1, _("Curve lines to follow the non-linearity of the view transform")},
};

bool CClickFilter::DecorateView(CDasherView *pView, CDasherInput *pInput) {
  bool bChanged(false);
  if (GetBoolParameter(BP_DRAW_MOUSE_LINE)) {
    myint mouseX, mouseY;
    pInput->GetDasherCoords(mouseX, mouseY, pView);
    AdjustZoomX(mouseX, pView, GetLongParameter(LP_S), GetLongParameter(LP_MAXZOOM));
    if (m_iLastX != mouseX || m_iLastY != mouseY) {
      bChanged = true;
      m_iLastX = mouseX; m_iLastY = mouseY;
    }
    myint x[3], y[3];
    x[0] = x[2] = 0;
    x[1] = mouseX;
    y[0] = mouseY - mouseX;
    y[1] = mouseY;
    y[2] = mouseY + mouseX;
    if (GetBoolParameter(BP_CURVE_MOUSE_LINE)) {
      pView->DasherSpaceLine(x[0], y[0], x[1], y[1], GetLongParameter(LP_LINE_WIDTH), 1);
      pView->DasherSpaceLine(x[1], y[1], x[2], y[2], GetLongParameter(LP_LINE_WIDTH), 1);
    } else {
      //Note that the nonlinearity at edges of screen causes the lines to wobble close to the top/bottom:
      // we draw lines _straight_ towards their targets on the Y-axis (calculated after applying nonlinearity),
      // but truncated to the visible portion of the screen.
      //A quick/approximate (but wrong!) solution, to make the lines stay at much the same angle, is
      // to draw them to intersect min/max visible y only, as follows:
        //if (y[0] < iDasherMinY) {
        //  x[0] = mouseX - (mouseY - iDasherMinY);
        //  y[0] = iDasherMinY;
        //}
        //if (y[2] > iDasherMaxY) {
        //  x[2] = mouseX - (iDasherMaxY - mouseY);
        //  y[2] = iDasherMaxY;
        //}
      pView->DasherPolyline(x, y, 3, GetLongParameter(LP_LINE_WIDTH), 1);
    }
  }
  return bChanged;
}

void CZoomAdjuster::AdjustZoomX(myint &iDasherX, CDasherView *pView, myint safety, myint maxZoom) {
  //these equations don't work well for iDasherX just slightly over ORIGIN_X;
  // this is probably due to rounding error, but the "safety margin" doesn't
  // really seem helpful when zooming out (or translating) anyway...
  if (iDasherX >= CDasherModel::ORIGIN_X) return;

  //safety param. Used to be just added onto DasherX,
  // but comments suggested should be interpreted as a fraction. Hence...
  myint iNewDasherX = (iDasherX*1024 + CDasherModel::ORIGIN_X*safety) / (1024+safety);

  //max zoom parameter...also force x>=2 (what's wrong with x==1?)
  iDasherX = std::max(std::max(myint(2),CDasherModel::ORIGIN_X/maxZoom),iNewDasherX);  
}

void CClickFilter::KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  switch(iId) {
  case 100: // Mouse clicks
    {
      myint iDasherX;
      myint iDasherY;

      pInput->GetDasherCoords(iDasherX, iDasherY, pView);
      AdjustZoomX(iDasherX, pView, GetLongParameter(LP_S), GetLongParameter(LP_MAXZOOM));
      ScheduleZoom(pModel, iDasherY-iDasherX, iDasherY+iDasherX);
    }
    break;
  default:
    break;
  }
}

bool CClickFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
}
