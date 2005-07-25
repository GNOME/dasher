#include "SimpleTimer.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
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

