// toshtilt.cpp
//
// Reads from the accelerometers in the Toshiba Portege m200 tablet PC,
// and emits UDP packets intended to driver Dasher's socket input.
//
// Seb Wills, November 2005. <saw27@mrao.cam.ac.uk>
// Hereby placed in the public domain.
//
// Many thanks to Larry O'Brien for reverse-engineering how to read from the
// accelerometers, documented at http://www.devx.com/TabletPC/Article/28483
//
// A Visual Studio 7 "solution" file is included with this file, but
// if you can't get it to load, the project setup was quite simple. I
// selected a Win32 Console application using the wizard. This file is
// the only one I modified. I also had to add ws2_32.lib to the linker
// dependencies (in the Project properties pages).

#include "../Common/Common.h"

#include "stdafx.h"
#include "windows.h"
#include "winsock.h"
#include "math.h"

int getTilt(int *x, int *y);

// global:
typedef INT (CALLBACK* LPFNDLLFUNC1)(INT*,INT*);
LPFNDLLFUNC1 lpfnGetImmediate;    // Function pointer


int _tmain(int argc, _TCHAR* argv[])
{


HINSTANCE hDLL;               // Handle to DLL
INT irawx, irawy;

// Set up UDP socket for writing to Dasher...

WORD wVersionRequested = MAKEWORD(1,1);
WSADATA wsaData;
int nRet;
unsigned short int nPort=20320;
SOCKET sock;

nRet = WSAStartup(wVersionRequested, &wsaData);
if(wsaData.wVersion != wVersionRequested) {
	fprintf(stderr, "Wrong winsock version\n");
	return(1);
}

sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
if(sock == INVALID_SOCKET) {
	perror("socket()");
	return(1);
}

SOCKADDR_IN saRemote;

/* This is how you would do it if Dasher on a different machine:
LPHOSTENT lpHostEntry;
lpHostEntry = gethostbyname("localhost");
if(lpHostEntry == NULL) {
	perror("gethostbyname()");
	return(1);
}
*/

saRemote.sin_family = AF_INET;
/* If you used gethostbyname section above:
//saRemote.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
*/
//otherwise:
saRemote.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
saRemote.sin_port = htons(nPort);


// Explicitly link to the Toshiba DLL...

hDLL = LoadLibrary("C:\\Program Files\\Toshiba\\Acceleration Utilities\\TAcelMgr\\TAcelMgr.dll");
if (hDLL == NULL) {
    printf("Failed to load Tosbiba Acceleration Utilities DLL, TAcelMgr.dll\n");
	return -1;
}
lpfnGetImmediate = (LPFNDLLFUNC1)GetProcAddress(hDLL,
                                           "GetImmediate");
if (!lpfnGetImmediate)
{
   // handle the error
   FreeLibrary(hDLL);
   printf("Failed to get pointer to function in DLL\n");
   return -2;
}


/***** CALIBRATE *******/

INT minx,maxx,miny,maxy,mindummy,maxdummy;
printf("Please tilt tablet LEFT...\n");
Sleep(2500);
getTilt(&minx,&mindummy);
printf("Please tilt tablet RIGHT...\n");
Sleep(1500);
getTilt(&maxx,&maxdummy);
bool rotated=false;
if(fabs(((double)maxx-(double)minx)) < fabs(((double)maxdummy-(double)mindummy))) {
	printf("(ok, device is rotated!)\n");
	minx=mindummy;
	maxx=maxdummy;
	rotated=true;
}
printf("Please tilt tablet UP...\n");
Sleep(1500);
getTilt(&mindummy,&miny);
printf("Please tilt table DOWN...\n");
Sleep(1500);
getTilt(&maxdummy,&maxy);
if(rotated) {
	miny=mindummy;
	maxy=maxdummy;
}

printf("\nThank you.\n x range %d to %d\n y range %d to %d\n\n", minx, maxx, miny, maxy);

printf("\nStandby...\n");
Sleep(2000);	  

double centrex = ((double)maxx+(double)minx)/2.0;
double centrey = ((double)maxy+(double)miny)/2.0;
double halfrangex = (((double)maxx-(double)minx))/2.0;
double halfrangey = (((double)maxy-(double)miny))/2.0;

char buffer[256];
while(1) {
	// call the function
	getTilt(&irawx, &irawy);
	if(rotated) {
		INT temp=irawy;
		irawy=irawx;
		irawx=temp;
	}
	//ix=irawx-3435921935; // these values seem to be for horizontal
	//iy=irawy-3435921929;
	double dx, dy; // "d" here means "double", not differentiation!
	//dx=((double)irawx- ((double)minx + halfrangex))/halfrangex;
	//dy=((double)irawy- ((double)miny + halfrangey))/halfrangey;
	dx=((double)irawx- centrex)/halfrangex;
	dy=((double)irawy- centrey)/halfrangey;
	
	//printf("x=%lu -> %ld, y=%lu -> %ld\n", irawx, ix, irawy, iy);
	printf("x=%d -> %lf, y=%d -> %lf\n", irawx, dx, irawy, dy);
    // Send datagram to Dasher:
	sprintf(buffer, "x %lf\ny %lf\n", dx, dy);
	nRet = sendto(sock, buffer, strlen(buffer), 0, (LPSOCKADDR)&saRemote, sizeof(struct sockaddr));
	if(nRet == SOCKET_ERROR) {
		perror("sendto()");
		WSAGetLastError();
	}
	Sleep(50);
}

return 0;
}

int getTilt(INT *x, INT *y) {
	int res;
	res=lpfnGetImmediate(x,y);
	// only lower 16 bits seem to be meaningful
	// (the upper two bytes did strange things for me: consistantly 0xcccc in the Debug build, but flipping around between various things in Release build)
	*x &= 0xffff;
    *y &= 0xffff;
	return res;
}
