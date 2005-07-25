
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
bool CUserLogParam::ComparePtr(CUserLogParam* pA, CUserLogParam* pB)
{
  if ((pA == NULL) || (pB == NULL))
    return false;
  int iResult = pA->strName.compare(pB->strName);

  if (iResult < 0)
    return true;
  else if (iResult == 0)
  {
    if (pA->strTimeStamp.compare(pB->strTimeStamp) < 0)
      return true;
  }

  return false;
}


