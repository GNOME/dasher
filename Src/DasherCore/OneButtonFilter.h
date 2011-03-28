#ifndef __ONE_BUTTON_FILTER_H__
#define __ONE_BUTTON_FILTER_H__

#include "InputFilter.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
class COneButtonFilter : public CInputFilter {
 public:
  COneButtonFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~COneButtonFilter();

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual bool Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog);
  bool GetSettings(SModuleSettings **pSettings, int *iCount);
 private:
  ///true iff the scan line is moving down/up, or is in the 'reverse' stage
  bool bStarted;
  ///set by DecorateView: true iff we have drawn an undecorated display, else false.
  bool m_bNoDecorations;
  int iStartTime;
  int iLocation;
};
}
/// @}

#endif
