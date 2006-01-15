#include "DynamicFilter.h"

CDynamicFilter::CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore)
  : CInputFilter(pEventHandler, pSettingsStore) {
  m_iTarget = 0;

  m_iTargetX = new int[2];
  m_iTargetY = new int[2];

  m_iTargetX[0] = 100;
  m_iTargetY[0] = 100;

  m_iTargetX[1] = 100;
  m_iTargetY[1] = 3996; 
}

CDynamicFilter::~CDynamicFilter() {
  delete[] m_iTargetX;
  delete[] m_iTargetY;  
}

void CDynamicFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  m_pDasherModel->Tap_on_display(m_iTargetX[m_iTarget], m_iTargetY[m_iTarget], Time, 0, 0);
}

void CDynamicFilter::KeyDown(int iId, CDasherModel *pModel) {
  if(iId == 1)
    m_iTarget = 1 - m_iTarget;
}
