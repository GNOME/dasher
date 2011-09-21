#ifndef __DEFAULT_FILTER_H__
#define __DEFAULT_FILTER_H__

#include "DynamicFilter.h"
#include "AutoSpeedControl.h"
#include "StartHandler.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
class CDefaultFilter : public CDynamicFilter, public CSettingsObserver {
 public:
  CDefaultFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName);
  ~CDefaultFilter();

  /// Responds to changes in BP_START_MOUSE / BP_MOUSEPOS_MODE to create StartHandler
  virtual void HandleEvent(int iParameter);

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
  virtual void Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol);
  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void Activate();
  virtual void Deactivate();
  bool GetSettings(SModuleSettings **, int *);
  void pause();
  //pauses, and calls the interface's Done() method
  void stop();
 protected:
  void CreateStartHandler();
  void run(unsigned long iTime);
  virtual CStartHandler *MakeStartHandler();
  virtual void ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
  void ApplyOffset(myint &iDasherX, myint &iDasherY);
  
  /// Last-known Dasher-coords of the target
  myint m_iLastX, m_iLastY;
  bool m_bGotMouseCoords;
private:
  friend class CCircleStartHandler;
  friend class CTwoBoxStartHandler;
  CAutoSpeedControl *m_pAutoSpeedControl;
  myint m_iSum;
  CStartHandler *m_pStartHandler;
  int m_iCounter;
  bool m_bTurbo;
};
}
/// @}

#endif
