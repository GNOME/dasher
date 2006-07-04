#include "GameModeHelper.h"

CGameModeHelper::CGameModeHelper(Dasher::CDasherInterfaceBase *pInterface) {
  m_pInterface = pInterface;

  strTarget = "this is a test";

  m_pInterface->AddGameModeString(strTarget);
  strOutput = "";
}

void CGameModeHelper::Output(std::string strText) {
  strOutput.append(strText);

  if(strOutput == strTarget) {
    int iDummy;
    iDummy = 0;
  }
}

void CGameModeHelper::Delete(int iCount) {
  strOutput = strOutput.substr(0, strOutput.size() - iCount);
}