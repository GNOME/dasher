#ifndef __GameModeHelper_h__
#define __GameModeHelper_h__

// TODO: There seems to be an implicit dependence between these header files:
#include "Common\WinCommon.h"
#include "DasherInterface.h"

#include <string>

class CGameModeHelper {
public:
  CGameModeHelper(Dasher::CDasherInterfaceBase *pInterface);

  void Output(std::string strText);
  void Delete(int iCount);

private:
  Dasher::CDasherInterfaceBase *m_pInterface;

  std::string strTarget;
  std::string strOutput;
};

#endif