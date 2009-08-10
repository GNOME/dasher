#ifndef __EYETRACKER_FILTER_H__
#define __EYETRACKER_FILTER_H__

#include "DefaultFilter.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
class CEyetrackerFilter : public CDefaultFilter {
 public:
  CEyetrackerFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

 private:
  virtual void ApplyAutoCalibration(myint &iDasherX, myint &iDasherY, bool bUpdate);
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
  double xmax(double x, double y);

  myint m_iYAutoOffset;

  myint m_iSum;
  myint m_iSigBiasPixels;

  int m_iCounter;
  int m_iFilterTimescale;
};
}
/// @}

#endif
