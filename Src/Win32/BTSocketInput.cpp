#include "Common\WinCommon.h"

#include "BTSocketInput.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// Configuration optiond here:
static const char *szPort = "9999";
static const char *szHostname = "localhost";

static const int xmin = -32;
static const int xmax = 32;

static const int ymin = 32;
static const int ymax = -32;

// TODO: This doesn't do any error handling at all
// TODO: Probably incompatable with the socket server module

CBTSocketInput::CBTSocketInput()
: CDasherCoordInput(100, "BT Tilt Socket"){
}

CBTSocketInput::~CBTSocketInput(void) {
}

bool CBTSocketInput::GetDasherCoords(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
    // Send the magic command...
	const char *szCommand = "<message command=\"orientation\"></message>\r\n";
	int iRetVal = send(m_oSocket, szCommand, strlen(szCommand), 0);

	// And wait for the reply...
	char szBuffer[2048];
	int iBytes = recv(m_oSocket, szBuffer, 2047, 0);
	szBuffer[iBytes] = 0;

	char *szXStart = strstr(szBuffer, "x=\"") + 3;
	char *szXEnd = strchr(szXStart, '\"');
	
	char *szYStart = strstr(szBuffer, "y=\"") + 3;
	char *szYEnd = strchr(szXStart, '\"');

	*szXEnd = 0;
	*szYEnd = 0;

	int xrange = xmax - xmin;
	int yrange = ymax - ymin;

	iDasherX = (atoi(szXStart) - xmin) * 4096 / xrange;
	iDasherY = (atoi(szYStart) - ymin) * 4096 / yrange;

	return true;
}

void CBTSocketInput::Activate() {
	// Initialise Winsock
	WSADATA  wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	struct addrinfo sHints;
	ZeroMemory(&sHints, sizeof(sHints));
	sHints.ai_family = AF_UNSPEC;
	sHints.ai_socktype = SOCK_STREAM;
	sHints.ai_protocol = IPPROTO_TCP;

	struct addrinfo *sAddr;
	getaddrinfo(szHostname, szPort, &sHints, &sAddr);

	m_oSocket = socket(sAddr->ai_family, sAddr->ai_socktype, sAddr->ai_protocol);
	connect(m_oSocket, sAddr->ai_addr, (int)sAddr->ai_addrlen);
	freeaddrinfo(sAddr);

}

void CBTSocketInput::Deactivate() {
	shutdown(m_oSocket, SD_BOTH);
	closesocket(m_oSocket);
	WSACleanup();
}	
