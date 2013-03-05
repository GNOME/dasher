
#include "../Common/Common.h"

#include <cstring>
#include "FileLogger.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

CFileLogger::CFileLogger(const std::string& strFilenamePath, eLogLevel iLogLevel, int iOptionsMask)
{
  m_strFilenamePath       = "";
  m_iLogLevel             = iLogLevel;
  m_iFunctionIndentLevel  = 0;

  m_bFunctionLogging      = false;
  m_bTimeStamp            = false;
  m_bDateStamp            = false;
  m_bDeleteOldFile        = false;
  m_bFunctionTiming       = false;
  m_bOutputScreen         = false;

  // See what options are set in our bit mask options parameter
  if (iOptionsMask & logFunctionEntryExit)
    m_bFunctionLogging = true;
  if (iOptionsMask & logTimeStamp)
    m_bTimeStamp = true;
  if (iOptionsMask & logDateStamp)
    m_bDateStamp = true;
  if (iOptionsMask & logDeleteOldFile)
    m_bDeleteOldFile = true;
  if (iOptionsMask & logFunctionTiming)
    m_bFunctionTiming = true;
  if (iOptionsMask & logOutputScreen)
    m_bOutputScreen = true;

  // On windows anyway if somebody can open up a file with CreateFile() in a different 
  // directory and cause the working directory to change.  We don't want our log file
  // moving around, so we'll find a absolute path when we are created and stick to
  // that for the remainder of our life.
  m_strFilenamePath = GetFullFilenamePath(strFilenamePath);

  // See if we should get rid of any existing filename with our given name.  This prevents having
  // to remember to delete the file before every new debugging run.
  if (m_bDeleteOldFile)
  {
    FILE* fp = NULL;

    fp = fopen(m_strFilenamePath.c_str(), "w");
    if (fp != NULL)
    {
      fclose(fp);
      fp = NULL;
    }
  }

}

CFileLogger::~CFileLogger()
{
#ifdef _WIN32
  if (m_bFunctionTiming)
  {
    // Dump the results of our function timing logging

    MAP_STRING_INT64::iterator map_iter;

    Log("%-60s%20s%10s", logNORMAL, "Function","Ticks", "Percent");
    Log("%-60s%20s%10s", logNORMAL, "--------","-----", "-------");

    __int64 iMaxTicks = 0;

    // First pass to count the max ticks 
    // We assume that there was a function logger on the outer most (main) program.
    // This allows the percent reflect the relative time spent inside emedded calls.

    for (map_iter = m_mapFunctionTicks.begin(); map_iter != m_mapFunctionTicks.end(); map_iter++)
    {
      if (map_iter->second > iMaxTicks)
        iMaxTicks = map_iter->second;
    }

    for (map_iter = m_mapFunctionTicks.begin(); map_iter != m_mapFunctionTicks.end(); map_iter++)
    {
      std::string name = map_iter->first;
      __int64 iTicks = map_iter->second;
      Log("%-60s%20I64Ld%10.2f", logNORMAL, name.c_str(), iTicks, (double) iTicks / (double) iMaxTicks  * (double) 100.0);
    }
  }
#endif
}

// Changes the filename of this logging object
void CFileLogger::SetFilename(const std::string& strFilename)
{
  m_strFilenamePath = strFilename;

  // See if we should get rid of any existing filename with our given name.  This prevents having
  // to remember to delete the file before every new debugging run.
  if (m_bDeleteOldFile)
  {
    FILE* fp = NULL;

    fp = fopen(m_strFilenamePath.c_str(), "w");
    if (fp != NULL)
    {
      fclose(fp);
      fp = NULL;
    }
  }
}

// Logs a string to our file.  eLogLevel specifies the importance of this message, we
// only write to the log file if it is at least as important as the level set in the 
// constructor.  Accepts printf style formatting in the first string which must be 
// filled with the variable parameter list at the end.
// NOTE: Currently not thread safe!
void CFileLogger::Log(const char* szText, eLogLevel iLogLevel, ...)
{
  va_list       args;  

  if ((m_strFilenamePath.length() > 0) && 
      (m_iLogLevel <= iLogLevel) && 
      (szText != NULL))
  {
    FILE* fp = NULL;
    fp = fopen(m_strFilenamePath.c_str(), "a");

    std::string strTimeStamp = GetTimeDateStamp();

    if (fp != NULL)
    {
      std::string strIndented = strTimeStamp + GetIndentedString(szText) + "\n";

      va_start(args, iLogLevel);
      vfprintf(fp, strIndented.c_str(), args);
      va_end(args);

      // Optionally we can output message to stdout
      if (m_bOutputScreen)
      {
        va_start(args, iLogLevel);
        vprintf(strIndented.c_str(), args);
        va_end(args);
      }

      fclose(fp);
      fp = NULL;
    }
  }
}

