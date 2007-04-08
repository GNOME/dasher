#include "ActionButton.h"
#include "DasherInterfaceBase.h"
#include "DasherTypes.h"

#include <iostream>

CActionButton::CActionButton(Dasher::CDasherInterfaceBase *pInterface, const std::string &strCommand, bool bAlwaysVisible) 
  : m_strCommand(strCommand) {
  m_pInterface = pInterface;
  m_bAlwaysVisible = bAlwaysVisible;
}

void CActionButton::SetPosition(int iX, int iY, int iWidth, int iHeight) {
  m_iX = iX;
  m_iY = iY;
  m_iWidth = iWidth;
  m_iHeight = iHeight;
}

void CActionButton::Draw(Dasher::CDasherScreen *pScreen, bool bVisible) {
  if(bVisible || m_bAlwaysVisible)
    pScreen->DrawRectangle(m_iX, m_iY, m_iX + m_iWidth, m_iY + m_iHeight, 1, 2, Dasher::Opts::Nodes1, true, true, 1);
}

bool CActionButton::HandleClickDown(int iTime, int iX, int iY, bool bVisible) {
  if(!bVisible && !m_bAlwaysVisible)
    return false;

  if((iX > m_iX) && (iX < (m_iX + m_iWidth)) && (iY > m_iY) && (iY < (m_iY + m_iHeight))) {
    Execute(iTime);
    return true;
  }
  else
    return false;
}

bool CActionButton::HandleClickUp(int iTime, int iX, int iY, bool bVisible) {
  if(!bVisible && !m_bAlwaysVisible)
    return false;

  if((iX > m_iX) && (iX < (m_iX + m_iWidth)) && (iY > m_iY) && (iY < (m_iY + m_iHeight)))
    return true;
  else
    return false;
}

void CActionButton::Execute(int iTime) {
  m_pInterface->ExecuteCommand(m_strCommand);
}
