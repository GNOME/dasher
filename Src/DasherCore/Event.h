#ifndef __event_h__
#define __event_h__

// Classes representing different event types.

namespace Dasher{
  class CEvent;
  class CParameterNotificationEvent;
}

class Dasher::CEvent {
 public:
  int m_iEventType;
};

class Dasher::CParameterNotificationEvent : public Dasher::CEvent {
 public:
  CParameterNotificationEvent( int iParameter ) {
    m_iEventType = 1;
    m_iParameter = iParameter;
  };
  
  int m_iParameter;
};

#endif
