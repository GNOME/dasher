// SocketInputBase.h
//
// (C) Copyright Seb Wills 2005
//
// Abstract base class for socket input: parent of non-abstract classes in each implementation (Windows, Linux, ...),

#ifndef __socketinputbase_h__
#define __socketinputbase_h__

#include "DasherInput.h"
#include "SettingsStore.h"
#include "Messages.h"

#include <iostream>

#define DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT 2      // just X and Y for now
#define DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH 128

namespace Dasher {
  class CSocketInputBase;

using namespace std;
/// \ingroup Input
/// \{
class CSocketInputBase : public CScreenCoordInput, public CSettingsUserObserver {

public:

  CSocketInputBase(CSettingsUser *pCreator, CMessageDisplay *pMsgs);

  virtual ~CSocketInputBase();

  virtual void HandleEvent(int iParameter);

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

  /// Gets the last coordinates received; if only one coordinate is being read, this is put
  /// into iDasherY (and iDasherX set to 0).
  bool GetScreenCoords(screenint &iScreenX, screenint &iScreenY, CDasherView *pView) {

    //update max values for reader thread...(note any changes here won't be incorporated
    // until values are next received over socket, but never mind)
    dasherMaxCoordinateValues[0] = pView->Screen()->GetWidth();
    dasherMaxCoordinateValues[1] = pView->Screen()->GetHeight();

    if (coordinateCount==1) {
      iScreenX = 0;
      iScreenY = dasherCoordinates[0];
    } else if (coordinateCount==2) {
      iScreenX = dasherCoordinates[0];
      iScreenY = dasherCoordinates[1];
    } else {
      //Aiieee, we're receiving >2 coords? Don't know what to do...
      return false;
    }
    return true;
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

  //Reports an error by appending an error message obtained from strerror(errno) onto the provided prefix
  void ReportErrnoError(const std::string &prefix);

  virtual void SocketDebugMsg(const char *pszFormat, ...);
  
  CMessageDisplay *const m_pMsgs;

};
}
/// \}
#endif



