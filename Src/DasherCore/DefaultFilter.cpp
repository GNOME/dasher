#include "../Common/Common.h"
#include "DefaultFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include "CircleStartHandler.h"
#include "TwoBoxStartHandler.h"

#include <iostream>

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_TARGET_OFFSET, T_LONG, -100, 100, 400, 1, _("Vertical distance from mouse/gaze to target (400=screen height)")},
  {BP_AUTOCALIBRATE, T_BOOL, -1, -1, -1, -1, _("Learn offset (previous) automatically, e.g. gazetrackers")},
  {BP_REMAP_XTREME, T_BOOL, -1, -1, -1, -1, _("At top and bottom, scroll more and translate less (makes error-correcting easier)")},
  {LP_GEOMETRY, T_LONG, 0, 3, 1, 1, _("Screen geometry (mostly for tall thin screens) - 0=old-style, 1=square no-xhair, 2=squish, 3=squish+log")},
  {LP_SHAPE_TYPE, T_LONG, 0, 5, 1, 1, _("Shape type: 0=disjoint rects, 1=overlapping, 2=triangles, 3=trunc-tris, 4=quadrics, 5=circles")},
  {LP_X_LIMIT_SPEED, T_LONG, 1, 800, 1536, 1, _("Distance from right-hand-side Y-axis, at which maximum speed is reached. (2048=xhair)")},
  {BP_TURBO_MODE, T_BOOL, -1, -1, -1, -1, _("Hold right mouse button / key 1 to go 3/4 faster")},
  {BP_EXACT_DYNAMICS, T_BOOL, -1, -1, -1, -1, _("Use exact computation of per-frame movement (slower)")},
};

bool CDefaultFilter::GetSettings(SModuleSettings **sets, int *iCount) {
  *sets = sSettings;
  *iCount = sizeof(sSettings) / sizeof(sSettings[0]);
  return true;
}

CDefaultFilter::CDefaultFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName)
  : CDynamicFilter(pCreator, pInterface, pFramerate, iID, szName), CSettingsObserver(pCreator), m_bTurbo(false) {
  m_pStartHandler = 0;
  m_pAutoSpeedControl = new CAutoSpeedControl(this);

  // Initialize autocalibration (i.e. seen nothing yet)
  m_iSum = 0;
  m_iCounter = 0;
  if (GetBoolParameter(BP_AUTOCALIBRATE)) //eyetracker calibration has likely changed from previous session
    SetLongParameter(LP_TARGET_OFFSET, 0); //so start over from scratch
}

CDefaultFilter::~CDefaultFilter() {
  delete m_pAutoSpeedControl;
  delete m_pStartHandler;
}

bool CDefaultFilter::DecorateView(CDasherView *pView, CDasherInput *pInput) {

  bool bDidSomething(false);

  if(GetBoolParameter(BP_DRAW_MOUSE)) {
    //Draw a small box at the current mouse position
    pView->DasherDrawCentredRectangle(m_iLastX, m_iLastY, 5, 2, false);

    bDidSomething = true;
  }

  if(GetBoolParameter(BP_DRAW_MOUSE_LINE)) {
    // Draw a line from the origin to the current mouse position
    myint x[2];
    myint y[2];

    // Start of line is the crosshair location
    x[0] = CDasherModel::ORIGIN_X;
    y[0] = CDasherModel::ORIGIN_Y;

    //If the user's finger/mouse is in the margin, draw the line to the closest
    // point we'll actually head to.
    x[1] = max(myint(1),m_iLastX);
    y[1] = m_iLastY;

    // Actually plot the line
    if (GetBoolParameter(BP_CURVE_MOUSE_LINE))
      pView->DasherSpaceLine(x[0],y[0],x[1],y[1], GetLongParameter(LP_LINE_WIDTH), 1);
    else
      pView->DasherPolyline(x, y, 2, GetLongParameter(LP_LINE_WIDTH), 1);

  /*  // Plot a brachistochrone

  const int noOfPoints = 18;
  myint X[noOfPoints];
  myint Y[noOfPoints];
  myint CenterXY[2];
  X[0] = x[0];
  Y[0] = y[0];
  X[noOfPoints-1] = 0;
  Y[noOfPoints-1] = y[1];
  CenterXY[0] = 0; CenterXY[1] = 0.5*((double)(X[0]*X[0])/(double)(Y[0]-Y[noOfPoints-1])+(Y[0]+Y[noOfPoints-1]));

  double angle = (((Y[noOfPoints-1]>CenterXY[1])?1.5708:-1.5708) - atan((double)(Y[0]-CenterXY[1])/(double)X[0]))/(double)(noOfPoints-1);
  for(int i = 1; i < noOfPoints-1; ++i)
    {
      X[i] = CenterXY[0] + cos(angle)*(X[i-1]-CenterXY[0]) - sin(angle)*(Y[i-1]-CenterXY[1]);
      Y[i] = CenterXY[1] + sin(angle)*(X[i-1]-CenterXY[0]) + cos(angle)*(Y[i-1]-CenterXY[1]);
    }

    pView->DasherPolyline(X, Y, noOfPoints, GetLongParameter(LP_LINE_WIDTH), 2);*/
  /*  std::cout << "(" << X[0] << "," << Y[0] << ") (" << X[noOfPoints-1] << "," << Y[noOfPoints-1] << ") "
	    << "(" << CenterXY[0] << "," << CenterXY[1]
	    << ") angle:" << angle << "," << angle*180.0/3.1415926 << std::endl;*/

    bDidSomething = true;
  }
  //only require another frame if we actually drew a mouse line in a different place to before...
  bDidSomething = m_bGotMouseCoords & bDidSomething;
  m_bGotMouseCoords = false;
  if(m_pStartHandler)
    bDidSomething = m_pStartHandler->DecorateView(pView) || bDidSomething;

  return bDidSomething;
}