// Overloaded version that takes a STL::string
void CFileLogger::Log(const std::string strText, eLogLevel iLogLevel, ...)
{
  va_list       args;  

  if ((m_strFilenamePath.length() > 0) && 
      (m_iLogLevel <= iLogLevel))
  {
    FILE* fp = NULL;
    fp = fopen(m_strFilenamePath.c_str(), "a");

    std::string strTimeStamp = GetTimeDateStamp();

    if (fp != NULL)
    {
      std::string strIndented = strTimeStamp + GetIndentedString(strText) + "\n";

      va_start(args, iLogLevel);
      vfprintf(fp, strIndented.c_str(), args);
      va_end(args);

      // Optionally we can output message to stdout
      if (m_bOutputScreen)
      {
        va_start(args, iLogLevel);
        vprintf(strIndented.c_str(), args);
        va_end(args);
      }

      fclose(fp);
      fp = NULL;
    }
  }
}

// Version that assume log level is logDEBUG
void CFileLogger::LogDebug(const char* szText, ...)
{
  // Note: it would be nice not to duplicate code, but the variable
  // parameter list makes this problematic.
  va_list       args;  

  if ((m_strFilenamePath.length() > 0) && 
      (m_iLogLevel == logDEBUG) &&
      (szText != NULL))
  {
    FILE* fp = NULL;
    fp = fopen(m_strFilenamePath.c_str(), "a");

    std::string strTimeStamp = GetTimeDateStamp();

    if (fp != NULL)
    {
      std::string strIndented = strTimeStamp + GetIndentedString(szText) + "\n";

      va_start(args, szText);
      vfprintf(fp, strIndented.c_str(), args);
      va_end(args);

      // Optionally we can output message to stdout
      if (m_bOutputScreen)
      {
        va_start(args, szText);
        vprintf(strIndented.c_str(), args);
        va_end(args);
      }

      fclose(fp);
      fp = NULL;
    }
  }
}

// Version that assume log level is logNormal
void CFileLogger::LogNormal(const char* szText, ...)
{
  // Note: it would be nice not to duplicate code, but the variable
  // parameter list makes this problematic.
  va_list       args;  

  if ((m_strFilenamePath.length() > 0) && 
      (m_iLogLevel <= logNORMAL) &&
      (szText != NULL))
  {
    FILE* fp = NULL;
    fp = fopen(m_strFilenamePath.c_str(), "a");

    std::string strTimeStamp = GetTimeDateStamp();

    if (fp != NULL)
    {
      std::string strIndented = strTimeStamp + GetIndentedString(szText) + "\n";

      va_start(args, szText);
      vfprintf(fp, strIndented.c_str(), args);
      va_end(args);

      // Optionally we can output message to stdout
      if (m_bOutputScreen)
      {
        va_start(args, szText);
        vprintf(strIndented.c_str(), args);
        va_end(args);
      }

      fclose(fp);
      fp = NULL;
    }
  }
}

// Version that assume log level is logCRITICAL
void CFileLogger::LogCritical(const char* szText, ...)
{
  // Note: it would be nice not to duplicate code, but the variable
  // parameter list makes this problematic.
  va_list       args;  

  // Always log critical messages
  if ((m_strFilenamePath.length() > 0) &&
      (szText != NULL))
  {
    FILE* fp = NULL;
    fp = fopen(m_strFilenamePath.c_str(), "a");

    std::string strTimeStamp = GetTimeDateStamp();

    if (fp != NULL)
    {
      std::string strIndented = strTimeStamp + GetIndentedString(szText) + "\n";

      va_start(args, szText);
      vfprintf(fp, strIndented.c_str(), args);
      va_end(args);

      // Optionally we can output message to stdout
      if (m_bOutputScreen)
      {
        va_start(args, szText);
        vprintf(strIndented.c_str(), args);
        va_end(args);
      }

      fclose(fp);
      fp = NULL;
    }
  }
}

// Logs entry into a particular function
void CFileLogger::LogFunctionEntry(const std::string& strFunctionName)
{
  if (m_bFunctionLogging)
  {
    std::string strStart = "start: ";
    strStart += strFunctionName;
    Log(strStart.c_str());
    m_iFunctionIndentLevel++;
  }
}

// Logs exit into a particular function
void CFileLogger::LogFunctionExit(const std::string& strFunctionName)
{
  if (m_bFunctionLogging)
  {
    m_iFunctionIndentLevel--;
    std::string strEnd = "end: ";
    strEnd += strFunctionName;
    Log(strEnd.c_str());
  }
}

#ifdef _WIN32
void CFileLogger::LogFunctionTicks(const std::string& strFunctionName, __int64 iTicks)
{
  __int64 iCurrent;

  iCurrent = m_mapFunctionTicks[strFunctionName];
  iCurrent = iCurrent + iTicks;

  m_mapFunctionTicks[strFunctionName] = iCurrent;
}
#endif

