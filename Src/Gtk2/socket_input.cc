// socket_input.cc
//
// (C) Copyright Seb Wills 2005

#include "socket_input.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "../DasherCore/Event.h"

using namespace Dasher;

CDasherSocketInput::CDasherSocketInput(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore)
:CDasherComponent(pEventHandler, pSettingsStore) {

  debug_socket_input = false;
  readerRunning = false;
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
  SetPort(GetLongParameter(LP_SOCKET_PORT));
  SetRawRange(0, ((double)GetLongParameter(LP_SOCKET_INPUT_X_MIN)) / 1000.0, ((double)GetLongParameter(LP_SOCKET_INPUT_X_MAX)) / 1000.0);
  SetRawRange(1, ((double)GetLongParameter(LP_SOCKET_INPUT_Y_MIN)) / 1000.0, ((double)GetLongParameter(LP_SOCKET_INPUT_Y_MAX)) / 1000.0);
  SetCoordinateLabel(0, GetStringParameter(SP_SOCKET_INPUT_X_LABEL).c_str());
  SetCoordinateLabel(1, GetStringParameter(SP_SOCKET_INPUT_Y_LABEL).c_str());

  sock = -1;
}

void Dasher::CDasherSocketInput::HandleEvent(Dasher::CEvent *pEvent) {

  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
    case LP_SOCKET_PORT:
      SetPort(GetLongParameter(LP_SOCKET_PORT));
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

bool Dasher::CDasherSocketInput::StartListening() {
  struct sockaddr_in name;

  // this shouldn't be called if we are already listening, but if it is, let's failsafe
  // rather than attempt to bind to the same port twice.
  if(readerRunning) {
    StopListening();
  }

  if((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    return false;
  }

  name.sin_family = AF_INET;
  name.sin_port = htons(port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
    std::cerr << "Socket input: Error binding to port. errno=" << errno << std::endl;
    perror("bind");
    close(sock);
    sock = -1;
    return false;
  }

  if(pthread_create(&readerThread, NULL, ThreadLauncherStub, this) != 0) {
    cerr << "Socket input: Error creating reader thread" << endl;
    close(sock);
    sock = -1;
    return false;
  }

  readerRunning = true;
  return true;
}

void Dasher::CDasherSocketInput::StopListening() {

  if(!readerRunning) {
    return;
  }

  pthread_cancel(readerThread);

  if(sock >= 0) {
    close(sock);
  }
  readerRunning = false;
  if(debug_socket_input) {
    cerr << "Socket input: stopped listening to socket." << endl;
  }
}

void CDasherSocketInput::SetPort(int _port) {
  if(debug_socket_input) {
    cerr << "Setting socket input port to " << _port << endl;
  }
  if(readerRunning) {
    StopListening();
    port = _port;
    StartListening();
  }
  else {
    port = _port;
  }
}

void CDasherSocketInput::SetCoordinateLabel( int iWhichCoordinate, const char *Label) {
  DASHER_ASSERT(iWhichCoordinate < DASHER_SOCKET_INPUT_MAX_COORDINATE_COUNT);
  if(strlen(Label) > DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH) {
    cerr << "Warning truncating socket input label '" << Label << "' to " << DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH << " characters." << endl;
  }
  strncpy(coordinateNames[iWhichCoordinate], Label, DASHER_SOCKET_INPUT_MAX_COORDINATE_LABEL_LENGTH);
  if(debug_socket_input) {
    cerr << "Socket input: set coordinate " << iWhichCoordinate << " label to '" << coordinateNames[iWhichCoordinate] << "'." << endl;
  }
}


// private methods:

void *CDasherSocketInput::ReadForever() {
  // this gets called in its own thread. It reads datagrams and updates the coordinate variables

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);     // kill this thread immediately if another thread cancels it
  // don't know how this interacts with recv blocking

  int numbytes;
  while(sock >= 0) {
    if(debug_socket_input) {
      std::cerr << "Reading from socket..." << std::endl;
    }
    if((numbytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) == -1) {
      std::cerr << "Socket input: Error reading from socket" << std::endl;
      continue;
    }
    buffer[numbytes] = '\0';

    if(debug_socket_input) {
      std::cerr << " received string: '" << buffer << "'." << std::endl;
    }

    ParseMessage(buffer);

  }
}

// Parse and act on a message received from the socket
// Allowed to modify contents of memory pointed to by message, up to its final '\0'.
bool CDasherSocketInput::ParseMessage(char *message) {

  char *p;
  double rawdouble;
  myint dasherCoordinateTemp;
  // parse line by line
  while((p = index(message, '\n')) != NULL) {
    *p = '\0';
    // Each line is expected to be of the form "Label <value>"
    // We run through each coordinate label, checking if this line matches it
    for(int i = 0; i < coordinateCount; i++) {
      int len = strlen(coordinateNames[i]);
      if(strncmp(coordinateNames[i], message, len) == 0) {
	if(debug_socket_input) {
	  cerr << "Matched label '" << coordinateNames[i]; // message completed later
	}
	// First len chars match the label of this coordinate. Value should be at the next non-space char.
	if(sscanf(message + len, "%lf", &rawdouble) == 1) {
	  if(debug_socket_input) {
	    cerr << "', parsed value as " << rawdouble << "." << endl;
	  }

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
            dasherCoordinates[i] = (myint) ((rawdouble - rawMinValues[i]) / (rawMaxValues[i] - rawMinValues[i]) * (double)dasherMaxCoordinateValues[i]);
          }

	  if(debug_socket_input) {
	    cerr << "Socket input: new value for coordinate " << i << " rescales to " << dasherCoordinates[i] << " in dasher's internal coordinates (range 0-" << dasherMaxCoordinateValues[i] << ")." << endl;
	  }

          // don't break out of the for loop in case we get asked to drive two coordinates from same label
        } else {
	  if(debug_socket_input) {
	    cerr << ", but couldn't parse the text following that label as a number." << endl;
	  }
	}
      }
    }

    message = p + 1;            // move on to next line (if there isn't one, we'll point at the terminating '\0')
  }
}
