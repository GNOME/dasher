#include "SimpleTimer.h"

#include <sys/timeb.h>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CSimpleTimer::CSimpleTimer()
{
  struct timeb sTimeBuffer;

  ftime(&sTimeBuffer);

  m_iStartMs       = sTimeBuffer.millitm;
  m_iStartSecond   = sTimeBuffer.time;
}

CSimpleTimer::~CSimpleTimer()
{
}

double CSimpleTimer::GetElapsed()
{
  struct timeb sTimeBuffer;

  ftime(&sTimeBuffer);

  int     iEndMs       = sTimeBuffer.millitm;
  int     iEndSecond   = sTimeBuffer.time;

  return  ((double) iEndMs     / 1000.0 + (double) iEndSecond) - 
          ((double) m_iStartMs / 1000.0 + (double) m_iStartSecond);

}

