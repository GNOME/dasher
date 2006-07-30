#ifndef __ONE_DIMENSIONAL_FILTER_H__
#define __ONE_DIMENSIONAL_FILTER_H__

#include "DefaultFilter.h"

class COneDimensionalFilter : public CDefaultFilter {
 public:
  COneDimensionalFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);

 private:
   virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);

   int iLastY;
   int iOffset;
};

#endif
