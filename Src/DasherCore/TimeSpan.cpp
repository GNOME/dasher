
#include "../Common/Common.h"

#include <cstring>
#include "TimeSpan.h"

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CTimeSpan::CTimeSpan(const string& strName, bool bAddDate)
{
  InitMemberVars();

  m_strName       = strName;
  m_strStartTime  = GetTimeStamp();
  m_pTimer        = new CSimpleTimer();
  m_dElapsed      = 0.0;
  m_strEndTime    = "";
  m_strStartDate  = "";

  // A time span can optionally record the current date
  if (bAddDate)
    m_strStartDate = GetDateStamp();
}

CTimeSpan::~CTimeSpan()
{
  if (m_pTimer != NULL)
  {
    delete m_pTimer;
    m_pTimer = NULL;
  }
}

// Get the XML for this TimeSpan object.  If bSinglePointInTime is true, then 
// this is just a single point in time and we don't need the end time or 
// elapsed time.
string CTimeSpan::GetXML(const string& strPrefix, bool bSinglePointInTime)
{
  string strResult = "";

  // Only stop if we haven't called Stop() explicitly
  if (m_strEndTime.size() == 0)
    Stop();

  strResult += strPrefix;
  strResult += "<";
  strResult += m_strName;
  strResult += ">\n";

  if (!bSinglePointInTime)
  {
    strResult += strPrefix;
    strResult += "\t<Elapsed>";
    char strNum[256];
    sprintf(strNum, "%0.3f", m_dElapsed);
    strResult += strNum;
    strResult += "</Elapsed>\n";
  }

  if (m_strStartDate.length() > 0)
  {
    strResult += strPrefix;
    strResult += "\t<Date>";
    strResult += m_strStartDate;
    strResult += "</Date>\n";
  }

  if (!bSinglePointInTime)
  {
    strResult += strPrefix;
    strResult += "\t<Start>";
    strResult += m_strStartTime;
    strResult += "</Start>\n";

    strResult += strPrefix;
    strResult += "\t<End>";
    strResult += m_strEndTime;
    strResult += "</End>\n";
  }
  else
  {
    strResult += strPrefix;
    strResult += "\t<Time>";
    strResult += m_strStartTime;
    strResult += "</Time>\n";
  }

  strResult += strPrefix;
  strResult += "</";
  strResult += m_strName;
  strResult += ">\n";

  return strResult;
}

string CTimeSpan::GetTimeStamp()
{
  string strTimeStamp = "";
#ifdef _WIN32
    struct timeb sTimeBuffer;
#else
    struct timeval sTimeBuffer;
    struct timezone sTimezoneBuffer;
    time_t t;
#endif
    char* szTimeLine = NULL;

#ifdef _WIN32
    ftime(&sTimeBuffer);
    szTimeLine = ctime(&(sTimeBuffer.time));
#else
    gettimeofday(&sTimeBuffer, &sTimezoneBuffer);
    t = sTimeBuffer.tv_sec;
    szTimeLine = ctime(&t);
#endif
  
  if ((szTimeLine != NULL) && (strlen(szTimeLine) > 18))
  {
    for (int i = 11; i < 19; i++)
      strTimeStamp += szTimeLine[i];
    strTimeStamp += ".";
    char szMs[16];
#ifdef _WIN32
    sprintf(szMs, "%d", sTimeBuffer.millitm);
#else
    sprintf(szMs, "%d", static_cast<int>(sTimeBuffer.tv_usec / 1000));
#endif
    if (strlen(szMs) == 1)
      strTimeStamp += "00";
    else if (strlen(szMs) == 2)
      strTimeStamp += "0";
    strTimeStamp += szMs;
  }

  return strTimeStamp;
}

void CTimeSpan::Stop()
{
  // Only do this if we actually have a timer, if we were
  // created from XML then we don't want to change what 
  // we read from the file.
  if (m_pTimer != NULL)
  {
    m_strEndTime    = GetTimeStamp();
    m_dElapsed       = m_pTimer->GetElapsed();
  }
}

// We allow a time span to continue to erase the 
// effects of a previous Stop() call
void CTimeSpan::Continue()
{
  m_strEndTime = "";
  m_dElapsed = 0.0;
}

bool CTimeSpan::IsStopped()
{
  if (m_strEndTime.length() > 0)
    return true;

  return false;
}

double CTimeSpan::GetElapsed()
{
  return m_dElapsed;
}

string CTimeSpan::GetDateStamp()
{
  std::string strDateStamp = "";
  char* szTimeLine = NULL;
  time_t t;

  t = time(NULL);
  szTimeLine = ctime(&t);

  // Format is:
  // Wed Jun 22 10:22:00 2005
  // 0123456789012345678901234
  if ((szTimeLine != NULL) && (strlen(szTimeLine) > 23))
  {
    for (int i = 4; i < 10; i++)
      strDateStamp += szTimeLine[i];            
    for (int i = 19; i < 24; i++)
      strDateStamp += szTimeLine[i];            
  }

  return strDateStamp;
}

void CTimeSpan::InitMemberVars()
{
  m_pTimer        = NULL;
  m_strName       = "";
  m_strStartTime  = "";
  m_dElapsed      = 0.0;
  m_strEndTime    = "";
  m_strStartDate  = "";
}

// Construct based on some yummy XML like:
// 		<Elapsed>12.062</Elapsed>
//		<Date>Jul 04 2005</Date>
//		<Start>15:48:52.625</Start>
//		<End>15:49:04.687</End>
CTimeSpan::CTimeSpan(const string& strName, const string& strXML)
{
  InitMemberVars();

  m_dElapsed      = (double) XMLUtil::GetElementFloat("Elapsed", strXML);    

  m_strStartDate  = XMLUtil::GetElementString("Date", strXML);
  m_strStartTime  = XMLUtil::GetElementString("Start", strXML);
  m_strEndTime    = XMLUtil::GetElementString("End", strXML);

  m_strName       = strName;
}
