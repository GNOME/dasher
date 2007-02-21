#ifndef __ONE_DIMENSIONAL_FILTER_H__
#define __ONE_DIMENSIONAL_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup Input
/// @{
class COneDimensionalFilter : public CDefaultFilter {
 public:
  COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel);

 private:
   virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);

   int iLastY;
   int iOffset;
};
/// @}

#endif