// Gets an indented version of the function name 
std::string CFileLogger::GetIndentedString(const std::string& strText)
{
  std::string strIndented = "";
  for (int i = 0; i < m_iFunctionIndentLevel; i++)
    strIndented += " ";
  strIndented += strText;

  return strIndented;
}

bool CFileLogger::GetFunctionTiming()
{
  return m_bFunctionTiming;
}

// Utility method that converts a filename into a fully qualified
// path and filename on Windows.  This can be used to make sure
// a relative filename stays pointed at the same location despite
// changes in the working directory.
std::string CFileLogger::GetFullFilenamePath(std::string strFilename)
{
#ifdef _WIN32    

  // Windows code
  const int   MAX_PATH_LENGTH = 1024;

#ifdef _UNICODE

  // In Unicode, we need the parameters to GetFullPathName() in wide characters
  wchar_t     szPath[MAX_PATH_LENGTH];
  wchar_t*    pszFilePart = NULL;
  wchar_t     wstrFilenamePath[MAX_PATH_LENGTH];
  char        szResult[MAX_PATH_LENGTH];

  unsigned int i = 0;
  for (i = 0; i < strFilename.length(); i++)
    wstrFilenamePath[i] = (wchar_t) strFilename[i];
  wstrFilenamePath[i] = '\0';

  ::GetFullPathName(wstrFilenamePath, MAX_PATH_LENGTH, szPath, &pszFilePart);
  i = 0;
  while (szPath[i] != '\0')
  {
    szResult[i] = (char) szPath[i];
    i++;
  }
  szResult[i] = '\0';

  return szResult;

#else
  // Using normal non-unicode strings
  char        szPath[MAX_PATH_LENGTH];
  char*       pszFilePart = NULL;

  ::GetFullPathName(strFilename.c_str(), MAX_PATH_LENGTH, szPath, &pszFilePart);

  return szPath;

#endif

#else
  // Non-windows code
  return strFilename;

#endif
}

/////////////////////////////////////// CFunctionLogger /////////////////////////////////////////////////////////////

CFunctionLogger::CFunctionLogger(const std::string& strFunctionName, CFileLogger* pLogger) 
{
  m_pLogger = pLogger;

  if ((m_pLogger != NULL) && (strFunctionName.length() > 0))
  {
    m_strFunctionName = strFunctionName;

    if (!m_pLogger->GetFunctionTiming())
      m_pLogger->LogFunctionEntry(m_strFunctionName);
    else {
#ifdef _WIN32
      QueryPerformanceCounter(&m_iStartTicks);
#endif
    }
  }

}

CFunctionLogger::~CFunctionLogger()
{
  if ((m_pLogger != NULL) && (m_strFunctionName.length() > 0))
  {
    if (!m_pLogger->GetFunctionTiming())
      m_pLogger->LogFunctionExit(m_strFunctionName);
    else
    {
#ifdef _WIN32
      LARGE_INTEGER iEndTicks;
      QueryPerformanceCounter(&iEndTicks);
      // Add our total ticks to the tracking map object in the logger object
	  m_pLogger->LogFunctionTicks(m_strFunctionName,
		  iEndTicks.QuadPart - m_iStartTicks.QuadPart);
#endif
    }
  }
}

// Update what log level this object is using
void CFileLogger::SetLogLevel(const eLogLevel iNewLevel)
{
  m_iLogLevel = iNewLevel;
}

// Update whether function entry/exit is logged
void CFileLogger::SetFunctionLogging(bool bFunctionLogging)
{
  m_bFunctionLogging = bFunctionLogging;
}


// Gets the time and/or date stamp as specified
// by our construction options.
std::string CFileLogger::GetTimeDateStamp()
{
  std::string strTimeStamp = "";

  if ((m_bTimeStamp) || (m_bDateStamp))
  {
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
 
    // Format is:
    // Wed Jun 22 10:22:00 2005
    // 0123456789012345678901234
    if ((szTimeLine != NULL) && (strlen(szTimeLine) > 23))
    {
      if (m_bDateStamp)
      {
        for (int i = 4; i < 10; i++)
          strTimeStamp += szTimeLine[i];            
        for (int i = 19; i < 24; i++)
          strTimeStamp += szTimeLine[i];            
        if (m_bTimeStamp)
          strTimeStamp += " ";
      }

      if (m_bTimeStamp)
      {
        for (int i = 11; i < 19; i++)
          strTimeStamp += szTimeLine[i];
        strTimeStamp += ".";
        char strMs[16];
#ifdef _WIN32
        sprintf(strMs, "%d", sTimeBuffer.millitm);
#else
        sprintf(strMs, "%d", static_cast<int>(sTimeBuffer.tv_usec / 1000));
#endif
        if (strlen(strMs) == 1)
          strTimeStamp += "00";
        else if (strlen(strMs) == 2)
          strTimeStamp += "0";
        strTimeStamp += strMs;
      }

      strTimeStamp += "\t";
    }
  }

  return strTimeStamp;
}
