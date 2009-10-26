#ifndef __CLICK_FILTER_H__
#define __CLICK_FILTER_H__

#include "InputFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class CClickFilter : public CInputFilter {
 public:
  CClickFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
    : CInputFilter(pEventHandler, pSettingsStore, pInterface, 7, 1, _("Click Mode")) { };

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *pDasherView, CDasherModel *pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY);
  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);
};
}
/// @}

#endif