void CDefaultFilter::Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol) {
  if (!(m_bGotMouseCoords = pInput->GetDasherCoords(m_iLastX, m_iLastY, pView))) {
    stop();
    return;
  };
  //Got coordinates
  ApplyTransform(m_iLastX, m_iLastY, pView);
  if (!isPaused())
  {
    if(GetBoolParameter(BP_STOP_OUTSIDE)) {
      myint iDasherMinX;
      myint iDasherMinY;
      myint iDasherMaxX;
      myint iDasherMaxY;
      pView->VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

      if((m_iLastX > iDasherMaxX) || (m_iLastX < iDasherMinX) || (m_iLastY > iDasherMaxY) || (m_iLastY < iDasherMinY)) {
        stop();
        return;
      }
    }

    double dSpeedMul(FrameSpeedMul(m_pDasherModel, Time));
    if (m_bTurbo) dSpeedMul*=1.75;
    
    OneStepTowards(m_pDasherModel, m_iLastX,m_iLastY, Time, dSpeedMul);

    if (dSpeedMul==1.0)
      m_pAutoSpeedControl->SpeedControl(m_iLastX, m_iLastY, pView);
  }

  if(m_pStartHandler)
    m_pStartHandler->Timer(Time, m_iLastX, m_iLastY, pView);
}

void CDefaultFilter::run(unsigned long iTime) {
  CDynamicFilter::run(iTime);
  if (m_pStartHandler) m_pStartHandler->onRun(iTime);
}

void CDefaultFilter::pause() {
  CDynamicFilter::pause();
  if (m_pStartHandler) m_pStartHandler->onPause();
}

void CDefaultFilter::KeyDown(unsigned long iTime, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {

  if ((iId==0 && GetBoolParameter(BP_START_SPACE))
      || (iId==100 && GetBoolParameter(BP_START_MOUSE))) {
    if(isPaused())
      run(iTime);
    else
      stop();
  }
  else if (iId==101 || iId==102 || iId==1) {
    //Other mouse buttons, if platforms support; or button 1
    if (GetBoolParameter(BP_TURBO_MODE))
      m_bTurbo = true;
  }
}

void CDefaultFilter::KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if (iId==101 || iId==102 || iId==1)
    m_bTurbo=false;
}

void CDefaultFilter::stop() {
  if (isPaused()) return;
  pause();
  m_pInterface->Done();
}

void CDefaultFilter::HandleEvent(int iParameter) {
  switch (iParameter) {
  case BP_CIRCLE_START:
  case BP_MOUSEPOS_MODE:
    CreateStartHandler();
    break;
  case BP_TURBO_MODE:
    m_bTurbo &= GetBoolParameter(BP_TURBO_MODE);
  }
}

