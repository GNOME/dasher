#ifndef __GameModeHelper_h__
#define __GameModeHelper_h__

// TODO: There seems to be an implicit dependence between these header files:
#include "Common\WinCommon.h"
#include "../DasherCore/DasherInterfaceBase.h"

#include <string>

class CGameModeHelper {
public:
  CGameModeHelper(Dasher::CDasherInterfaceBase *pInterface);

  void Output(std::string strText);
  void Delete(int iCount);
  void Message(int message, const void* messagedata);
private:
  Dasher::CDasherInterfaceBase *m_pInterface;

    void UpdateTargetLabel();
  std::string m_strTarget;
  std::string m_strOutput;
};

#endif