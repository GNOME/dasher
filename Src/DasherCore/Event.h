#ifndef __event_h__
#define __event_h__

// Classes representing different event types.

#include <string>
#include "DasherTypes.h"

namespace Dasher{
  class CEvent;
  class CParameterNotificationEvent;
  class CEditEvent;
  class CEditContextEvent;
  class CStartEvent;
  class CStopEvent;
}

enum { EV_PARAM_NOTIFY=1, EV_EDIT, EV_EDIT_CONTEXT, EV_START, EV_STOP };

class Dasher::CEvent {
 public:
  int m_iEventType;
};

class Dasher::CParameterNotificationEvent : public Dasher::CEvent {
 public:
  CParameterNotificationEvent( int iParameter ) {
    m_iEventType = EV_PARAM_NOTIFY;
    m_iParameter = iParameter;
  };
  
  int m_iParameter;
};

class Dasher::CEditEvent : public Dasher::CEvent {
 public:
   CEditEvent( int iEditType, const std::string &sText ) {
    m_iEventType = EV_EDIT;
    m_iEditType = iEditType;
    m_sText = sText;
  };

  int m_iEditType;
  std::string m_sText;
};

class Dasher::CEditContextEvent : public Dasher::CEvent {
 public:
  CEditContextEvent( int iMaxLength ) {
    m_iEventType = EV_EDIT_CONTEXT;
    m_iMaxLength = iMaxLength;
  };

  int m_iMaxLength;
};

class Dasher::CStartEvent : public Dasher::CEvent {
 public:
  CStartEvent() {
    m_iEventType = EV_START;
  };
};

class Dasher::CStopEvent : public Dasher::CEvent {
 public:
  CStopEvent() {
    m_iEventType = EV_STOP;
  };
};


#endif
