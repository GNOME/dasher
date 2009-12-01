#include "../Common/Common.h"
#include "OneDimensionalFilter.h"

using namespace Dasher;

/*COneDimensionalFilter::COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel)
  : COneDimensionalFilter(pEventHandler, pSettingsStore, pInterface, m_pDasherModel, 4, _("One Dimensional Mode")) {
}*/

COneDimensionalFilter::COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, iID, szName) {
}


void COneDimensionalFilter::ApplyTransform(myint &iDasherX, myint &iDasherY) {

  double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb,x;	
  
  // The distance between the Y coordinate and the centreline in pixels
  disty=(myint)GetLongParameter(LP_OY)-iDasherY;
  
  circlesize=    (myint)GetLongParameter(LP_MAX_Y)/2.5;
  yforwardrange= (myint)GetLongParameter(LP_MAX_Y)/3.2; // Was 1.6
  yfullrange=    yforwardrange*1.6;
  ybackrange=    yfullrange-yforwardrange;
  ellipse_eccentricity=6;
  
  if (disty>yforwardrange) {
    // If the distance between y-coord and centreline is > radius,
    // we should be going backwards, off the top.
    yb=(disty-yforwardrange)/ybackrange;
    
    if (yb>1) {
      x=0;
      iDasherY=myint(GetLongParameter(LP_OY));
    }
    else { 
      angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
      
      x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
      iDasherY=myint(-(1+cos(angle))*circlesize/2+(myint)GetLongParameter(LP_OY));
    }
  }
  else if (disty <-(yforwardrange)) {
    // Backwards, off the bottom.
    yb=-(disty+yforwardrange)/ybackrange;
    
    if (yb>1) {
      x=0;
      iDasherY=myint((myint)GetLongParameter(LP_OY));
    }   
    else {
      angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
      
      x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
      iDasherY=myint((1+cos(angle))*circlesize/2+(myint)GetLongParameter(LP_OY));
    }   
  }
  
  else {
    angle=((disty*3.14159/2)/yforwardrange);
    x=cos(angle)*circlesize;
    iDasherY=myint(-sin(angle)*circlesize+(myint)GetLongParameter(LP_OY));
  }
  x=(myint)GetLongParameter(LP_OX)-x;
  
  iDasherX = myint(x);
}

