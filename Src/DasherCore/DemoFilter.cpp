#include "../Common/Common.h"
#include "DemoFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include "CircleStartHandler.h"
#include "TwoBoxStartHandler.h"
#include "GameModule.h"

#include <iostream>

using namespace Dasher;

CDemoFilter::CDemoFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate)
  : CDynamicFilter(pCreator, pInterface, pFramerate, 19, _("Demo Mode (no input)")),
m_dNoiseX(0.0), m_dNoiseY(0.0), m_iDemoX(0), m_iDemoY(0) {

}

CDemoFilter::~CDemoFilter() {
}

bool CDemoFilter::DecorateView(CDasherView *pView, CDasherInput *pInput) {

  if(GetBoolParameter(BP_DRAW_MOUSE)) {
    pView->DasherDrawCentredRectangle(m_iDemoX, m_iDemoY, 5, 2, false);
  }

  myint x[2];
  myint y[2];
  
  // Start of line is the crosshair location
  
  x[0] = CDasherModel::ORIGIN_X;
  y[0] = CDasherModel::ORIGIN_Y;
  
  x[1] = m_iDemoX; y[1] = m_iDemoY;
  
  // Actually plot the line
  if (GetBoolParameter(BP_CURVE_MOUSE_LINE))
    pView->DasherSpaceLine(x[0],y[0],x[1],y[1], GetLongParameter(LP_LINE_WIDTH), 1);
  else
    pView->DasherPolyline(x, y, 2, GetLongParameter(LP_LINE_WIDTH), 1);
  
  return true;
}

void CDemoFilter::Activate() {
  m_pInterface->EnterGameMode(NULL);
  HandleEvent(LP_FRAMERATE); //just to make sure!
}

void CDemoFilter::Deactivate() {
  m_pInterface->LeaveGameMode();
}

std::pair<double,double> GaussianRand() // Is there a random number class already?
{
  double u1 = (double(rand())+1)/(double(RAND_MAX)+1.0);
  double u2 = (double(rand())+1)/(double(RAND_MAX)+1.0);
  double g1 = sqrt(-2.0*log(u1))*cos(6.283185307*u2);
  double g2 = sqrt(-2.0*log(u1))*sin(6.283185307*u2);
  //  std::cout << u1 << " : " << u2 << " : " << g1 << std::endl;
  return std::pair<double,double>(g1,g2);
}

void CDemoFilter::Timer(unsigned long Time, CDasherView *m_pDasherView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol) {
  if (isPaused()) return;
  CGameModule *mod = (CGameModule *)m_pInterface->GetGameModule();
  const myint iTargetY(mod->m_iTargetY);
  myint iCenterY = mod->ComputeBrachCenter();

  // ...and now calculate the ideal direction...
  double iIdealUnitVec[2];  
  
  iIdealUnitVec[0] = double(CDasherModel::ORIGIN_Y<iTargetY?(iCenterY-CDasherModel::ORIGIN_Y):(CDasherModel::ORIGIN_Y-iCenterY));
  iIdealUnitVec[1] = double(CDasherModel::ORIGIN_Y<iTargetY ? CDasherModel::ORIGIN_X : -CDasherModel::ORIGIN_X);
  double mag = sqrt((double)(iIdealUnitVec[0]*iIdealUnitVec[0]+iIdealUnitVec[1]*iIdealUnitVec[1]));
  iIdealUnitVec[0] = iIdealUnitVec[0]/mag;
  iIdealUnitVec[1] = iIdealUnitVec[1]/mag;
  
  // ...and then modify for realism... 
  // ...by adding noise...
  std::pair<double,double> noise = GaussianRand();
  m_dNoiseX = m_dNoiseOld*m_dNoiseX + m_dNoiseNew*noise.first;
  m_dNoiseY = m_dNoiseOld*m_dNoiseY + m_dNoiseNew*noise.second;
  
  // ...and springy behaviour...
  //if(!m_bSentenceFinished) {
    const myint iNoiseMag(GetLongParameter(LP_DEMO_NOISE_MAG));
    m_iDemoX = myint((CDasherModel::ORIGIN_X+(1500*iIdealUnitVec[0])+iNoiseMag*m_dNoiseX)*m_dSpring
                     +(1.0-m_dSpring)*m_iDemoX);
    m_iDemoY = myint((CDasherModel::ORIGIN_Y+(1500*iIdealUnitVec[1])+iNoiseMag*m_dNoiseY)*m_dSpring
                     +(1.0-m_dSpring)*m_iDemoY);
  //} else {
  //  m_iDemoX = myint(ORIGIN_X*m_DemoCfg.dSpring + (1.0-m_DemoCfg.dSpring)*m_iDemoX);
  //  m_iDemoY = myint(ORIGIN_Y*m_DemoCfg.dSpring + (1.0-m_DemoCfg.dSpring)*m_iDemoY);
  //}
  
  // ...and finally set the mouse coordinates.

  OneStepTowards(m_pDasherModel, m_iDemoX, m_iDemoY, Time, 1.0);
}

void CDemoFilter::KeyDown(unsigned long iTime, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {
  
  if ((iId==0 && GetBoolParameter(BP_START_SPACE))
      || (iId==100 && GetBoolParameter(BP_START_MOUSE))) {
    if(isPaused())
      run(iTime);
    else
      pause();
  }
}

void CDemoFilter::HandleEvent(int iParameter) {
    switch (iParameter) {
      case LP_DEMO_SPRING:
      case LP_DEMO_NOISE_MEM:
      case LP_MAX_BITRATE:
      case LP_FRAMERATE:
        // Recalculates the parameters used in the demo following a change in framerate or speed.
        double spring = GetLongParameter(LP_DEMO_SPRING)/100.0;
        double noisemem = GetLongParameter(LP_DEMO_NOISE_MEM)/100.0;
        double lambda = 0.7*GetLongParameter(LP_MAX_BITRATE)/(double)GetLongParameter(LP_FRAMERATE);
        
        m_dSpring = (1-exp(-spring*lambda));
        m_dNoiseNew = noisemem*(1-exp(-lambda));
        m_dNoiseOld = sqrt(1.0-m_dNoiseNew*m_dNoiseNew);
    }
}