void CDefaultFilter::CreateStartHandler() {
  delete m_pStartHandler;
  m_pStartHandler = MakeStartHandler();
}

void CDefaultFilter::Activate() {
  CreateStartHandler();
  CInputFilter::Activate();
}

void CDefaultFilter::Deactivate() {
  delete m_pStartHandler;
  m_pStartHandler = 0;
}

CStartHandler *CDefaultFilter::MakeStartHandler() {
  if(GetBoolParameter(BP_CIRCLE_START))
    return new CCircleStartHandler(this);
  if(GetBoolParameter(BP_MOUSEPOS_MODE))
    return new CTwoBoxStartHandler(this);
  return NULL;
}

double xmax(double y) {
  // DJCM -- define a function xmax(y) thus:
  // xmax(y) = a*[exp(b*y*y)-1]
  // then:  if(x<xmax(y) [if the mouse is to the RIGHT of the line xmax(y)]
  // set x=xmax(y).  But set xmax=c if(xmax>c).
  // I would set a=1, b=1, c=16, to start with.

  static const int a = 1, b = 1;
  static const double c = 100;
  return min(c,a * (exp(b * y * y) - 1));
  //cout << "xmax = " << xmax << endl;
}

void CDefaultFilter::ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
  ApplyOffset(iDasherX, iDasherY);
  if (GetLongParameter(LP_GEOMETRY)==1) {
    //crosshair may be offscreen; so do something to allow us to navigate
    // up/down and reverse
    myint iDasherMaxX,temp;
    pView->VisibleRegion(temp, temp, iDasherMaxX, temp);
    const myint xd(iDasherX - iDasherMaxX),yd(iDasherY-CDasherModel::ORIGIN_Y);
    const myint dist(xd*xd + yd*yd); //squared distance from closest point onscreen to crosshair
    if (iDasherMaxX < CDasherModel::ORIGIN_X) {
      //crosshair actually offscreen; rescale so left edge of screen = translate
      iDasherX = (iDasherX * CDasherModel::ORIGIN_X)/iDasherMaxX;
    }
    //boost reversing if near centerpoint of LHS (even if xhair onscreen)
    iDasherX += (2*CDasherModel::ORIGIN_Y*CDasherModel::ORIGIN_Y)/(dist+50); //and close to centerpoint = reverse
  }
  if (GetBoolParameter(BP_REMAP_XTREME)) {
    // Y co-ordinate...
    myint dasherOY=CDasherModel::ORIGIN_Y;
    double double_y = ((iDasherY-dasherOY)/(double)(dasherOY) ); // Fraction above the crosshair
    static const double repulsionparameter=0.5;
    iDasherY = myint(dasherOY * (1.0 + double_y + (double_y*double_y*double_y * repulsionparameter )));

    // X co-ordinate...
    iDasherX = max(iDasherX,myint(CDasherModel::ORIGIN_X * xmax(double_y)));
  }
}

void CDefaultFilter::ApplyOffset(myint &iDasherX, myint &iDasherY) {

  // TODO: It turns out that this was previously computed in pixels,
  // altough everythign else made use of Dasher coordinates. Hack in a
  // factor of 10 to get the offset in Dasher coordinates, but it
  // would be a good idea at some point to sort this out properly.

  iDasherY += 10 * GetLongParameter(LP_TARGET_OFFSET);

  if(GetBoolParameter(BP_AUTOCALIBRATE) && !isPaused()) {
    // Auto-update the offset

    m_iSum += CDasherModel::ORIGIN_Y - iDasherY; // Distance above crosshair
    ++m_iCounter;

    //int m_iFilterTimescale=20
    if(++m_iCounter > 20) {
      m_iCounter = 0;

      // 'Conditions A', as specified by DJCM.  Only make the auto-offset
      // change if we're past the significance boundary.
      //int m_iSigBiasPixels(CDasherModel::MAX_Y/2);

      if (((m_iSum>0)?m_iSum:-m_iSum) > CDasherModel::MAX_Y/2)
        SetLongParameter(LP_TARGET_OFFSET, GetLongParameter(LP_TARGET_OFFSET) + ((m_iSum>0) ? -1 : 1));
      //TODO, "else return" - check effectiveness with/without?
      // old code exited now if neither above cases applied,
      // but had TODO suggesting maybe we should _always_ reset m_iSum
      // (as we now do here)...
      m_iSum = 0;
    }
  }
}
