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

bool CClickFilter::DecorateView(CDasherView *pView) {
  bool bChanged(false);
  if (GetBoolParameter(BP_DRAW_MOUSE_LINE)) {
    myint mouseX, mouseY;
    pView->GetCoordinates(mouseX, mouseY);
    //unfortunately we have to copy the limit set by DasherModel::ScheduleZoom here
    // ....call for a refactor? but of some/what sort?
    if (mouseX<2) mouseX=2;
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

bool CClickFilter::Timer(int Time, CDasherView *pDasherView, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol) {
  return pModel->NextScheduledStep(Time, pAdded, pNumDeleted);
}

void CClickFilter::KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY) {
  switch(iId) {
  case 100: // Mouse clicks
    if(pDasherView) {
      myint iDasherX;
      myint iDasherY;

      pDasherView->GetCoordinates(iDasherX, iDasherY);

      pModel->ScheduleZoom(iTime, iDasherX,iDasherY, GetLongParameter(LP_MAXZOOM));
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
