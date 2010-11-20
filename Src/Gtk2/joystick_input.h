#ifndef __joystick_input_h__
#define __joystick_input_h__

#include "../DasherCore/DasherInput.h"
#include "../DasherCore/DasherInterfaceBase.h"
#include "../DasherCore/DasherTypes.h"
#include "Timer.h"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <linux/joystick.h>

using namespace Dasher;

class CDasherJoystickInput : public CDasherInput {
public:
  CDasherJoystickInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface) 
    : CDasherInput(pEventHandler, pSettingsStore, 16, "Joystick Input") {

    m_pInterface = pInterface;
    
  };

  // Fill pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  virtual int GetCoordinates(int iN, myint * pCoordinates) {
    UpdateCoordinates();

    pCoordinates[0] = m_iX;
    pCoordinates[1] = m_iY;

    return 1;
  };

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };

  void Activate() {
    // TODO: Error handling, check versioning of kernel joystick driver
    OpenDevice();
    UpdateCoordinates();
  };

  void Deactivate() {
    CloseDevice();
  };

private:
  void UpdateCoordinates() {
    js_event sEvent;

    while(read (iFd, &sEvent, sizeof(struct js_event)) > 0) {

      if(sEvent.type & JS_EVENT_AXIS) {
	switch(sEvent.number) {
	case 0: // Stick 1, X axis
	  m_iX = 2048 - (sEvent.value * 2048) / 32767;
	  break;
	case 1: // Stick 1, Y axis
	  m_iY = (sEvent.value * 2048) / 32767 + 2048;
	  break;
	default:
	  // Ignore other axes
	  break;
	}
      }
      else if(sEvent.type & JS_EVENT_BUTTON) {
	// For now map everything to button 100

	if(sEvent.value == 1)
	  m_pInterface->KeyDown(get_time(), 100);
	else
	  m_pInterface->KeyUp(get_time(), 100);
      }
    }
    /* EAGAIN is returned when the queue is empty */
    if (errno != EAGAIN) {
      /* error */
    }
  }

  void OpenDevice() {
    const char *szDeviceName=GetStringParameter(SP_JOYSTICK_DEVICE).c_str();

    iFd = open(szDeviceName, O_RDONLY | O_NONBLOCK);
  }

  void CloseDevice() {
    close(iFd);
  }

  myint m_iX;
  myint m_iY;

  /// File descriptor for the joystick device
  int iFd;

  CDasherInterfaceBase *m_pInterface;

};

class CDasherJoystickInputDiscrete : public CDasherInput {
public:
  CDasherJoystickInputDiscrete(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface) 
    : CDasherInput(pEventHandler, pSettingsStore, 17, "Joystick Input (Discrete)") {

    m_pInterface = pInterface;
    iZone = -1;
  };

  // Fill pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  virtual int GetCoordinates(int iN, myint * pCoordinates) {
    UpdateCoordinates();

    pCoordinates[0] = m_iX;
    pCoordinates[1] = m_iY;

    return 1;
  };

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };

  void Activate() {
    // TODO: Error handling, check versioning of kernel joystick driver
    OpenDevice();
    UpdateCoordinates();
  };

  void Deactivate() {
    CloseDevice();
  };

