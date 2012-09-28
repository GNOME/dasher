
#include "../Common/Common.h"

#ifdef _WIN32

#include "MemoryLeak.h"

void EnableLeakDetection( void )
{
#ifdef _DEBUG
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
}

// Set the output to a file instead of the output window in Visual Studio
void SetOutputLeaksToFile()
{
#ifdef _DEBUG
  HANDLE hLogFile;

  hLogFile = CreateFile(TEXT("memory_leak.log"), 
    GENERIC_WRITE, 
    FILE_SHARE_WRITE, 
    NULL, 
    CREATE_ALWAYS, 
    FILE_ATTRIBUTE_NORMAL, 
    NULL);

  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, hLogFile);
  _RPT0(_CRT_WARN,"file message\n");

#endif
}

#endif
