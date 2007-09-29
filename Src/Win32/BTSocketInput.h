#pragma once

#include "../DasherCore/DasherInput.h"

#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace Dasher {
  class CBTSocketInput;
} 

class Dasher::CBTSocketInput : public CDasherInput {
public:
  CBTSocketInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);
  ~CBTSocketInput(void);

  virtual int GetCoordinates(int iN, myint * pCoordinates);

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

  virtual void Activate();
  virtual void Deactivate();

private:
  SOCKET m_oSocket;
 
};