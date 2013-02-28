// FileLogger
//
// A very simple class that does logging to a file.
// Currently not thread safe and uses CRT file I/O that will melt down 
// on Windows if too many files are open.
//
// Copyright 2004 by Keith Vertanen
//

#ifndef __FileLogger_h__
#define __FileLogger_h__

#include <stdio.h>
#include <string>
#include <stdarg.h>
#include <time.h>
#include <map>
#include "DasherTypes.h"
// Probably better to enable in project settings since FileLogger.h is included from several physical locations.
// #define DEBUG_ONLY_LOGGING   // Enabled debug logging that has been ifdef'd to prevent performance problems in release build

// Macro that lets us wrap log statements that we want compiled out of the code in non-debug builds
#ifdef DEBUG_ONLY_LOGGING
#define DEBUG_ONLY( s )     s
#else
#define DEBUG_ONLY( s )
#endif

// Macros that can be used to call a globally declared logging object.  These
// would need to be modified if the global variable is named differently.  By
// using these macros you are protected from using the logger if it hasn't
// yet been created (it should be intialized to NULL).  Also has versions that
// automatically indicate the log level without sending a parameter.
//
// Note: to use these you must use double open and close parentheses, this
// is due to the variable parameter list that logging can take to do printf
// style output.  GCC supports variadic macros, but Visual Studio doesn't yet.
//

// General purpose version, a log level must be sent in prior to any variable
// length parameter list.  Usage:
//    LOG(("my favorite number is %d!", logDEBUG, 42))
#define LOG( s )\
  if (g_pLogger != NULL)\
    g_pLogger->Log s ;

// Debug only logging macro.  Usage:
//    LOG(("my favorite number is %d!", 42))
#define LOG_DEBUG( s )\
  if (g_pLogger != NULL)\
    g_pLogger->LogDebug s ;

// Normal error message macro.  Usage:
//    LOG(("errors number %d!", 42))
#define LOG_ERROR( s )\
  if (g_pLogger != NULL)\
    g_pLogger->LogNormal s ;

// Normal error message macro.  Usage:
//    LOG(("plane %d crashed into plane %d!", 42, 24))
#define LOG_CRITICAL( s )\
  if (g_pLogger != NULL)\
    g_pLogger->LogCritical s ;

#ifdef _WIN32
// Types required by our high resolution WIN32 timing routines
#include "WinCommon.h"
typedef std::map<std::string, __int64> MAP_STRING_INT64;

#endif


enum eLogLevel
{
    logDEBUG = 0,
    logNORMAL = 1,
    logCRITICAL = 2
};

// Bit mask options that are used when we construct object
enum eFileLoggerOptions
{
    logFunctionEntryExit    = 1,
    logTimeStamp            = 2,
    logDateStamp            = 4,
    logDeleteOldFile        = 8,
    logFunctionTiming       = 16,
    logOutputScreen         = 32
};
/// \ingroup Logging
/// @{
class CFileLogger
{
public:
    CFileLogger(const std::string& strFilenamePath, eLogLevel level, int optionsMask);
    ~CFileLogger();


    void Log(const char* szText, eLogLevel iLogLevel = logNORMAL, ...);         // Logs a string to our file if it meets or exceeds our logging level
    void LogDebug(const char* szText, ...);                                     // Logs debug level messages
    void LogNormal(const char* szText, ...);                                    // Logs normal level messages    
    void LogCritical(const char* szText, ...);                                  // Logs critical level messages

    // Versions that exists so we can pass in STL strings
    void Log(const std::string strText, eLogLevel iLogLevel = logNORMAL, ...);        // Logs a string to our file if it meets or exceeds our logging level
    
    void SetFilename(const std::string& strFilename);
    void SetLogLevel(const eLogLevel newLevel);
    void SetFunctionLogging(bool functionLogging);

    void LogFunctionEntry(const std::string& strFunctionName);                  // Used by FunctionLogger to log entry to a function
    void LogFunctionExit(const std::string& strFunctionName);                   // Used by FunctionLogger to log exit from a function
#ifdef _WIN32
    void LogFunctionTicks(const std::string& strFunctionName, __int64 iTicks);  // Used by FunctionLogger to log how long was spent in a function
#endif
    bool GetFunctionTiming();
    
    static std::string GetFullFilenamePath(std::string strFilename);

private:
    std::string     m_strFilenamePath;          // Filename and path of our output file	
    eLogLevel       m_iLogLevel;                // What level of logging this object should write
    bool            m_bFunctionLogging;         // Whether we will log function entry/exit 
    bool            m_bTimeStamp;               // Whether we log the time
    bool            m_bDateStamp;               // Whether we log the date
    bool            m_bFunctionTiming;          // Whether our FunctionLogger objects should do performance timing
    bool            m_bDeleteOldFile;           // Should we delete a previous instance of the log file
    bool            m_bOutputScreen;            // Should we output to stdout as well as the file
    int             m_iFunctionIndentLevel;     // How many nested calls to FunctionLogger we have

    std::string     GetIndentedString(const std::string& strText);
    std::string     GetTimeDateStamp();

#ifdef _WIN32
    MAP_STRING_INT64    m_mapFunctionTicks;     // Keeps track of how many ticks spent in each of our functions (who create a CFunctionLogger object)
#endif

};

// Helper class, you can create CFunctionLogger objects at 
// the top of a function and it will log its entry and exit.
class CFunctionLogger
{
public:
    CFunctionLogger(const std::string& strFunctionName, CFileLogger* pLogger);
    ~CFunctionLogger();

private:
    std::string     m_strFunctionName;          // Name of the function this object is logging
    CFileLogger*    m_pLogger;                  // Pointer to the logging object to use 
    
#ifdef _WIN32
    LARGE_INTEGER   m_iStartTicks;              // Tick count at start of timing
#endif

};
/// @}
#endif
