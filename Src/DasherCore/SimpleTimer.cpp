
#include "../Common/Common.h"

#include "SimpleTimer.h"

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

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
#ifdef _WIN32
  struct timeb sTimeBuffer;
#else
  struct timeval sTimeBuffer;
  struct timezone sTimezoneBuffer;
#endif

#ifdef _WIN32
    ftime(&sTimeBuffer);
    m_iStartMs       = sTimeBuffer.millitm;
    m_iStartSecond   = sTimeBuffer.time;
#else
    gettimeofday(&sTimeBuffer, &sTimezoneBuffer);
    m_iStartMs       = (int)sTimeBuffer.tv_usec / 1000;
    m_iStartSecond   = (int)sTimeBuffer.tv_sec;
#endif

}

CSimpleTimer::~CSimpleTimer()
{
}

double CSimpleTimer::GetElapsed()
{
#ifdef _WIN32
  struct timeb sTimeBuffer;
#else
  struct timeval sTimeBuffer;
  struct timezone sTimezoneBuffer;
#endif

#ifdef _WIN32
  ftime(&sTimeBuffer);
  int     iEndMs       = sTimeBuffer.millitm;
  int     iEndSecond   = sTimeBuffer.time;
#else
  gettimeofday(&sTimeBuffer, &sTimezoneBuffer);
  int     iEndMs       = (int)sTimeBuffer.tv_usec / 1000;
  int     iEndSecond   = (int)sTimeBuffer.tv_sec;
#endif


  return  ((double) iEndMs     / 1000.0 + (double) iEndSecond) - 
          ((double) m_iStartMs / 1000.0 + (double) m_iStartSecond);

}

