// SocketInputBase.cpp
//
// (C) Copyright Seb Wills 2005

#include "../Common/Common.h"

#include "SocketInputBase.h"

#include "../DasherCore/Event.h"

#include <string.h>
#include <errno.h>
#include <stdarg.h>
#ifdef WIN32
#include <winsock2.h>
#define DASHER_SOCKET_CLOSE_FUNCTION closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define DASHER_SOCKET_CLOSE_FUNCTION close
#endif

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_SOCKET_PORT, T_LONGSPIN, 0, 65535, 1, 10, _("Port:")},
  {SP_SOCKET_INPUT_X_LABEL, T_STRING, -1, -1, -1, -1, _("X label:")},
  {LP_SOCKET_INPUT_X_MIN, T_LONGSPIN, -2147480000, 2147480000, 1000, 10000, _("X minimum:")},
  {LP_SOCKET_INPUT_X_MAX, T_LONGSPIN, -2147480000, 2147480000, 1000, 10000, _("X maximum:")},
  {SP_SOCKET_INPUT_Y_LABEL, T_STRING, -1, -1, -1, -1, _("Y label:")},
  {LP_SOCKET_INPUT_Y_MIN, T_LONGSPIN, -2147480000, 2147480000, 1000, 10000, _("Y minimum:")},
  {LP_SOCKET_INPUT_Y_MAX, T_LONGSPIN, -2147480000, 2147480000, 1000, 10000, _("Y maximum:")},
  {BP_SOCKET_DEBUG, T_BOOL, -1, -1, -1, -1, _("Print socket-related debugging information to console:")}
};

Dasher::CSocketInputBase::CSocketInputBase(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
  : CDasherInput(pEventHandler, pSettingsStore, 1, 0, _("Socket Input")) {
  port = -1;
  debug_socket_input = false;
  readerRunning = false;
  sock = -1;
  SetCoordinateCount(2);
  for(int i = 0; i < DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT; i++) {
    dasherMaxCoordinateValues[i] = 4096;        // the real value will come later when SetMaxCoordinates is invoked
    rawMinValues[i] = 0.0;      // suitable defaults for BCI2000
    rawMaxValues[i] = 512.0;
    memset(coordinateNames[i], '\0', DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH + 1);
    dasherCoordinates[i] = 2048; // initialise to mid-range value
  }

  // initialise using parameter settings:
  SetDebug(GetBoolParameter(BP_SOCKET_DEBUG));
  SetReaderPort(GetLongParameter(LP_SOCKET_PORT));
  SetRawRange(0, ((double)GetLongParameter(LP_SOCKET_INPUT_X_MIN)) / 1000.0, ((double)GetLongParameter(LP_SOCKET_INPUT_X_MAX)) / 1000.0);
  SetRawRange(1, ((double)GetLongParameter(LP_SOCKET_INPUT_Y_MIN)) / 1000.0, ((double)GetLongParameter(LP_SOCKET_INPUT_Y_MAX)) / 1000.0);
  SetCoordinateLabel(0, GetStringParameter(SP_SOCKET_INPUT_X_LABEL).c_str());
  SetCoordinateLabel(1, GetStringParameter(SP_SOCKET_INPUT_Y_LABEL).c_str());
  SocketDebugMsg("Socket input is initialised but not yet enabled");
}

Dasher::CSocketInputBase::~CSocketInputBase() {
  //  Would like to call StopListening(); Can't do this here because by the time this (base class) destructor is called,
  // the derived class instance has been deleted, so we can no longer call it's StopListening.
  // Instead, you should call StopListening in the derived class's destructor.
}

void Dasher::CSocketInputBase::HandleEvent(Dasher::CEvent *pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
    case LP_SOCKET_PORT:
      SetReaderPort(GetLongParameter(LP_SOCKET_PORT));
      break;
    case SP_SOCKET_INPUT_X_LABEL:
      SetCoordinateLabel(0, GetStringParameter(SP_SOCKET_INPUT_X_LABEL).c_str());
      break;
    case SP_SOCKET_INPUT_Y_LABEL:
      SetCoordinateLabel(1, GetStringParameter(SP_SOCKET_INPUT_Y_LABEL).c_str());
      break;
    case LP_SOCKET_INPUT_X_MIN:
    case LP_SOCKET_INPUT_X_MAX:
      SetRawRange(0, ((double)GetLongParameter(LP_SOCKET_INPUT_X_MIN)) / 1000.0, ((double)GetLongParameter(LP_SOCKET_INPUT_X_MAX)) / 1000.0);
      break;
    case LP_SOCKET_INPUT_Y_MIN:
    case LP_SOCKET_INPUT_Y_MAX:
      SetRawRange(1, ((double)GetLongParameter(LP_SOCKET_INPUT_Y_MIN)) / 1000.0, ((double)GetLongParameter(LP_SOCKET_INPUT_Y_MAX)) / 1000.0);
      break;
    case BP_SOCKET_DEBUG:
      SetDebug(GetBoolParameter(BP_SOCKET_DEBUG));
      break;
    default:
      break;
    }
  }
}

