#ifndef __TWO_BUTTON_DYNAMIC_FILTER_H__
#define __TWO_BUTTON_DYNAMIC_FILTER_H__

#include "InputFilter.h"

class CTwoButtonDynamicFilter : public CInputFilter {
 public:
  CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);

  // Inherited methods
  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);
  virtual void KeyUp(int iTime, int iId, CDasherModel *pModel);
  virtual void Activate();
  virtual void Deactivate();

 private:
  int m_iKeyTime;

  bool m_bBackoff;
  bool m_bDecorationChanged;
};

#endif
