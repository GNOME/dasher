
#include "UserLogParam.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// Needed so we can sort() vectors of parameters
bool CUserLogParam::ComparePtr(CUserLogParam* a, CUserLogParam* b)
{
    if ((a == NULL) || (b == NULL))
        return false;
    int result = a->strName.compare(b->strName);

    if (result < 0)
        return true;
    else if (result == 0)
    {
        if (a->strTimeStamp.compare(b->strTimeStamp) < 0)
            return true;
    }

    return false;
}