bool Dasher::CSocketInputBase::StartListening() {
  struct sockaddr_in name;

  // this shouldn't be called if we are already listening, but if it is, let's failsafe
  // rather than attempt to bind to the same port twice.
  if(readerRunning) {
    StopListening();
  }

  SocketDebugMsg("Socket input: binding to socket and starting to listen.");

  if((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
    ReportErrnoError("Error creating socket"); //FIXME should use _( (gettext), but need something cross-platform.
    // Could have a member functino, overridden in the platform-specific subclasses, which returns the localised text
    return false;
  }

  name.sin_family = AF_INET;
  name.sin_port = htons(port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
    ReportErrnoError("Error binding to socket - already in use?");
    DASHER_SOCKET_CLOSE_FUNCTION(sock);
    sock = -1;
    return false;
  }

  if(!LaunchReaderThread()) {
    // LaunchReaderThread will already have displayed an error message
    DASHER_SOCKET_CLOSE_FUNCTION(sock);
    sock = -1;
    return false;
  }

  readerRunning = true;
  return true;
}


void Dasher::CSocketInputBase::StopListening() {

  if(!readerRunning) {
    return;
  }

  CancelReaderThread();

  if(sock >= 0) {
    DASHER_SOCKET_CLOSE_FUNCTION(sock);
  }
  readerRunning = false;
  SocketDebugMsg("Socket input: stopped listening to socket.");
}

void CSocketInputBase::SetReaderPort(int _port) {
  if(_port == port) {
    SocketDebugMsg("SetReaderPort called with same value (%d), so ignoring.",port);
    return;
  }

  SocketDebugMsg("Setting socket input port to %d.", _port);
  if(readerRunning) {
    StopListening();
    port = _port;
    StartListening();
  }
  else {
    port = _port;
  }
}

void CSocketInputBase::SetCoordinateLabel( int iWhichCoordinate, const char *Label) {
  DASHER_ASSERT(iWhichCoordinate < DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT);
  if(strlen(Label) > DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH) {
    cerr << "Warning truncating socket input label '" << Label << "' to " << DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH << " characters." << endl;
  }
  strncpy(coordinateNames[iWhichCoordinate], Label, DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH);
  SocketDebugMsg("Socket input: set coordinate %d label to '%s'.", iWhichCoordinate,  coordinateNames[iWhichCoordinate]);
}

void CSocketInputBase::SetRawRange(int iWhich, double dMin, double dMax) {
  rawMinValues[iWhich] = dMin;
  rawMaxValues[iWhich] = dMax;
  SocketDebugMsg("Socket input: set coordinate %d input range to: min: %lf, max: %lf.", iWhich, dMin, dMax);
}


// private methods:

void CSocketInputBase::ReadForever() {
  // this gets called in its own thread. It reads datagrams and updates the coordinate variables

  int numbytes;
  while(sock >= 0) {
    SocketDebugMsg("Reading from socket...");
    if((numbytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) == -1) {
      ReportError("Socket input: Error reading from socket");
      continue;
    }
    buffer[numbytes] = '\0';

    SocketDebugMsg(" received string: '%s'.", buffer);

    ParseMessage(buffer);

  }
}

// Parse and act on a message received from the socket
// Allowed to modify contents of memory pointed to by message, up to its final '\0'.
void CSocketInputBase::ParseMessage(char *message) {

  char *p;
  double rawdouble;
 // myint dasherCoordinateTemp;
  // parse line by line
  while((p = strchr(message, '\n')) != NULL) {
    *p = '\0';
    // Each line is expected to be of the form "Label <value>"
    // We run through each coordinate label, checking if this line matches it
    for(int i = 0; i < coordinateCount; i++) {
      int len = strlen(coordinateNames[i]);
      if(strncmp(coordinateNames[i], message, len) == 0) {
        SocketDebugMsg("Matched label '%s'...", coordinateNames[i]);
        // First len chars match the label of this coordinate. Value should be at the next non-space char.
        if(sscanf(message + len, "%lf", &rawdouble) == 1) {
          SocketDebugMsg("...parsed value as %lf.", rawdouble);

#ifdef DASHER_SOCKET_INPUT_BCI2000_OVERFLOW_WORKAROUND
          // a temporary workaround to undo an integer overflow that occurs in messages sent from BCI2000
          if(rawdouble > 32000) {
            rawdouble = 0;
          }
          if(rawdouble > 768 && rawdouble < 32000) {
            rawdouble = 768;
          }
#endif

          // Clipping:
          // for clipping purposes, we want to ignore whether Max < Min (which indicates that
          // we need to flip the sense of the input)
          double actualMax = (rawMaxValues[i] > rawMinValues[i]) ? rawMaxValues[i] : rawMinValues[i];
          double actualMin = (rawMaxValues[i] > rawMinValues[i]) ? rawMinValues[i] : rawMaxValues[i];
          if(rawdouble < actualMin) {
            cerr << "Socket input: clipped " << coordinateNames[i] << " value of " << rawdouble << "to configured minimum of " << actualMin << endl;
            rawdouble = actualMin;
          }
          if(rawdouble > actualMax) {
            cerr << "Socket input: clipped " << message << " value of " << rawdouble << "to configured maximum of " << actualMax << endl;
            rawdouble = actualMax;
          }

          // convert to dasher coordinates:
          
          const bool do_lowpass = false;
          if(do_lowpass) {
            // initial attempt at putting a low-pass filter in. Not well tested; disabled for now.
            double timeconst = 100.0;   // no of updates
            double newcoord = ((rawdouble - rawMinValues[i]) / (rawMaxValues[i] - rawMinValues[i]) * dasherMaxCoordinateValues[i]);
            dasherCoordinates[i] = (myint) ((1 - 1 / timeconst) * (double)dasherCoordinates[i] + (1 / timeconst) * newcoord);
          }
          else {
            // straightforward linear mapping to dasher coordinates:
	    // Treat X coordinate specially: reverse sense so it has the more intuitive left-to-right direction
	    double min = (i==0) ? rawMaxValues[i] : rawMinValues[i];
	    double max = (i==0) ? rawMinValues[i] : rawMaxValues[i];
            if(max != min) { // prevent nasty explosion
              dasherCoordinates[i] = (myint) ((rawdouble - min) / (max - min) * (double)dasherMaxCoordinateValues[i]);
            }
          }

          SocketDebugMsg("Socket input: new value for coordinate %d rescales to %u in Dasher's internal coordinates (range 0-%d).", i, (unsigned int) dasherCoordinates[i], (int) dasherMaxCoordinateValues[i]);

          // don't break out of the for loop in case we get asked to drive two coordinates from same label
        } else {
          SocketDebugMsg("... but couldn't parse the text following that label as a number.");
        }
      }
    }

    message = p + 1;            // move on to next line (if there isn't one, we'll point at the terminating '\0')
  }
}

void CSocketInputBase::SetDebug(bool _debug) {
  if(!_debug) {
    SocketDebugMsg("Disabling socket debug messages.");
  }
  debug_socket_input = _debug;
  if(_debug) {
    SocketDebugMsg("Enabled socket debug messages.");
  }
}

void CSocketInputBase::ReportError(std::string s) {
  // override this to pop up a message box etc.
  cerr << s << endl;
}

void CSocketInputBase::ReportErrnoError(std::string prefix) {
  // override this to pop up a message box
  cerr << "Dasher Socket Input error: ";
  perror(prefix.c_str());
}

void CSocketInputBase::SocketDebugMsg(const char *pszFormat, ...) {
  if(debug_socket_input) {
    va_list v;
    va_start(v, pszFormat);
    vfprintf(stderr, pszFormat, v);
    fprintf(stderr, "\n");
    va_end(v);
  }
}

bool CSocketInputBase::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};
