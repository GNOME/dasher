
#include "../Common/Common.h"

#include "SimpleTimer.h"

#include <sys/time.h>

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
  struct timeval sTimeBuffer;
  struct timezone sTimezoneBuffer;

  gettimeofday(&sTimeBuffer, &sTimezoneBuffer);

  m_iStartMs       = sTimeBuffer.tv_usec / 1000;
  m_iStartSecond   = sTimeBuffer.tv_sec;
}

CSimpleTimer::~CSimpleTimer()
{
}

double CSimpleTimer::GetElapsed()
{
  struct timeval sTimeBuffer;
  struct timezone sTimezoneBuffer;

  gettimeofday(&sTimeBuffer, &sTimezoneBuffer);

  int     iEndMs       = sTimeBuffer.tv_usec / 1000;
  int     iEndSecond   = sTimeBuffer.tv_sec;

  return  ((double) iEndMs     / 1000.0 + (double) iEndSecond) - 
          ((double) m_iStartMs / 1000.0 + (double) m_iStartSecond);

}

