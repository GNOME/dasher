#ifndef __ONE_DIMENSIONAL_FILTER_H__
#define __ONE_DIMENSIONAL_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class COneDimensionalFilter : public CDefaultFilter {
 public:
//  COneDimensionalFilter(CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel);
  COneDimensionalFilter(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID = 4, const char *szName = _("One Dimensional Mode"));
  ///Override to remove DefaultFilters BP_REMAP_XTREME, BP_AUTOCALIBRATE, LP_OFFSET
  bool GetSettings(SModuleSettings **pSettings, int *iCount);
 protected:
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
  const myint forwardmax;
  virtual CStartHandler *MakeStartHandler();
};
}
/// @}

#endif
