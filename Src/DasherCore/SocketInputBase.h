// SocketInputBase.h
//
// (C) Copyright Seb Wills 2005
//
// Abstract base class for socket input: parent of non-abstract classes in each implementation (Windows, Linux, ...),

#ifndef __socketinputbase_h__
#define __socketinputbase_h__

#include "../DasherCore/DasherInput.h"
#include "../DasherCore/DasherComponent.h"
#include "../DasherCore/EventHandler.h"

#include <iostream>

#define DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT 2      // just X and Y for now
#define DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH 128

namespace Dasher {
  class CSocketInputBase;

using namespace std;
/// \ingroup Input 
/// \{
class CSocketInputBase : public CDasherInput {

public:

  CSocketInputBase(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);

  virtual ~CSocketInputBase();

  virtual void HandleEvent(CEvent * pEvent);

  virtual void SetDebug(bool _debug);

  virtual bool StartListening();

  virtual void StopListening();

  virtual bool isListening() {
    return readerRunning;
  }

  virtual void SetReaderPort(int port);

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

  void Activate() {
    StartListening();
  };
  
  void Deactivate() {
    StopListening();
  };

  // Defines the label used in the input stream for a particular coordinate.
  // We make our own copy of the label, in our own buffer. This should ensure thread-safety.
  // Even if this method is called while our other thread is doing a strcmp on the label,
  // the buffer will always be null-terminated somewhere (even if the last byte of the buffer, which is
  // never overwritten), so won't segfault.
  virtual void SetCoordinateLabel(int iWhichCoordinate, const char *Label);

  virtual void SetRawRange(int iWhich, double dMin, double dMax);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

protected:

  myint dasherCoordinates[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  myint dasherMaxCoordinateValues[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  double rawMinValues[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  double rawMaxValues[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT];
  int coordinateCount;
  char coordinateNames[DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT][DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH + 1];

  int port;
  bool debug_socket_input;

  int sock;

  char buffer[4096];

  bool readerRunning;

  virtual bool LaunchReaderThread() =0;

  virtual void CancelReaderThread() =0;

  virtual void ReadForever();

  virtual void ParseMessage(char *message);

  virtual void ReportErrnoError(std::string prefix); // override as appropriate for each platform

  virtual void ReportError(std::string s); // override as appropriate for each platform
    
  virtual void SocketDebugMsg(const char *pszFormat, ...);
  
};
}
/// \}
#endif



