#ifndef __ONE_DIMENSIONAL_FILTER_H__
#define __ONE_DIMENSIONAL_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class COneDimensionalFilter : public CDefaultFilter {
 public:
//  COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel);
  COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface,  ModuleID_t iID = 4, const char *szName = _("One Dimensional Mode"));
  bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  ///Override to remove DefaultFilters BP_REMAP_XTREME, BP_AUTOCALIBRATE, LP_OFFSET
  bool GetSettings(SModuleSettings **pSettings, int *iCount);
 protected:
   virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
   const myint forwardmax;
   myint m_iDasherMaxX;
};
}
/// @}

#endif
