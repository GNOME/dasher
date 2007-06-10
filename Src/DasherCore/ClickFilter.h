#ifndef __CLICK_FILTER_H__
#define __CLICK_FILTER_H__

#include "InputFilter.h"

/// \ingroup InputFilter
/// @{
class CClickFilter : public CInputFilter {
 public:
  CClickFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~CClickFilter();

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY);
 
 private:
  CDasherView *pDasherView;
};
/// @}

#endif
