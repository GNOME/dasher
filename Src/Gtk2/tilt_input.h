// tilt_input.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __joystick_input_h__
#define __joystick_input_h__

#include "../DasherCore/DasherInput.h"
#include "../DasherCore/DasherInterfaceBase.h"
#include "../DasherCore/DasherTypes.h"
#include "Timer.h"


#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
        

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

using namespace Dasher;

class CDasherTiltInput : public CDasherInput {
public:
  CDasherTiltInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface) 
    : CDasherInput(pEventHandler, pSettingsStore, 19, "Tilt Input") {

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
    write(m_iFD, "z", 1);
    
    char buf[4];
    int res = read(m_iFD, buf, 3); 
    buf[res]=0;

    unsigned int iValue = static_cast<unsigned char>(buf[0]);
    unsigned int iValue2 = static_cast<unsigned char>(buf[1]);

    m_iX = ((signed int)iValue - 128) * 2048 / 16 + 2048;
    m_iY = ((signed int)iValue2 - 128) * 2048 / 16 + 2048;
    
    //std::cout << iValue << " - " << iValue2 << std::endl;
  }

  void OpenDevice() {

    struct termios newtio;
    
    m_iFD = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
    if (m_iFD <0) {perror(MODEMDEVICE); exit(-1); }


    //std::cout << "File descriptor: " << m_iFD << std::endl;
    
    tcgetattr(m_iFD,&oldtio); /* save current port settings */
    
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 3;   /* blocking read until 5 chars received */
    
    tcflush(m_iFD, TCIFLUSH);
    tcsetattr(m_iFD,TCSANOW,&newtio);
  }

  void CloseDevice() { 
    tcsetattr(m_iFD,TCSANOW,&oldtio);
    close(m_iFD);
  }

  myint m_iX;
  myint m_iY;

  int m_iFD;

  struct termios oldtio;

  CDasherInterfaceBase *m_pInterface;

};

#endif

