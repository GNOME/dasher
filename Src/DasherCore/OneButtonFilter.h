#ifndef __ONE_BUTTON_FILTER_H__
#define __ONE_BUTTON_FILTER_H__

#include "InputFilter.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
class COneButtonFilter : public CInputFilter {
 public:
  COneButtonFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~COneButtonFilter();

  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
  bool GetSettings(SModuleSettings **pSettings, int *iCount);
 private:
  ///true iff the scan line is moving down/up, or is in the 'reverse' stage
  bool bStarted;
  ///set by DecorateView: true iff we have drawn an undecorated display, else false.
  bool m_bNoDecorations;
  int iStartTime;
  int iLocation;
};
}
/// @}

#endif
