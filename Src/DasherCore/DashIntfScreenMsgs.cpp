#include "DashIntfScreenMsgs.h"

using namespace Dasher;

void CDashIntfScreenMsgs::Message(const string &strText) {
  m_dqMessages.push_back(pair<CDasherScreen::Label*,unsigned long>(m_DasherScreen->MakeLabel(strText, GetLongParameter(LP_MESSAGE_FONTSIZE)), 0));
}

bool CDashIntfScreenMsgs::FinishRender(unsigned long ulTime) {
  bool bMsgsChanged=false;
  //Finally any messages. Newest that will fit at bottom, proceeding upwards
  while (!m_dqMessages.empty() && m_dqMessages.front().second && ulTime-m_dqMessages.front().second>GetLongParameter(LP_MESSAGE_TIME)) {
    //message has been displayed for long enough.
    m_dqMessages.pop_front(); // => stop displaying it
    bMsgsChanged=true;
  }
  if (!m_dqMessages.empty()) {
    //still messages to display...first find out longest-ago N that will fit
    const unsigned int iFontSize(GetLongParameter(LP_MESSAGE_FONTSIZE));
    screenint iY = m_DasherScreen->GetHeight();
    const screenint iMinY((iY*3)/4), iSW(m_DasherScreen->GetWidth());
    for (deque<pair<CDasherScreen::Label*, unsigned long> >::iterator it = m_dqMessages.begin(); it!=m_dqMessages.end() && iY>iMinY; it++) {
      if (it->second==0) {it->second = ulTime; bMsgsChanged=true;} //display message for first time
      iY-=m_DasherScreen->TextSize(it->first, iFontSize).second;
    }
    //then render oldest first proceeding downwards
    for (deque<pair<CDasherScreen::Label*, unsigned long> >::iterator it = m_dqMessages.begin(); it!=m_dqMessages.end() && it->second!=0; it++) {
      pair<screenint,screenint> textDims = m_DasherScreen->TextSize(it->first, iFontSize);
      //black (5) rectangle:
      m_DasherScreen->DrawRectangle((iSW - textDims.first)/2, iY, (iSW+textDims.first)/2, iY+textDims.second, 5, -1, -1);
      //white (0) text:
      m_DasherScreen->DrawString(it->first, (iSW-textDims.first)/2, iY, iFontSize, 0);
      iY+=textDims.second;
    }
  }
  return bMsgsChanged;
}

void CDashIntfScreenMsgs::ChangeScreen(CDasherScreen *pNewScreen) {
  CDasherInterfaceBase::ChangeScreen(pNewScreen);
  for (deque<pair<CDasherScreen::Label*,unsigned long> >::iterator it=m_dqMessages.begin(); it!=m_dqMessages.end(); it++) {
    const CDasherScreen::Label *pOldLabel(it->first);
    it->first = pNewScreen->MakeLabel(pOldLabel->m_strText, pOldLabel->m_iWrapSize);
    delete pOldLabel;
  }
}
