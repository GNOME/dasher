#ifndef __TWO_BUTTON_DYNAMIC_FILTER_H__
#define __TWO_BUTTON_DYNAMIC_FILTER_H__

#include "DynamicFilter.h"

#include <deque>

/// \ingroup InputFilter
/// @{
class CTwoButtonDynamicFilter : public CDynamicFilter {
 public:
  CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~CTwoButtonDynamicFilter();

  // Inherited methods
  virtual bool DecorateView(CDasherView *pView);
 
  virtual void Activate();
  virtual void Deactivate();

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

 
  
 private:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);

  void AutoSpeedSample(int iTime, CDasherModel *pModel);
  void AutoSpeedUndo(int iCount);

  class SBTree {
  public:
    SBTree(int iValue);
    ~SBTree();

    void Add(int iValue);
    SBTree* Delete(int iValue);

    int GetCount() {
      return m_iCount;
    };

    int GetOffset(int iOffset);

    void SetRightMost(SBTree* pNewTree);

  private:
    int m_iValue;
    SBTree *m_pLeft;
    SBTree *m_pRight;
    int m_iCount;
  };

  int m_iLastTime;
  SBTree *m_pTree;
  std::deque<int> m_deOffsetQueue;
};
/// @}

#endif
