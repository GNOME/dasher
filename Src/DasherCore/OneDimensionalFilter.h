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
 protected:
   virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
};
}
/// @}

#endif
