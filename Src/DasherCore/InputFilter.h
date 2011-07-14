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

  virtual void KeyDown(unsigned long Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {}
  
  virtual void KeyUp(unsigned long Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {}

  virtual bool Timer(unsigned long Time, CDasherView *m_pDasherView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol)=0;// { return false; };

  virtual void Activate() {};
  virtual void Deactivate() {};

  virtual bool GetMinWidth(int &iMinWidth) {
    return false;
  }
  
  virtual bool supportsPause() {return false;}
  
 protected:
  CDasherInterfaceBase * const m_pInterface;
};
}
/// @}

#endif
