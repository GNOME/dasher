
// Handles logging of user activities such as what they write, how 
// long they were writing, mouse positions, parameters, etc
//
// Two different kinds of logging can be produced:
//      1) Running stats file that records basic stats about
//         how much and how fast a user is writing.
//      2) Detailed per session log file for use during
//         user trials.
//
// If detailed mode isn't enabled, calls should stop here in this object
// and not go on to create UserLogTrial objects or anything that isn't
// strictly needed to do the simple logging.
//
// For normal dasher, a new trial involves this sequence:
//      1) Program start or new doc
//      2) Nav start
//      3) Nav stop
//      4) Optionally goto 2
// A new trial must be indicated by the user resetting using new doc event.
// Time from 2-4 is elapsed trial time.
//
// Copyright 2005 by Keith Vertanen

#ifndef __USER_LOG_H__
#define __USER_LOG_H__

#include "FileLogger.h"
#include <string>
#include <vector>
#include "SimpleTimer.h"
#include "TimeSpan.h"
#include <iostream>
#include <fstream>
#include "UserLogTrial.h"
#include "Alphabet/Alphabet.h"
#include <algorithm>
#include "UserLogParam.h"

// We'll only compile in some bits if we are the tool that reads in UserLog files
// and can operate on them in some way.
#ifdef USER_LOG_TOOL
    #include "XMLUtil.h"
#endif

using namespace std;

extern CFileLogger* gLogger;

const int USER_LOG_DEFAULT_SIZE_TRIAL_XML   = 65536;    // How big we think the XML string representing a single trial will be
const int LOG_MOUSE_EVERY_MS                = 200;      // How often to log the mouse position (-1 for never), the frequency is also depends on how often the WM_TIMER event fires in dasher

static const string    USER_LOG_SIMPLE_FILENAME = "dasher_usage.log";      // Filename of the short text log file
static const string    USER_LOG_DETAILED_PREFIX = "dasher_";               // Prefix of the detailed XML log files
static const bool      USER_LOG_DUMP_AFTER_TRIAL = true;                   // Do we want to dump the XML after each trial is complete?

enum eUserLogLevel
{
    userLogSimple           = 1,    // Simple running log file
    userLogDetailed         = 2     // Detailed per session user trial style
};

#ifndef VECTOR_STRING
typedef vector<string>                      VECTOR_STRING;
#endif
#ifndef VECTOR_STRING_ITER
typedef vector<string>::iterator            VECTOR_STRING_ITER;
#endif
#ifndef VECTOR_VECTOR_STRING
typedef vector<VECTOR_STRING>               VECTOR_VECTOR_STRING;
#endif
#ifndef VECTOR_VECTOR_STRING_ITER
typedef vector<VECTOR_STRING>::iterator     VECTOR_VECTOR_STRING_ITER;
#endif

class CUserLog 
{
public:
    CUserLog(int logTypeMask, Dasher::CAlphabet* pAlphabet);
    ~CUserLog();

    // Methods called whenever our user interface gets a relevant event, this
    // object will decide how to put it into its representation.
    void                        AddParam(const string& strName, const string& strValue, int optionMask = 0);
    void                        AddParam(const string& strName, double value, int optionMask = 0);
    void                        AddParam(const string& strName, int value, int optionMask = 0);
    void                        StartWriting();
    void                        StopWriting(float nats);
    void                        StopWriting();
    void                        AddSymbols(Dasher::VECTOR_SYMBOL_PROB* vectorNewSymbolProbs, eUserLogEventType = userLogEventMouse);
    void                        DeleteSymbols(int numToDelete, eUserLogEventType event = userLogEventMouse);    
    void                        NewTrial();
    
    void                        AddWindowSize(int top, int left, int bottom, int right);
    void                        AddCanvasSize(int top, int left, int bottom, int right);
    void                        AddMouseLocation(int x, int y, float nats);
    void                        AddMouseLocationNormalized(int x, int y, bool bStoreIntegerRep, float nats);
    void                        OutputFile();
	void					    SetAlphabetPtr(Dasher::CAlphabet* pAlphabet);
    void                        InitIsDone();
    void                        SetOuputFilename(const string& strFilename = "");

#ifdef USER_LOG_TOOL
    CUserLog(string strXMLFilename);
    VECTOR_VECTOR_STRING        GetTabMouseXY(bool bReturnNormalized);
    VECTOR_VECTOR_DENSITY_GRIDS GetMouseDensity(int gridSize);
#endif

protected:
    CTimeSpan*                  m_pApplicationSpan;     // How long the application has been up
    string                      m_strFilename;          // Name we output our XML file to
    VECTOR_USER_LOG_TRIAL_PTR   m_vectorTrials;         // Holds object for each trial in this session
    VECTOR_USER_LOG_PARAM_PTR   m_vectorParams;         // Stores general parameters we want in the XML
    double                      m_lastMouseUpdate;      // When the last mouse update was pushed
    bool                        m_bSimple;              // Are we outputting the simple running log file?
    bool                        m_bDetailed;            // Are we outputting per session detailed logs?
    CFileLogger*                m_pSimpleLogger;        // Used to log the simple running log file
	Dasher::CAlphabet*			m_pAlphabet;		    // Pointer to Dasher alphabet object
    bool                        m_bIsWriting;           // Has StartWriting() been called but not StopWriting()?
    bool                        m_bInitIsDone;          // Set to true once the initialization of default values is done
    WindowSize                  m_canvasCoordinates;    // The size of our canvas from the last call to AddCanvasSize()
    WindowSize                  m_windowCoordinates;    // Records the window coordinates at the start of navigation
    bool                        m_bNeedToWriteCanvas;   // Do we need to write new canvas coordinates on the next navigation?

    CUserLogTrial*              AddTrial();
    CUserLogTrial*              GetCurrentTrial();
    string                      GetXML();
    bool                        WriteXML();
    bool                        UpdateMouseLocation();
    string                      GetParamsXML();
    void                        PrepareNewTrial();
    string                      GetCycleParamStats();
    string                      GetVersionInfo();
    void                        InitMemberVars();

    // Things that support simple stats of a single Start/Stop cycle:
    Dasher::VECTOR_SYMBOL_PROB  m_vectorCycleHistory;   // Tracks just the most recent Start/Stop cycle, used for simple logging
    unsigned int                m_cycleNumDeletes;      // Track number of deletes in last Start/Stop cycle
    CSimpleTimer*               m_pCycleTimer;          // Length of the last Start/Stop cycle
    double                      m_cycleMouseNormXSum;   // Sum of all normalized mouse X coordinates
    double                      m_cycleMouseNormYSum;   // Sum of all normalized mouse Y coordinates
    unsigned long               m_cycleMouseCount;      // How many mouse updates have been stores
    double                      m_cycleNats;            // The last nats value we got from a mouse event

    string                      GetStartStopCycleStats();
    double                      GetCycleBits();
    void                        ComputeSimpleMousePos(int x, int y);
    void                        ResetCycle();

};

#endif
