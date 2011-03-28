#ifndef __CLICK_FILTER_H__
#define __CLICK_FILTER_H__

#include "InputFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class CClickFilter : public CInputFilter {
 public:
  CClickFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
    : CInputFilter(pEventHandler, pSettingsStore, pInterface, 7, _("Click Mode")) { };

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual bool Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pDasherModel, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY);
  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

  static void AdjustZoomCoords(myint &iDasherX, myint &iDasherY, CDasherView *comp);
  
 private:
  //for mouse lines
  myint m_iLastX, m_iLastY;
};
}
/// @}

#endif
