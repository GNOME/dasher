#ifndef __ONE_BUTTON_FILTER_H__
#define __ONE_BUTTON_FILTER_H__

#include "InputFilter.h"

class COneButtonFilter : public CInputFilter {
 public:
  COneButtonFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore);
  ~COneButtonFilter();

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual void DecorateView(CDasherView *pView);
  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);
  
 private:

  bool bStarted;
  int iStartTime;
  int iLocation;
};

#endif
