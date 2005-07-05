// Command line application that can read a UserLog XML file
// and rebuilds the C++ objects that represented it.  
//
// Copyright 2005 by Keith Vertanen
//

#include <string>
#include <vector>
using namespace std;

// Declare our global file logging object
#include "FileLogger.h"
#ifdef _DEBUG
	const eLogLevel gLogLevel   = logDEBUG;
    const int       gLogOptions = logTimeStamp | logDateStamp | logDeleteOldFile;    
#else
	const eLogLevel gLogLevel = logNORMAL;
    const int       gLogOptions = logTimeStamp | logDateStamp;
#endif
CFileLogger* gLogger = NULL;
#ifdef _WIN32

#include "UserLog.h"

// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
#ifdef _DEBUG
    // Windows debug build memory leak detection
	EnableLeakDetection();
#endif
#endif

    // Global logging object we can use from anywhere
	gLogger = new CFileLogger("UserLog.log",
								gLogLevel,		
                                gLogOptions);

	{ // memory leak scoping

        CUserLog objUserLog("test.xml");

        objUserLog.SetOuputFilename("new.xml");
        objUserLog.OutputFile();
    }

    if (gLogger != NULL)
	{
		delete gLogger;
		gLogger  = NULL;
	}

    return 0;
}