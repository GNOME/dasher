#include "../Common/Common.h"
#include "EyetrackerFilter.h"

#include <iostream>

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {BP_AUTOCALIBRATE, T_BOOL, -1, -1, -1, -1, _("Automatic calibration")}
};

CEyetrackerFilter::CEyetrackerFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, 5, _("Eyetracker Mode")) {

  // Setup some autocalibration parameters
  m_iYAutoOffset = 0;
  
  m_iSum = 0;
  m_iSigBiasPixels = static_cast<int>(0.5 * GetLongParameter(LP_MAX_Y));

  m_iCounter = 0;
  m_iFilterTimescale = 20;
}

bool CEyetrackerFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};

void CEyetrackerFilter::ApplyTransform(myint &iDasherX, myint &iDasherY) {
  
  
  // Y co-ordinate...
  myint dasherOY=(myint)GetLongParameter(LP_OY); 
  double double_y = ((iDasherY-dasherOY)/(double)(dasherOY) ); // Fraction above the crosshair
  static const double repulsionparameter=0.5;
  iDasherY = myint(dasherOY * (1.0 + double_y + (double_y*double_y*double_y * repulsionparameter )));
  
  // X co-ordinate...  
  iDasherX = max(iDasherX,myint(GetLongParameter(LP_OX) * xmax(double_y)));
}

double CEyetrackerFilter::xmax(double y) {
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

void CEyetrackerFilter::ApplyAutoCalibration(myint &iDasherX, myint &iDasherY, bool bUpdate) {
  if(!GetBoolParameter(BP_AUTOCALIBRATE))
    return;

  // TODO: It turns out that this was previously computed in pixels,
  // altough everythign else made use of Dasher coordinates. Hack in a
  // factor of 10 to get the offset in Dasher coordinates, but it
  // would be a good idea at some point to sort this out properly.

  iDasherY += 10 * m_iYAutoOffset;

  if(!bUpdate)
    return;

  // Now update the auto offset

  myint iDistY = (myint)GetLongParameter(LP_OY) - iDasherY; // Distance above crosshair

  m_iSum += iDistY;
  ++m_iCounter;

  if(m_iCounter > m_iFilterTimescale) {
    m_iCounter = 0;

    // 'Conditions A', as specified by DJCM.  Only make the auto-offset
    // change if we're past the significance boundary.
    
    // TODO: The conditionals below don't make any sense!

    if(m_iSum > m_iSigBiasPixels || m_iSum < -m_iSigBiasPixels) {
      if(m_iSum > m_iFilterTimescale) {
      	m_iYAutoOffset = m_iYAutoOffset - 1;
      }
      else if(m_iSum < -m_iFilterTimescale)
	      m_iYAutoOffset = m_iYAutoOffset + 1;

      // TODO: Should this maybe be outside of the if, so that we have
      // an average offset, rather than just slowing the time it takes
      // to drift?
      m_iSum = 0;
    }
  }
}
