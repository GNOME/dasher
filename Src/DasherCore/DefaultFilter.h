#ifndef __DEFAULT_FILTER_H__
#define __DEFAULT_FILTER_H__

#include "InputFilter.h"
#include "AutoSpeedControl.h"
#include "StartHandler.h"

class CDefaultFilter : public CInputFilter {
 public:
  CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel, long long int iID);
  ~CDefaultFilter();

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool DecorateView(CDasherView *pView);
  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);

 protected:
  virtual void CreateStartHandler();
  
 private:
  virtual void DrawMouse(CDasherView *pView);
  virtual void DrawMouseLine(CDasherView *pView);
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);

  CAutoSpeedControl *m_pAutoSpeedControl;
  CStartHandler *m_pStartHandler;
};

#endif
