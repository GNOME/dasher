#ifndef __INPUT_FILTER_H__
#define __INPUT_FILTER_H__

#include "DasherView.h"
#include "DasherModel.h"
#include "ModuleManager.h"
#include "UserLogBase.h"
#include "DasherInput.h"

namespace Dasher {
  class CDasherInterfaceBase;
}
/// \defgroup InputFilter Input filters
/// @{
namespace Dasher {
class CInputFilter : public CDasherModule {
 public:
  CInputFilter(CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
    : CDasherModule(iID, InputMethod, szName), m_pInterface(pInterface) {
  };

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput) { return false; };

  virtual void KeyDown(int Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY) {
    KeyDown(Time, iId, pDasherView, pInput, pModel, pUserLog);
  };
  virtual void KeyUp(int Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel, bool bPos, int iX, int iY) {
    KeyUp(Time, iId, pDasherView, pInput, pModel);
  };

  virtual bool Timer(unsigned long Time, CDasherView *m_pDasherView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol)=0;// { return false; };

  virtual void Activate() {};
  virtual void Deactivate() {};

  virtual bool GetMinWidth(int &iMinWidth) {
    return false;
  }
  
  virtual bool supportsPause() {return false;}
  
 protected:
  CDasherInterfaceBase *m_pInterface;

  virtual void KeyDown(int Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog) {};
  virtual void KeyUp(int Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {};
};
}
/// @}

#endif
