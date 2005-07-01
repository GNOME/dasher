#ifndef __eventhandler_h__
#define __eventhandler_h__

#include "Event.h"
#include "DasherComponent.h"

#include <vector>

namespace Dasher {
  class CEventHandler;
}

class Dasher::CEventHandler {
 public:

  CEventHandler() {};
  ~CEventHandler() {};

  // Insert an event, which will be propagated to all listeners.

  void InsertEvent( Dasher::CEvent *pEvent );

  // (Un)register a listener with the event handler.

  void RegisterListener( Dasher::CDasherComponent *pListener );
  void UnregisterListener( Dasher::CDasherComponent *pListener );

 protected:

  // Vector containing all currently registered listeners.

  std::vector< Dasher::CDasherComponent * > m_vListeners;

};

#endif
