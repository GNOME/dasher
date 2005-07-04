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

class Dasher::CEditEvent : public Dasher::CEvent {
 public:
  CEditEvent( int iEditType, symbol iSymbol ) {
    m_iEventType = 2;
    m_iEditType = iEditType;
    m_iSymbol = iSymbol;
  };

  int m_iEditType;
  symbol m_iSymbol;
};

class Dasher::CEditContextEvent : public Dasher::CEvent {
 public:
  CEditContextEvent( int iMaxLength ) {
    m_iEventType = 3;
    m_iMaxLength = iMaxLength;
  };

  int m_iMaxLength;
};

#endif
