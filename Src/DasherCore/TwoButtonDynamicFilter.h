#ifndef __TWO_BUTTON_DYNAMIC_FILTER_H__
#define __TWO_BUTTON_DYNAMIC_FILTER_H__

#include "InputFilter.h"

#include <deque>

class CTwoButtonDynamicFilter : public CInputFilter {
 public:
  CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, long long int iID, int iType, const char *szName);
  ~CTwoButtonDynamicFilter();

  // Inherited methods
  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel, CUserLogBase *pUserLog);
  virtual void KeyUp(int iTime, int iId, CDasherModel *pModel);
  virtual void Activate();
  virtual void Deactivate();

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

  virtual bool GetMinWidth(int &iMinWidth);
  
 protected:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);

  bool m_bDecorationChanged;

 private:
  void Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);
  void AutoSpeedSample(int iTime, CDasherModel *pModel);

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

  private:
    int m_iValue;
    SBTree *m_pLeft;
    SBTree *m_pRight;
    int m_iCount;
  };

  SBTree *m_pTree;

  int m_iLastButton;
  int m_iKeyDownTime;
  int m_iState; // 0 = paused, 1 = running 2 = backing off
  int m_iQueueId;
  std::deque<int> m_deQueueTimes;

  bool m_bKeyDown;
  bool m_bKeyHandled;
  int m_iHeldId;
  int m_iLastTime;

  CUserLogBase *m_pUserLog;
};

#endif