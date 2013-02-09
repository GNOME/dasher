#include "../Common/Common.h"
#include "OneDimensionalFilter.h"
#include "CircleStartHandler.h"

using namespace Dasher;

/*COneDimensionalFilter::COneDimensionalFilter(CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel)
  : COneDimensionalFilter(pSettingsStore, pInterface, m_pDasherModel, 4, _("One Dimensional Mode")) {
}*/

COneDimensionalFilter::COneDimensionalFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pCreator, pInterface, pFramerate, iID, szName), forwardmax(CDasherModel::MAX_Y/2.5) {
}

void COneDimensionalFilter::ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView) {

  myint iDasherMaxX,temp;
  pView->VisibleRegion(temp, temp, iDasherMaxX, temp);

  double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb,x;	
  
  // The distance between the Y coordinate and the centreline in pixels
  disty=CDasherModel::ORIGIN_Y-iDasherY;
  
  circlesize=    forwardmax*(1.0-max(0.0,min(1.0,(double)iDasherX/iDasherMaxX)));
  yforwardrange= CDasherModel::MAX_Y/3.2; // Was 1.6
  yfullrange=    yforwardrange*1.6;
  ybackrange=    yfullrange-yforwardrange;
  ellipse_eccentricity=6;
  
  if (disty>yforwardrange) {
    // If the distance between y-coord and centreline is > radius,
    // we should be going backwards, off the top.
    yb=(disty-yforwardrange)/ybackrange;
    
    if (yb>1) {
      x=0;
      iDasherY=CDasherModel::ORIGIN_Y;
    }
    else { 
      angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
      
      x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
      iDasherY=myint(-(1+cos(angle))*circlesize/2+CDasherModel::ORIGIN_Y);
    }
  }
  else if (disty <-(yforwardrange)) {
    // Backwards, off the bottom.
    yb=-(disty+yforwardrange)/ybackrange;
    
    if (yb>1) {
      x=0;
      iDasherY=CDasherModel::ORIGIN_Y;
    }   
    else {
      angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
      
      x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
      iDasherY=myint((1+cos(angle))*circlesize/2+CDasherModel::ORIGIN_Y);
    }   
  }
  
  else {
    angle=((disty*3.14159/2)/yforwardrange);
    x=cos(angle)*circlesize;
    iDasherY=myint(-sin(angle)*circlesize+CDasherModel::ORIGIN_Y);
  }
  x=CDasherModel::ORIGIN_X-x;
  
  iDasherX = myint(x);
}

bool COneDimensionalFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  return false;
}

CStartHandler *COneDimensionalFilter::MakeStartHandler() {
  if (GetBoolParameter(BP_CIRCLE_START)) {
    class C1DCircleStartHandler : public CCircleStartHandler {
    public:
      C1DCircleStartHandler(COneDimensionalFilter *f) : CCircleStartHandler(f) {
      }
      CDasherScreen::point CircleCenter(CDasherView *pView) {
        if (m_iScreenRadius==-1) {//if we need to recompute
          CCircleStartHandler::CircleCenter(pView); //that does the radius
          const myint rad(static_cast<myint>(GetLongParameter(LP_CIRCLE_PERCENT)) * CDasherModel::ORIGIN_Y / 100); //~~rad/2 in dasher-coords
          m_pView->Dasher2Screen(CDasherModel::ORIGIN_X-static_cast<COneDimensionalFilter*>(m_pFilter)->forwardmax+rad, CDasherModel::ORIGIN_Y,m_fwdCenter.x, m_fwdCenter.y);
        }
        if (!static_cast<COneDimensionalFilter*>(m_pFilter)->isPaused()) return CCircleStartHandler::CircleCenter(pView);
        //paused. put start circle at center of 1D transform, rather than center of screen
        // but keep the same m_iScreenRadius, in pixels - so recompute if necessary:
        return m_fwdCenter;
      }
      void onPause() {
        //circle needs to move for pause/unpause; setting radius to -1 causes
        // next call to DecorateView or Timer to re-call ComputeScreenLoc.
        m_iScreenRadius=-1;
        CCircleStartHandler::onPause();
      }
    private:
      CDasherScreen::point m_fwdCenter;
    };
    return new C1DCircleStartHandler(this);
  }
  return CDefaultFilter::MakeStartHandler();
}
