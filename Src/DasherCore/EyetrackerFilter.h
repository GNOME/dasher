#ifndef __EYETRACKER_FILTER_H__
#define __EYETRACKER_FILTER_H__

#include "DefaultFilter.h"

class CEyetrackerFilter : public CDefaultFilter {
 public:
  CEyetrackerFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel);

 private:
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
  double xmax(double x, double y);
};

#endif
