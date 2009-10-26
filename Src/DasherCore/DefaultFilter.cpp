#include "../Common/Common.h"
#include "DefaultFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include "CircleStartHandler.h"
#include "TwoBoxStartHandler.h"

#include <iostream>

using namespace Dasher;

CDefaultFilter::CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, iID, 1, szName) {
  m_pStartHandler = 0;
  m_pAutoSpeedControl = new CAutoSpeedControl(m_pEventHandler, m_pSettingsStore);

  CreateStartHandler();
}

CDefaultFilter::~CDefaultFilter() {
  delete m_pAutoSpeedControl;
}

bool CDefaultFilter::DecorateView(CDasherView *pView) {

  bool bDidSomething(false);

  if(GetBoolParameter(BP_DRAW_MOUSE)) {
    DrawMouse(pView);
    bDidSomething = true;
  }

  if(GetBoolParameter(BP_DRAW_MOUSE_LINE)) {
    DrawMouseLine(pView);
    bDidSomething = true;
  }
  
  if(m_pStartHandler)
    bDidSomething = m_pStartHandler->DecorateView(pView) || bDidSomething;

  return bDidSomething;
}

bool CDefaultFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol) {
  bool bDidSomething = false;
  if (!GetBoolParameter(BP_DASHER_PAUSED))
  {
    myint iDasherX;
    myint iDasherY;

    m_pDasherView->GetCoordinates(iDasherX, iDasherY);

    ApplyAutoCalibration(iDasherX, iDasherY, true);
    ApplyTransform(iDasherX, iDasherY);

    if(GetBoolParameter(BP_PAUSE_OUTSIDE)) {
      myint iDasherMinX;
      myint iDasherMinY;
      myint iDasherMaxX;
      myint iDasherMaxY;
      m_pDasherView->VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
  
      if((iDasherX > iDasherMaxX) || (iDasherX < iDasherMinX) || (iDasherY > iDasherMaxY) || (iDasherY < iDasherMinY))
        m_pInterface->PauseAt(0,0);
		return false;
    }

    m_pDasherModel->OneStepTowards(iDasherX,iDasherY, Time, pAdded, pNumDeleted);
    bDidSomething = true;

    m_pAutoSpeedControl->SpeedControl(iDasherX, iDasherY, m_pDasherModel->Framerate(), m_pDasherView);
  }
	
  if(m_pStartHandler)
    m_pStartHandler->Timer(Time, m_pDasherView, m_pDasherModel);

  return bDidSomething;
}

void CDefaultFilter::KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog) {

  switch(iId) {
  case 0: // Start on space
    // FIXME - wrap this in a 'start/stop' method (and use for buttons as well as keys)
    if(GetBoolParameter(BP_START_SPACE)) {
      if(GetBoolParameter(BP_DASHER_PAUSED))
	m_pInterface->Unpause(iTime);
      else
	m_pInterface->PauseAt(0, 0);
    }
    break; 
  case 100: // Start on mouse
    if(GetBoolParameter(BP_START_MOUSE)) {
      if(GetBoolParameter(BP_DASHER_PAUSED))
	m_pInterface->Unpause(iTime);
      else
	m_pInterface->PauseAt(0, 0);
    }
    break;
  default:
    break;
  }
}

void CDefaultFilter::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
   
    switch (pEvt->m_iParameter) {
    case BP_CIRCLE_START:
    case BP_MOUSEPOS_MODE:
      CreateStartHandler();
      break;
    }
  }
}

void CDefaultFilter::CreateStartHandler() {
  if(m_pStartHandler) {
    delete m_pStartHandler;
    m_pStartHandler = 0;
  }

  if(GetBoolParameter(BP_CIRCLE_START))
    m_pStartHandler = new CCircleStartHandler(m_pEventHandler, m_pSettingsStore, m_pInterface);
  else if(GetBoolParameter(BP_MOUSEPOS_MODE))
    m_pStartHandler = new CTwoBoxStartHandler(m_pEventHandler, m_pSettingsStore, m_pInterface);

}

void CDefaultFilter::DrawMouse(CDasherView *pView) {
  myint iDasherX;
  myint iDasherY;

  pView->GetCoordinates(iDasherX, iDasherY);

  ApplyAutoCalibration(iDasherX, iDasherY, false);
  ApplyTransform(iDasherX, iDasherY);

  pView->DasherDrawCentredRectangle(iDasherX, iDasherY, 5, 2, Opts::ColorSchemes(Opts::Objects), false);
}

void CDefaultFilter::DrawMouseLine(CDasherView *pView) {
  myint x[2];
  myint y[2];

  // Start of line is the crosshair location

  x[0] = (myint)GetLongParameter(LP_OX);
  y[0] = (myint)GetLongParameter(LP_OY);

  //  myint iDasherX;
  //myint iDasherY;

  pView->GetCoordinates(x[1], y[1]);

  ApplyAutoCalibration(x[1], y[1], false);
  ApplyTransform(x[1], y[1]);

  // Actually plot the line

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

}

void CDefaultFilter::ApplyTransform(myint &iDasherX, myint &iDasherY) {
}

void CDefaultFilter::ApplyAutoCalibration(myint &iDasherX, myint &iDasherY, bool bUpdate) {
}