private:
  void UpdateCoordinates() {
    js_event sEvent;

    while(read (iFd, &sEvent, sizeof(struct js_event)) > 0) {

      if(sEvent.type & JS_EVENT_AXIS) {
	switch(sEvent.number) {
	case 0: // Stick 1, X axis
	  m_iX = sEvent.value;
	  break;
	case 1: // Stick 1, Y axis
	  m_iY = sEvent.value;
	  break;
	default:
	  // Ignore other axes
	  break;
	}
      }
      else if(sEvent.type & JS_EVENT_BUTTON) {
	// For now map everything to button 100

	if(sEvent.value == 1)
	  m_pInterface->KeyDown(get_time(), 100);
	else
	  m_pInterface->KeyUp(get_time(), 100);
      }
    }
    /* EAGAIN is returned when the queue is empty */
    if (errno != EAGAIN) {
      /* error */
    }

    int iNewZone = GetZone();

    if(iNewZone != iZone) {
      if(iZone > 0)
	m_pInterface->KeyUp(get_time(), iZone);
      if(iNewZone > 0)
	m_pInterface->KeyDown(get_time(), iNewZone);
    }

    iZone = iNewZone;
  }

  int GetZone() {
    const int iBound = 32767 / 2;

    if((m_iX < iBound) && (m_iX > -iBound) && (m_iY < iBound) && (m_iY > -iBound))
      return 0;
    else if(m_iX > m_iY) {
      if(m_iX > - m_iY) 
	return 3;
      else
	return 2;
    }
    else {  
      if(m_iX > - m_iY) 
	return 4;
      else
	return 1;
    }
  }

  void OpenDevice() {
    const char *szDeviceName=GetStringParameter(SP_JOYSTICK_DEVICE).c_str();

    iFd = open(szDeviceName, O_RDONLY | O_NONBLOCK);
  }

  void CloseDevice() {
    close(iFd);
  }

  myint m_iX;
  myint m_iY;

  /// File descriptor for the joystick device
  int iFd;

  CDasherInterfaceBase *m_pInterface;
  int iZone;

};

class CDasher1DJoystickInput : public CDasherInput {
public:
  CDasher1DJoystickInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface) 
    : CDasherInput(pEventHandler, pSettingsStore, 18, "Joystick Input (1D)") {

    m_pInterface = pInterface;
    iZone = -1;
  };

  // Fill pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  virtual int GetCoordinates(int iN, myint * pCoordinates) {
    UpdateCoordinates();

    pCoordinates[0] = 0;
    pCoordinates[1] = m_iY;

    return 1;
  };

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };

  void Activate() {
    // TODO: Error handling, check versioning of kernel joystick driver
    OpenDevice();
    UpdateCoordinates();
  };

  void Deactivate() {
    CloseDevice();
  };

private:
  void UpdateCoordinates() {
    js_event sEvent;

    while(read (iFd, &sEvent, sizeof(struct js_event)) > 0) {

      if(sEvent.type & JS_EVENT_AXIS) {
	switch(sEvent.number) {
	case 0: // Stick 1, X axis
	  m_iX = 2048 - (sEvent.value * 2048) / 32767;
	  break;
	case 1: // Stick 1, Y axis
	  m_iY = (sEvent.value * 2048) / 32767 + 2048;
	  break;
	default:
	  // Ignore other axes
	  break;
	}

	int iNewZone;

	if(m_iX > 3072) 
	  iNewZone = 1;
	else if(m_iX < 1024)
	  iNewZone = 2;
	else
	  iNewZone = 0;

	if(iNewZone != iZone) {
	  if((iNewZone == 0) && (iNewZone != -1))
	    m_pInterface->KeyUp(get_time(), 100);
	  else
	    m_pInterface->KeyDown(get_time(), 100);
	}

	iZone = iNewZone;
      }
      else if(sEvent.type & JS_EVENT_BUTTON) {
	// For now map everything to button 100

	if(sEvent.value == 1)
	  m_pInterface->KeyDown(get_time(), 100);
	else
	  m_pInterface->KeyUp(get_time(), 100);
      }
    }
    /* EAGAIN is returned when the queue is empty */
    if (errno != EAGAIN) {
      /* error */
    }
  }

  void OpenDevice() {
    const char *szDeviceName=GetStringParameter(SP_JOYSTICK_DEVICE).c_str();

    iFd = open(szDeviceName, O_RDONLY | O_NONBLOCK);
  }

  void CloseDevice() {
    close(iFd);
  }

  myint m_iX;
  myint m_iY;

  /// File descriptor for the joystick device
  int iFd;

  int iZone;

  CDasherInterfaceBase *m_pInterface;

};

#endif

