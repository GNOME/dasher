#ifndef __STYLUS_FILTER_H__
#define __STYLUS_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class CStylusFilter : public CDefaultFilter {
 public:
  CStylusFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName);
  ///Override DefaultFilter (which supports pause), as we don't
  /// - motion requires continually holding stylus against screen
  virtual bool supportsPause() {return false;}
  virtual bool Timer(int Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog);
  virtual void KeyUp(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  ///Transform coordinates of a click, to get location to zoom into.
  /// Default is to call CClickFilter::AdjustZoomCoords, which adds
  /// a safety margin according to LP_S, checks we don't exceed the
  /// zoom factor given by LP_MAXZOOM, and ensures x>=2.
  virtual void ApplyClickTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
 private:
  int m_iKeyDownTime;
};
}
/// @}

#endif
