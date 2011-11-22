#ifndef __ONE_BUTTON_FILTER_H__
#define __ONE_BUTTON_FILTER_H__

#include "InputFilter.h"
#include "SettingsStore.h"
namespace Dasher {
/// \ingroup InputFilter
/// @{
class COneButtonFilter : public CStaticFilter {
 public:
  COneButtonFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface);

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual void Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol);
  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  bool GetSettings(SModuleSettings **pSettings, int *iCount);
 private:
  ///true iff the scan line is moving down/up, or is in the 'reverse' stage
  bool bStarted;
  ///set by DecorateView: true iff we have drawn an undecorated display, else false.
  bool m_bNoDecorations;
  unsigned long iStartTime;
  int iLocation;
};
}
/// @}

#endif
