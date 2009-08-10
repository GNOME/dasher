#ifndef __ONE_DIMENSIONAL_FILTER_H__
#define __ONE_DIMENSIONAL_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class COneDimensionalFilter : public CDefaultFilter {
 public:
  COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);

 private:
   virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);

   int iLastY;
   int iOffset;
};
}
/// @}

#endif
