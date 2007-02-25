#ifndef __DYNAMIC_FILTER_H__
#define __DYNAMIC_FILTER_H__

#include "TwoButtonDynamicFilter.h"

/// \ingroup InputFilter
/// @{
class CDynamicFilter : public CTwoButtonDynamicFilter {
 public:
  CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~CDynamicFilter();

  virtual bool DecorateView(CDasherView *pView);

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

 protected:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);
  
 private:
  int m_iTarget;

  int *m_iTargetX;
  int *m_iTargetY;
};
/// @}

#endif
