#ifndef __STYLUS_FILTER_H__
#define __STYLUS_FILTER_H__

#include "DefaultFilter.h"
#include "ClickFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class CStylusFilter : public CDefaultFilter {
 public:
  CStylusFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID=15, const char *szName=_("Stylus Control"));
  ///Override DefaultFilter (which supports pause), as we don't
  /// - motion requires continually holding stylus against screen
  virtual bool supportsPause() {return false;}
  virtual bool Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol);
  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
 protected:
  ///Transform coordinates of a click, to get location to zoom into.
  /// Default is to call the same CZoomAdjuster::AdjustZoomCoords as CClickFilter
  virtual void ApplyClickTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
  ///Do not make a start handler when in stylus mode
  virtual CStartHandler *MakeStartHandler();
 private:
  unsigned long m_iKeyDownTime;
};
}
/// @}

#endif
