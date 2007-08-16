#ifndef __eventhandler_h__
#define __eventhandler_h__

#include <vector>

namespace Dasher {
  class CEventHandler;
  class CDasherComponent;
  class CDasherInterfaceBase;
  class CEvent;
}

/// \ingroup Core
/// @{
class Dasher::CEventHandler {
public:

  CEventHandler(Dasher::CDasherInterfaceBase * pInterface):m_pInterface(pInterface) {
    m_iInHandler = 0;
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
  std::vector < Dasher::CDasherComponent * >m_vListenerQueue;

  int m_iInHandler;

  Dasher::CDasherInterfaceBase * m_pInterface;

};
/// @}

#endif
