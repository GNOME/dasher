
#include "FileLogger.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#include "../Win32/Common/WinUTF8.h"

CFileLogger::CFileLogger(const std::string& strFilenamePath, eLogLevel logLevel, int optionsMask)
{
    m_strFilenamePath       = strFilenamePath;
    m_logLevel              = logLevel;
    m_functionIndentLevel   = 0;

    m_bFunctionLogging      = false;
    m_bTimeStamp            = false;
    m_bDateStamp            = false;
    m_bDeleteOldFile        = false;
    m_bFunctionTiming       = false;
    m_bOutputScreen         = false;

    // See what options are set in our bit mask options parameter
    if (optionsMask & logFunctionEntryExit)
        m_bFunctionLogging = true;
    if (optionsMask & logTimeStamp)
        m_bTimeStamp = true;
    if (optionsMask & logDateStamp)
        m_bDateStamp = true;
    if (optionsMask & logDeleteOldFile)
        m_bDeleteOldFile = true;
    if (optionsMask & logFunctionTiming)
        m_bFunctionTiming = true;
    if (optionsMask & logOutputScreen)
        m_bOutputScreen = true;

    // Make sure our filename and path is converted to fully qualified form
    SetFilenamePath(strFilenamePath);

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
#ifdef WIN32
    if (m_bFunctionTiming)
    {
        // Dump the results of our function timing logging

        MAP_STRING_INT64::iterator map_iter;

        Log("%-60s%20s%10s", logNORMAL, "Function","Ticks", "Percent");
        Log("%-60s%20s%10s", logNORMAL, "--------","-----", "-------");

        __int64 maxTicks = 0;

        // First pass to count the max ticks 
        // We assume that there was a function logger on the outer most (main) program.
        // This allows the percent reflect the relative time spent inside emedded calls.

        for (map_iter = m_mapFunctionTicks.begin(); map_iter != m_mapFunctionTicks.end(); map_iter++)
        {
            if (map_iter->second > maxTicks)
                maxTicks = map_iter->second;
        }

        for (map_iter = m_mapFunctionTicks.begin(); map_iter != m_mapFunctionTicks.end(); map_iter++)
        {
            std::string name = map_iter->first;
            __int64 ticks = map_iter->second;
            Log("%-60s%20I64Ld%10.2f", logNORMAL, name.c_str(), ticks, (double) ticks / (double) maxTicks  * (double) 100.0);
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
void CFileLogger::Log(const std::string& str, eLogLevel logLevel, ...)
{
    va_list       args;  

    if ((m_strFilenamePath.length() > 0) && (m_logLevel <= logLevel))
    {
        FILE *fp = NULL;
        fp = fopen(m_strFilenamePath.c_str(), "a");

        std::string strTimeStamp = GetTimeDateStamp();

        if (fp != NULL)
        {
            std::string strIndented = strTimeStamp + GetIndentedString(str) + "\n";

            va_start(args, logLevel);
            vfprintf(fp, strIndented.c_str(), args);
            va_end(args);

            // Optionally we can output message to stdout
            if (m_bOutputScreen)
            {
                va_start(args, logLevel);
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
        Log("start: " + strFunctionName);
        m_functionIndentLevel++;
    }
}

// Logs exit into a particular function
void CFileLogger::LogFunctionExit(const std::string& strFunctionName)
{
    if (m_bFunctionLogging)
    {
        m_functionIndentLevel--;
        Log("end: " + strFunctionName);
    }
}

#ifdef WIN32
void CFileLogger::LogFunctionTicks(const std::string& strFunctionName, __int64 ticks)
{
    __int64 current;

    current = m_mapFunctionTicks[strFunctionName];
    current = current + ticks;

    m_mapFunctionTicks[strFunctionName] = current;
}
#endif

// Gets an indented version of the function name 
std::string CFileLogger::GetIndentedString(const std::string& str)
{
    std::string strIndented = "";
    for (int i = 0; i < m_functionIndentLevel; i++)
        strIndented += " ";
    strIndented += str;

    return strIndented;
}

bool CFileLogger::GetFunctionTiming()
{
    return m_bFunctionTiming;
}

/////////////////////////////////////// CFunctionLogger /////////////////////////////////////////////////////////////

CFunctionLogger::CFunctionLogger(const std::string& strFunctionName, CFileLogger* logger) 
{
    m_logger = logger;

    if ((m_logger != NULL) && (strFunctionName.length() > 0))
    {
        m_strFunctionName = strFunctionName;

        if (!m_logger->GetFunctionTiming())
            m_logger->LogFunctionEntry(m_strFunctionName);
        else
        {
#ifdef WIN32
            BigInt startTicks;
            _asm
            {
                RDTSC
                    mov startTicks.int32val.i32[0], eax
                    mov startTicks.int32val.i32[4], edx
            }
            m_startTicks  = startTicks;
#endif
        }
    }

}

CFunctionLogger::~CFunctionLogger()
{
    if ((m_logger != NULL) && (m_strFunctionName.length() > 0))
    {
        if (!m_logger->GetFunctionTiming())
            m_logger->LogFunctionExit(m_strFunctionName);
        else
        {
#ifdef WIN32
            BigInt endTicks;

            _asm
            {
                RDTSC
                    mov endTicks.int32val.i32[0], eax
                    mov endTicks.int32val.i32[4], edx
            }
            // Add our total ticks to the tracking map object in the logger object
            m_logger->LogFunctionTicks(m_strFunctionName, endTicks.int64val.i64 -
                m_startTicks.int64val.i64);
#endif
        }

    }
}

// Update what log level this object is using
void CFileLogger::SetLogLevel(const eLogLevel newLevel)
{
    m_logLevel = newLevel;
}

// Update whether function entry/exit is logged
void CFileLogger::SetFunctionLogging(bool functionLogging)
{
    m_bFunctionLogging = functionLogging;
}


// Gets the time and/or date stamp as specified
// by our construction options.
std::string CFileLogger::GetTimeDateStamp()
{
    std::string strTimeStamp = "";

    if ((m_bTimeStamp) || (m_bDateStamp))
    {
        struct timeb timebuffer;
        char* timeline = NULL;

        ftime( &timebuffer );

        timeline = ctime( & ( timebuffer.time ) );

        // Format is:
        // Wed Jun 22 10:22:00 2005
        // 0123456789012345678901234
        if ((timeline != NULL) && (strlen(timeline) > 23))
        {
            if (m_bDateStamp)
            {
                for (int i = 4; i < 10; i++)
                    strTimeStamp += timeline[i];            
                for (int i = 19; i < 24; i++)
                    strTimeStamp += timeline[i];            
                if (m_bTimeStamp)
                    strTimeStamp += " ";
            }

            if (m_bTimeStamp)
            {
                for (int i = 11; i < 19; i++)
                    strTimeStamp += timeline[i];
                strTimeStamp += ".";
                char strMs[16];
                sprintf(strMs, "%d", timebuffer.millitm);
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

// Sets the member variable of the filename/path of our log file.
// This method handles making sure we save a fully qualified path
// by converting it if neccesary.
void CFileLogger::SetFilenamePath(const std::string& strFilenamePath)
{
    // On windows anyway if somebody can open up a file with CreateFile() in a different 
    // directory and cause the working directory to change.  We don't want our log file
    // moving around, so we'll find a absolute path when we are created and stick to
    // that for the remainder of our life.
#ifdef _WIN32    
    
    // Windows code
    const int   MAX_PATH_LENGTH = 1024;

#ifdef _UNICODE

    // In Unicode, we need the parameters to GetFullPathName() in wide characters
    wchar_t     szPath[MAX_PATH_LENGTH];
    wchar_t*    pszFilePart = NULL;
    wchar_t     wstrFilenamePath[MAX_PATH_LENGTH];
    char        strResult[MAX_PATH_LENGTH];

    unsigned int i = 0;
    for (i = 0; i < strFilenamePath.length(); i++)
        wstrFilenamePath[i] = (wchar_t) strFilenamePath[i];
    wstrFilenamePath[i] = '\0';

    ::GetFullPathName(wstrFilenamePath, MAX_PATH_LENGTH, szPath, &pszFilePart);
    i = 0;
    while (szPath[i] != '\0')
    {
        strResult[i] = (char) szPath[i];
        i++;
    }
    strResult[i] = '\0';

    m_strFilenamePath = strResult;
#else
    // Using normal non-unicode strings
    char        szPath[MAX_PATH_LENGTH];
    char*       pszFilePart = NULL;
    char        strResult[MAX_PATH_LENGTH];

    ::GetFullPathName(strFilenamePath.c_str(), MAX_PATH_LENGTH, szPath, &pszFilePart);
    
    m_strFilenamePath = szPath;
#endif

#else
    // Non-windows code
    m_strFilenamePath = str;

#endif

}

