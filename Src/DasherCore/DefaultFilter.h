#ifndef __DEFAULT_FILTER_H__
#define __DEFAULT_FILTER_H__

#include "InputFilter.h"
#include "AutoSpeedControl.h"

class CDefaultFilter : public CInputFilter {
 public:
  CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel);
  ~CDefaultFilter();

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual void DecorateView(CDasherView *pView);
  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);
 
 private:
  CAutoSpeedControl *m_pAutoSpeedControl;

  int m_iStatus;
  int m_iChangeTime;
  int m_iCircleRadius;
};

#endif
