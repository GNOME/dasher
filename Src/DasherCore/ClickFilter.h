#ifndef __CLICK_FILTER_H__
#define __CLICK_FILTER_H__

#include "InputFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
  ///Utility class for transforming co-ordinates of a mouse click to zoom target
  class CZoomAdjuster {
  public:
    /// Adjust co-ordinates of mouse click into coordinates for zoom target.
    /// \param safety Fraction (/1024) by which not to zoom in (e.g. 25 = 
    /// zoom in ~~2.5% less than suggested)
    /// \param maxZoom maximum factor by which to zoom in; note that
    /// regardless of this parameter, we never zoom in to x<2.
    void AdjustZoomX(myint &iDasherX, CDasherView *comp, myint safety, myint maxZoom);
  };
  
class CClickFilter : public CStaticFilter, private CZoomAdjuster {
 public:
  CClickFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface)
    : CStaticFilter(pCreator, pInterface, 7, _("Click Mode")) { };

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);
  
 private:
  //for mouse lines
  myint m_iLastX, m_iLastY;
};
}
/// @}

#endif
