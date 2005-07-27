#ifndef __eventhandler_h__
#define __eventhandler_h__

#include <vector>

namespace Dasher {
  class CEventHandler;
  class CDasherComponent;
  class CDasherInterfaceBase;
  class CEvent;
}

class Dasher::CEventHandler {
public:

  CEventHandler(Dasher::CDasherInterfaceBase * pInterface):m_pInterface(pInterface) {
  };
  ~CEventHandler() {
  };

  // Insert an event, which will be propagated to all listeners.

  void InsertEvent(Dasher::CEvent * pEvent);

  // (Un)register a listener with the event handler.

  void RegisterListener(Dasher::CDasherComponent * pListener);
  void UnregisterListener(Dasher::CDasherComponent * pListener);

protected:

  // Vector containing all currently registered listeners.

  std::vector < Dasher::CDasherComponent * >m_vListeners;

  Dasher::CDasherInterfaceBase * m_pInterface;

};

#endif
