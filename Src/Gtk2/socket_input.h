// socket_input.h
//
// (C) Copyright Seb Wills 2005
//
// This class handles reading pointer data from a network socket

#ifndef __socket_input_h__
#define __socket_input_h__

#include "../DasherCore/DasherInput.h"
#include "../DasherCore/DasherComponent.h"
#include "../DasherCore/EventHandler.h"

#include <iostream>
#include <pthread.h>

#define DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT 2      // just X and Y for now
#define DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH 128

namespace Dasher {
  class CDasherSocketInput;
}

using namespace Dasher;
using namespace std;

class Dasher::CDasherSocketInput:public CDasherInput, public CDasherComponent {

  // This non-member launcher stub function is required because pthreads can't launch a non-static member method.
  friend void *ThreadLauncherStub(void *_myClass) {
    CDasherSocketInput *myClass = (CDasherSocketInput *) _myClass;
    myClass->ReadForever();
  }

public:

  CDasherSocketInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);

  virtual bool SetDebug(bool _debug) {
    debug_socket_input = _debug;
  }

  void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool StartListening();

  virtual void StopListening();

  virtual bool isListening() {
    return readerRunning;
  }

  virtual void SetPort(int port);

  virtual int GetPort() {
    return port;
  }

  void SetCoordinateCount(int _coordinateCount) {
    DASHER_ASSERT(_coordinateCount <= DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT);
    coordinateCount = _coordinateCount;
  }

  // Fills pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  int GetCoordinates(int iN, myint * pCoordinates) {

    for(int i = 0; i < iN && i < coordinateCount; i++) {
      pCoordinates[i] = dasherCoordinates[i];
    }
    return 1;
  };

  // Get the number of co-ordinates that this device supplies

  int GetCoordinateCount() {
    return coordinateCount;
  };

  void SetMaxCoordinates(int iN, myint * iDasherMax) {
    for(int i = 0; i < iN && i < DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT; i++) {
      dasherMaxCoordinateValues[i] = iDasherMax[i];
    }
  };

  // Defines the label used in the input stream for a particular coordinate.
  // We make our own copy of the label, in our own buffer. This should ensure thread-safety.
  // Even if this method is called while our other thread is doing a strcmp on the label,
  // the buffer will always be null-terminated somewhere (even if the last byte of the buffer, which is
  // never overwritten), so won't segfault.
  virtual void SetCoordinateLabel(int iWhichCoordinate, const char *Label);

  virtual void SetRawRange(int iWhich, double dMin, double dMax) {
    rawMinValues[iWhich] = dMin;
    rawMaxValues[iWhich] = dMax;
    if(debug_socket_input) {
      std::cerr << "Socket input: set coordinate " << iWhich << " input range to: min: " << dMin << ", max: " << dMax << "." << std::endl;
    }
  }

private:
  myint dasherCoordinates[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  myint dasherMaxCoordinateValues[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  double rawMinValues[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  double rawMaxValues[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  int coordinateCount;
  char coordinateNames[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT][DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH + 1];

  int port;
  int sock;
  bool debug_socket_input;

  char buffer[4096];

  pthread_t readerThread;
  bool readerRunning;

  virtual void *ReadForever();

  virtual bool ParseMessage(char *message);

};

#endif
