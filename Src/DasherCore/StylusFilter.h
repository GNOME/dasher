#ifndef __STYLUS_FILTER_H__
#define __STYLUS_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup Input
/// @{
class CStylusFilter : public CDefaultFilter {
 public:
  CStylusFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *pDasherModel, long long int iID, const char *szName);

  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);
  virtual void KeyUp(int iTime, int iId, CDasherModel *pModel);
};
/// @}

#endif
