#ifndef __CLICK_FILTER_H__
#define __CLICK_FILTER_H__

#include "InputFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
  ///Utility class for transforming co-ordinates of a mouse click to zoom target
  class CZoomAdjuster : protected CSettingsUser {
  public:
    //Everything public so we can use composition as well as inheritance
    CZoomAdjuster(CSettingsUser *pCreator);
    /// Adjust co-ordinates of mouse click into coordinates for zoom target.
    /// Adds a safety margin according to LP_S, checks we don't exceed the
    /// zoom factor given by LP_MAXZOOM, and ensures x>=2.
    void AdjustZoomCoords(myint &iDasherX, myint &iDasherY, CDasherView *comp);
  };
  
class CClickFilter : public CInputFilter, private CZoomAdjuster {
 public:
  CClickFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface)
    : CInputFilter(pInterface, 7, _("Click Mode")), CZoomAdjuster(pCreator) { };

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual bool Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pDasherModel, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, bool bPos, int iX, int iY);
  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);
  
 private:
  //for mouse lines
  myint m_iLastX, m_iLastY;
};
}
/// @}

#endif
