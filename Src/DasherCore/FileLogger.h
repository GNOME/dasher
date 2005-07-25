
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
#include <sys/timeb.h>
#include <time.h>
#include <map>

// Probably better to enable in project settings since FileLogger.h is included from several physical locations.
// #define DEBUG_ONLY_LOGGING   // Enabled debug logging that has been ifdef'd to prevent performance problems in release build

// Macro that lets us wrap log statements that we want compiled out of the code in non-debug builds
#ifdef DEBUG_ONLY_LOGGING
#define DEBUG_ONLY( s )     s
#else
#define DEBUG_ONLY( s )
#endif

#ifdef WIN32
// Types required by our high resolution WIN32 timing routines

typedef  struct _BinInt32
{
    __int32 i32[2];
} BigInt32;

typedef  struct _BigInt64
{
    __int64 i64;
} BigInt64;

typedef union _bigInt
{
    BigInt32 int32val;
    BigInt64 int64val;
} BigInt;

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

class CFileLogger
{
public:
    CFileLogger(const std::string& strFilenamePath, eLogLevel level, int optionsMask);
    ~CFileLogger();


    void Log(const std::string& str, eLogLevel level = logNORMAL, ...);         // Logs a string to our file if it meets our exceeds our logging level
    void LogDebugStr(const std::string& str, ...);                              // Same as calling Log() with logDEBUG
    void LogNormalStr(const std::string& str, ...);                             // Same as calling Log() with logNORMAL
    void LogCriticalStr(const std::string& str, ...);                           // Same as calling Log() with logCRITICAL
    void SetFilename(const std::string& strFilename);
    void SetLogLevel(const eLogLevel newLevel);
    void SetFunctionLogging(bool functionLogging);

    void LogFunctionEntry(const std::string& strFunctionName);                  // Used by FunctionLogger to log entry to a function
    void LogFunctionExit(const std::string& strFunctionName);                   // Used by FunctionLogger to log exit from a function
#ifdef WIN32
    void LogFunctionTicks(const std::string& strFunctionName, __int64 ticks);   // Used by FunctionLogger to log how long was spent in a function
#endif
    bool GetFunctionTiming();
    
    static std::string GetFullFilenamePath(std::string strFilename);

private:
    std::string     m_strFilenamePath;          // Filename and path of our output file	
    eLogLevel       m_logLevel;                 // What level of logging this object should write
    bool            m_bFunctionLogging;         // Whether we will log function entry/exit 
    bool            m_bTimeStamp;               // Whether we log the time
    bool            m_bDateStamp;               // Whether we log the date
    bool            m_bFunctionTiming;          // Whether our FunctionLogger objects should do performance timing
    bool            m_bDeleteOldFile;           // Should we delete a previous instance of the log file
    bool            m_bOutputScreen;            // Should we output to stdout as well as the file
    int             m_functionIndentLevel;      // How many nested calls to FunctionLogger we have

    std::string     GetIndentedString(const std::string& str);
    std::string     GetTimeDateStamp();

#ifdef WIN32
    MAP_STRING_INT64    m_mapFunctionTicks;     // Keeps track of how many ticks spent in each of our functions (who create a CFunctionLogger object)
#endif

};

// Helper class, you can create CFunctionLogger objects at 
// the top of a function and it will log its entry and exit.
class CFunctionLogger
{
public:
    CFunctionLogger(const std::string& strFunctionName, CFileLogger* logger);
    ~CFunctionLogger();

private:
    std::string     m_strFunctionName;          // Name of the function this object is logging
    CFileLogger*    m_logger;                   // Pointer to the logging object to use 
    
#ifdef WIN32
    BigInt          m_startTicks;               // Tick count at start of timing
#endif

};

#endif
