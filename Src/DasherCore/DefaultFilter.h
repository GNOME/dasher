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
  virtual bool supportsPause() {return true;}

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual bool Timer(int Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog);
  bool GetSettings(SModuleSettings **, int *);
 protected:
  virtual void CreateStartHandler();
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
  void ApplyOffset(myint &iDasherX, myint &iDasherY);
  
protected:
  /// Last-known Dasher-coords of the target
  myint m_iLastX, m_iLastY;
  bool m_bGotMouseCoords;
private:
  CAutoSpeedControl *m_pAutoSpeedControl;
  CStartHandler *m_pStartHandler;
  myint m_iSum;
  int m_iCounter;
};
}
/// @}

#endif
