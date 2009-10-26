#ifndef __DEFAULT_FILTER_H__
#define __DEFAULT_FILTER_H__

#include "InputFilter.h"
#include "AutoSpeedControl.h"
#include "StartHandler.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
class CDefaultFilter : public CInputFilter {
 public:
  CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName);
  ~CDefaultFilter();

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog);

 protected:
  virtual void CreateStartHandler();
  virtual void ApplyAutoCalibration(myint &iDasherX, myint &iDasherY, bool bUpdate);
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
  
 private:
  virtual void DrawMouse(CDasherView *pView);
  virtual void DrawMouseLine(CDasherView *pView);

  CAutoSpeedControl *m_pAutoSpeedControl;
  CStartHandler *m_pStartHandler;
};
}
/// @}

#endif
