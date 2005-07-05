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
    struct timeb timebuffer;

    ftime(&timebuffer);

    m_startMs       = timebuffer.millitm;
    m_startSecond   = timebuffer.time;
}

CSimpleTimer::~CSimpleTimer()
{
}

double CSimpleTimer::GetElapsed()
{
    struct timeb timebuffer;

    ftime(&timebuffer);

    int     endMs       = timebuffer.millitm;
    int     endSecond   = timebuffer.time;

    return  ((double) endMs     / 1000.0 + (double) endSecond) - 
            ((double) m_startMs / 1000.0 + (double) m_startSecond);

}

