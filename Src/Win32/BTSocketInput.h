#pragma once

#include "../DasherCore/DasherInput.h"

#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace Dasher {
  class CBTSocketInput;
} 

class Dasher::CBTSocketInput : public CDasherCoordInput {
public:
  CBTSocketInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);
  ~CBTSocketInput(void);

  virtual bool GetDasherCoords(myint &iDasherX, myint &iDasherY, CDasherView *pView);

  virtual void Activate();
  virtual void Deactivate();

private:
  SOCKET m_oSocket;
 
};
