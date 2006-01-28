
#include "../Common/Common.h"

#include "UserLogParam.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
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


