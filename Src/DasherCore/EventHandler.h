#ifndef __eventhandler_h__
#define __eventhandler_h__

#include "DasherComponent.h"

#include <vector>

namespace Dasher {
  class CEventHandler;
}

class Dasher::CEventHandler {
 public:

  CEventHandler() {};
  ~CEventHandler() {};

  // Classes representing different event types.

  class CEvent {
  public:
    int m_iEventType;
  };

  class CParameterNotificationEvent : public CEvent {
  public:
    CParameterNotificationEvent( int iParameter ) {
      m_iEventType = 1;
      m_iParameter = iParameter;
    };

    int m_iParameter;
  };

  // Insert an event, which will be propagated to all listeners.

  void InsertEvent( CEvent *pEvent );

  // (Un)register a listener with the event handler.

  void RegisterListener( CDasherComponent *pListener );
  void UnregisterListener( CDasherComponent *pListener );

 protected:

  // Vector containing all currently registered listeners.

  std::vector< CDasherComponent * > m_vListeners;

}

#endif
