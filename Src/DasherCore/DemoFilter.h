#ifndef __DEMO_FILTER_H__
#define __DEMO_FILTER_H__

#include "DynamicFilter.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
class CDemoFilter : public CDynamicFilter {
 public:
  CDemoFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate);
  ~CDemoFilter();

  virtual void HandleEvent(int iParameter);

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual void Timer(unsigned long Time, CDasherView *m_pDasherView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol);
  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void Activate();
  virtual void Deactivate();
 private:
  double m_dSpring, m_dNoiseNew, m_dNoiseOld;
  double m_dNoiseX, m_dNoiseY;
  myint m_iDemoX, m_iDemoY;
};
}
/// @}

#endif
