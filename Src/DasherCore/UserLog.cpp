
#include "UserLog.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

using namespace expat;

CUserLog::CUserLog(int logTypeMask, Dasher::CAlphabet* pAlphabet)
{
    InitMemberVars();
    m_pAlphabet             = pAlphabet;

    if (logTypeMask & userLogSimple)
    {
        // First we check to see if the file exists, if it does not
        // then we want to force all parameter values to be sent to
        // the log file even before InitIsDone() is called.
        FILE* fp = fopen(USER_LOG_SIMPLE_FILENAME.c_str(), "r");
        if (fp == NULL)
        {
            m_bInitIsDone = true;
        }
        else
        {
            fclose(fp);
            fp = NULL;
        }

        m_bSimple       = true;
        m_pSimpleLogger = new CFileLogger(USER_LOG_SIMPLE_FILENAME, logDEBUG, logTimeStamp | logDateStamp);
        m_pSimpleLogger->Log("start, %s", logDEBUG, GetVersionInfo().c_str());
    }

    if (logTypeMask & userLogDetailed)
        m_bDetailed = true;        

    SetOuputFilename();
    m_pApplicationSpan = new CTimeSpan("Application", true);
}

CUserLog::~CUserLog()
{
    if ((m_bSimple) && (m_pSimpleLogger != NULL))
        m_pSimpleLogger->Log("stop", logDEBUG);

    if (m_pApplicationSpan != NULL)
    {
        delete m_pApplicationSpan;
        m_pApplicationSpan = NULL;
    }

    for (unsigned int i = 0; i < m_vectorTrials.size(); i++)
    {
        CUserLogTrial* trial = (CUserLogTrial*) m_vectorTrials[i];

        if (trial != NULL)
        {
            delete trial;
            trial = NULL;
        }
    }

    for (unsigned int i = 0; i < m_vectorParams.size(); i++)
    {
        CUserLogParam* param = (CUserLogParam*) m_vectorParams[i];

        if (param != NULL)
        {
            delete param;
            param = NULL;
        }
    }

    if (m_pSimpleLogger != NULL)
    {
        delete m_pSimpleLogger;
        m_pSimpleLogger = NULL;
    }

    if (m_pCycleTimer != NULL)
    {
        delete m_pCycleTimer;
        m_pCycleTimer = NULL;
    }
}

// Called when we want to output the log file (usually on exit of dasher)
void CUserLog::OutputFile()
{
    if (m_bDetailed)
    {
        // Let the last trial object know we are done with it, this lets it do
        // any final calculations.
        if (m_vectorTrials.size() > 0)
        {
            CUserLogTrial* trial = m_vectorTrials[m_vectorTrials.size() - 1];

            if (trial != NULL)
                trial->Done();
        }

        // Output our data to an XML file before we destruct
        WriteXML();
    }
}

void CUserLog::StartWriting()
{
    if (m_bSimple)
    {
        // The canvas size changes multiple times as a user resizes it.  We just want to write
        // one short log entry for the final position the next time they start writing.
        if (m_bNeedToWriteCanvas)
        {
            m_pSimpleLogger->Log("canvas:\t%d\t%d\t%d\t%d", 
                                  logDEBUG, 
                                  m_canvasCoordinates.top, 
                                  m_canvasCoordinates.left, 
                                  m_canvasCoordinates.bottom, 
                                  m_canvasCoordinates.right);
            m_bNeedToWriteCanvas = false;
        }
 
        // We log what happened between StartWriting() and StopWriting()
        // so clear out any previous history.
        ResetCycle();
    }

    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        // This could be the first use in this trial, create a new one if needed
        if (trial == NULL)
            trial = AddTrial();

        trial->StartWriting();
    }

    m_bIsWriting = true;
}

// This version should be called at the end of navigation with the nats
// value under the current mouse position.  This would be more accurate
// then the last value from a mouse event since some time may have 
// elapsed.
void CUserLog::StopWriting(float nats)
{
    m_cycleNats = (double) nats;
    StopWriting();
}

void CUserLog::StopWriting()
{
    m_bIsWriting = false;

    // In simple logging mode, we'll output the stats for this navigation cycle
    if ((m_bSimple) && (m_pSimpleLogger != NULL))
        m_pSimpleLogger->Log("%s", logDEBUG, GetStartStopCycleStats().c_str());

    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        if (trial == NULL)
        {
            gLogger->Log("CUserLog::StopWriting, trial was NULL!", logNORMAL);
            return;
        }

        trial->StopWriting();
    }
}

void CUserLog::AddSymbols(Dasher::VECTOR_SYMBOL_PROB* vectorNewSymbols, eUserLogEventType event)
{
    if (!m_bIsWriting)
    {
        // StartWriting() wasn't called, so we'll do it implicitly now
        gLogger->Log("CUserLog::AddSymbols, StartWriting() not called?", logDEBUG);
        StartWriting();
    }

    if (m_bSimple)
    {
        // Also store a copy in a vector that gets cleared 
        // time StartWriting() is called.

        for (unsigned int i = 0; i < vectorNewSymbols->size(); i++)
        {
            Dasher::SymbolProb newSymbolProb = (Dasher::SymbolProb) (*vectorNewSymbols)[i];
            m_vectorCycleHistory.push_back(newSymbolProb);
        }
    }
    
    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        if (vectorNewSymbols == NULL)
        {
            gLogger->Log("CUserLog::AddSymbols, vectorNewSymbols was NULL!", logNORMAL);
            return;
        }

        // We should have a trial object since StartWriting() should have been called before us
        if (trial == NULL)
        {
            gLogger->Log("CUserLog::AddSymbols, trial was NULL!", logNORMAL);
            return;
        }

        trial->AddSymbols(vectorNewSymbols, event, m_pAlphabet);
    }
}

void CUserLog::DeleteSymbols(int numToDelete, eUserLogEventType event)
{
    if (numToDelete <= 0)
        return;

    if (!m_bIsWriting)
    {
        // StartWriting() wasn't called, so we'll do it implicitly now
        gLogger->Log("CUserLog::DeleteSymbols, StartWriting() not called?", logDEBUG);
        StartWriting();
    }

    if (m_bSimple)
    {
        m_cycleNumDeletes += numToDelete;

        for (int i = 0; i < numToDelete; i++)
            m_vectorCycleHistory.pop_back();
    }   

    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        // We should have a trial object since StartWriting() should have been called before us
        if (trial == NULL)
        {
            gLogger->Log("CUserLog::DeleteSymbols, trial was NULL!", logNORMAL);
            return;
        }

        trial->DeleteSymbols(numToDelete, event);
    }
}

void CUserLog::NewTrial()
{
    if (m_bIsWriting)
    {
        // We should have called StopWriting(), but we'll do it here implicitly
        gLogger->Log("CUserLog::NewTrial, StopWriting() not called?", logDEBUG);        
        StopWriting();
    }

    // For safety we can dump the XML to file after each trial is done.  This
    // might be a good idea for long user trial sessions just in case Dasher
    // were to do something completely crazy like crash.
    if (USER_LOG_DUMP_AFTER_TRIAL)
        WriteXML();

    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        if (trial == NULL)
        {
            // Not an error, they may just hit new doc before anything else at start
            return;
        }

        if (trial->HasWritingOccured())
        {
            // Create a new trial if the existing one has already been used
            trial = AddTrial();
        }
    }
}

// Overloaded version that converts a double to a string
void CUserLog::AddParam(const string& strName, double value, int optionMask)
{
    char strNum[256];
    sprintf(strNum, "%0.4f", value);
    AddParam(strName, strNum, optionMask);
}

// Overloaded version that converts a int to a string
void CUserLog::AddParam(const string& strName, int value, int optionMask)
{
    char strNum[256];
    sprintf(strNum, "%d", value);
    AddParam(strName, strNum, optionMask);
}

// Adds a general parameter to our XML.  This lets various Dasher components 
// record what they are set at.  Optionally can be set to track multiple
// values for the same parameter or to always output a line to the simple
// log file when the parameter is set.
void CUserLog::AddParam(const string& strName, const string& strValue, int optionMask)
{
    bool bOutputToSimple    = false;
    bool bTrackMultiple     = false;
    bool bTrackInTrial      = false;
    bool bForceInTrial      = false;
    bool bShortInCycle      = false;

    if (optionMask & userLogParamOutputToSimple)
        bOutputToSimple = true;
    if (optionMask & userLogParamTrackMultiple)
        bTrackMultiple = true;
    if (optionMask & userLogParamTrackInTrial)
        bTrackInTrial = true;
    if (optionMask & userLogParamForceInTrial)
        bForceInTrial = true;
    if (optionMask & userLogParamShortInCycle)
        bShortInCycle = true;

    // See if we want to immediately output this name/value pair to
    // the running simple log file.  If we are tracking the parameter
    // in the short cycle stats line, then don't output here.
    if ((bOutputToSimple) && 
        (m_bSimple) && 
        (m_pSimpleLogger != NULL) && 
        (m_bInitIsDone) && 
        (!bShortInCycle))
    {
        m_pSimpleLogger->Log("%s = %s", logDEBUG, strName.c_str(), strValue.c_str());
    }

    if (m_bDetailed)
    {
        // See if this matches an existing parameter value that we may want to 
        // overwrite.  But only if we aren't suppose to keep track of multiple changes.
        if (!bTrackMultiple)
        {
            for (unsigned int i = 0; i < m_vectorParams.size(); i++)
            {
                CUserLogParam* param = (CUserLogParam*) m_vectorParams[i];

                if (param != NULL)
                {
                    if (param->strName.compare(strName) == 0)
                    {
                        param->strValue = strValue;
                        return;
                    }
                }
            }
        }

        // We need to add a new param
        CUserLogParam* newParam      = new CUserLogParam;

        newParam->strName           = strName;
        newParam->strValue          = strValue;
        newParam->strTimeStamp      = "";
        newParam->options           = optionMask;

        // Parameters that can have multiple values logged will also log when they were changed
        if (bTrackMultiple)
            newParam->strTimeStamp  = CTimeSpan::GetTimeStamp();

        m_vectorParams.push_back(newParam);

        // See if we need to pass the parameter onto the current trial object
        if ((bTrackInTrial) && (m_bDetailed))
        {
            CUserLogTrial* trial = GetCurrentTrial();

            if (trial != NULL)
                trial->AddParam(strName, strValue, optionMask);
        }

    }
}


// Adds a new point in our tracking of mouse locations
void CUserLog::AddMouseLocation(int x, int y, float nats)
{
    // Check to see if it is time to actually push a mouse location update
    if (UpdateMouseLocation())
    {
        if (m_bDetailed)
        {
            CUserLogTrial* trial = GetCurrentTrial();

            if (trial == NULL)
            {
                // Only track during an actual trial
                return;
            }

            // Only record mouse locations during navigation
            if (trial->IsWriting())
                trial->AddMouseLocation(x, y, nats);
        }

        // Keep track of the nats for the current mouse position
        if (m_bIsWriting)
            m_cycleNats = nats;
    }
}

// Adds the size of the current window
void CUserLog::AddWindowSize(int top, int left, int bottom, int right)
{
    m_windowCoordinates.top     = top;
    m_windowCoordinates.left    = left;
    m_windowCoordinates.bottom  = bottom;
    m_windowCoordinates.right   = right;

    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        if (trial == NULL)
        {
            // Only track during an actual trial
            return;
        }

        trial->AddWindowSize(top, left, bottom, right);
    }
}

// Adds the size of the current canvas, this should be called when our
// window is initially created and whenever somebody mucks with the 
// size.
void CUserLog::AddCanvasSize(int top, int left, int bottom, int right)
{
    // Only log to simple log object if the coordinates are different from 
    // what we had prior to now.
    if ((m_bSimple) && 
        (m_pSimpleLogger != NULL) &&
        ((m_canvasCoordinates.top != top) ||
         (m_canvasCoordinates.left != left) ||
         (m_canvasCoordinates.bottom != bottom) ||
         (m_canvasCoordinates.right != right)))
         m_bNeedToWriteCanvas = true;

    m_canvasCoordinates.top     = top;
    m_canvasCoordinates.left    = left;
    m_canvasCoordinates.bottom  = bottom;
    m_canvasCoordinates.right   = right;

    if (m_bDetailed)
    {
        CUserLogTrial* trial = GetCurrentTrial();

        if (trial == NULL)
        {
            // Only track during an actual trial
            return;
        }

        trial->AddCanvasSize(top, left, bottom, right);
    }
}

// We may want to use a noramlized version of mouse coordinates, this way it is
// invariant to changes in the window size before, during, or after navigation.
// The caller must send us both the x, y coordinates and the current window size.
void CUserLog::AddMouseLocationNormalized(int x, int y, bool bStoreIntegerRep, float nats)
{
    // Check to see if it is time to actually push a mouse location update
    if (UpdateMouseLocation())
    {
        if ((m_canvasCoordinates.bottom == 0) &&
            (m_canvasCoordinates.left == 0) &&
            (m_canvasCoordinates.right == 0) &&
            (m_canvasCoordinates.top == 0))
        {
            gLogger->Log("CUserLog::AddMouseLocationNormalized, called before AddCanvasSize()?", logNORMAL);
            return;
        }

        ComputeSimpleMousePos(x, y);

        if (m_bDetailed)
        {
            CUserLogTrial* trial = GetCurrentTrial();

            if (trial == NULL)
            {
                // Only track during an actual trial
                return;
            }

            // Only record mouse locations during navigation
            if (trial->IsWriting())
                trial->AddMouseLocationNormalized(x, y, bStoreIntegerRep, nats);
        }

        // Keep track of the nats for the current mouse position
        if (m_bIsWriting)
            m_cycleNats = nats;
    }
}

// For simple logging, we don't want to log the same parameters settings
// in the file every single time Dasher starts up.  So we require that
// this method be called once the initial loading of parameters is 
// complete.  This way only changes during a session are logged (we can
// also force logging of the parameter setting when the log file is 
// created by setting m_bInitIsDone to true in the constructor). 
void CUserLog::InitIsDone()
{
    m_bInitIsDone = true;
}

// Update our pointer to the alphabet we are using, we need th
// alphabet in order to convert symbols into display strings
// that we put in the log file.
void CUserLog::SetAlphabetPtr(Dasher::CAlphabet* pAlphabet)
{
    m_pAlphabet = pAlphabet;
}

// Sets our output filename based on the current date and time.
// Or if a parameter is passed in, use that as the output name.
void CUserLog::SetOuputFilename(const string& strFilename)
{
    if (strFilename.length() > 0)
    {
        m_strFilename = strFilename;
    }
    else
    {
        m_strFilename = USER_LOG_DETAILED_PREFIX;

        struct timeb timebuffer;
        char* timeline = NULL;

        ftime( &timebuffer );

        timeline = ctime( & ( timebuffer.time ) );

        if ((timeline != NULL) && (strlen(timeline) > 18))
        {
            for (int i = 4; i < 19; i++)
            {
                if (timeline[i] == ' ')
                    m_strFilename += "_";
                else if (timeline[i] != ':')
                    m_strFilename += timeline[i];
            }
        }

        m_strFilename += ".xml";
    }
}

////////////////////////////////////////// private methods ////////////////////////////////////////////////

// Just inits all our member variables, called by the constructors
void CUserLog::InitMemberVars()
{
    m_strFilename           = "";
    m_pApplicationSpan      = NULL;
    m_lastMouseUpdate       = 0.0;
    m_bSimple               = false;
    m_bDetailed             = false;
    m_pSimpleLogger         = NULL;    
    m_pAlphabet             = NULL;
    m_bIsWriting            = false;
    m_bInitIsDone           = false;
    m_bNeedToWriteCanvas    = false;

    m_pCycleTimer           = NULL;
    m_cycleNumDeletes       = 0;
    m_cycleMouseCount       = 0;
    m_cycleMouseNormXSum    = 0.0;
    m_cycleMouseNormYSum    = 0.0;
    m_cycleNats             = 0.0;

    m_canvasCoordinates.bottom  = 0;
    m_canvasCoordinates.top     = 0;
    m_canvasCoordinates.right   = 0;
    m_canvasCoordinates.left    = 0;

    m_windowCoordinates.bottom  = 0;
    m_windowCoordinates.top     = 0;
    m_windowCoordinates.right   = 0;
    m_windowCoordinates.left    = 0;
}

// Write this objects XML out  
bool CUserLog::WriteXML()
{
    try
    {
        fstream fout(m_strFilename.c_str(), ios::trunc | ios::out);
        fout << GetXML();
        fout.close();

        return true;

    } catch (...)
    {
        gLogger->Log("CUserLog::WriteXML, failed to write file %s", logNORMAL, m_strFilename.c_str());
        return false;
    }
}

// Serializes our data to XML
string CUserLog::GetXML()
{
    string strResult = "";
    strResult.reserve(DEFAULT_SIZE_USER_LOG_TRAIL_XML * (m_vectorTrials.size() + 1));

    strResult += "<?xml version=\"1.0\"?>\n";

    strResult += "<UserLog>\n";
    if (m_pApplicationSpan != NULL)
        strResult += m_pApplicationSpan->GetXML("\t");

    strResult += "\t<Params>\n";
    strResult += GetParamsXML();
    strResult += "\t</Params>\n";

    strResult += "\t<Trials>\n";
    for (unsigned int i = 0; i < m_vectorTrials.size(); i++)
    {
        CUserLogTrial* trial = (CUserLogTrial*) m_vectorTrials[i];

        // Only log trials that actually had some writing in it
        if ((trial != NULL) && (trial->HasWritingOccured()))
        {
            strResult += trial->GetXML("\t\t");
        }
    }
    strResult += "\t</Trials>\n";

    strResult += "</UserLog>\n";

    return strResult;
}

// Returns pointer to the current user trial, NULL if we don't have one yet
CUserLogTrial* CUserLog::GetCurrentTrial()
{
    if (m_vectorTrials.size() <= 0)
        return NULL;
    return m_vectorTrials[m_vectorTrials.size() - 1];
}

// Creates a new trial, adds to our vector and returns the pointer
CUserLogTrial* CUserLog::AddTrial()
{
    // Let the last trial object know we are done with it
    if (m_vectorTrials.size() > 0)
    {
        CUserLogTrial* trial = m_vectorTrials[m_vectorTrials.size() - 1];

        if (trial != NULL)
            trial->Done();
    }

    CUserLogTrial* trial = new CUserLogTrial();
    if (trial != NULL)
    {
        m_vectorTrials.push_back(trial);
        PrepareNewTrial();
    }
    else
        gLogger->Log("CUserLog::AddTrial, failed to create CUserLogTrialSpeech!", logNORMAL);

    return trial;
}

// See if the specified number of milliseconds has elapsed since the last mouse location update
bool CUserLog::UpdateMouseLocation()
{
    struct timeb timebuffer;
    ftime( &timebuffer );

    double time = (timebuffer.time * 1000.0) + timebuffer.millitm;

    if ((time - m_lastMouseUpdate) > LOG_MOUSE_EVERY_MS)
    {
        m_lastMouseUpdate = time;
        return true;
    }
    return false;
}

// Calculate how many bits entered in the last Start/Stop cycle
double CUserLog::GetCycleBits()
{
/*
    double result = 0.0;

    if (m_vectorCycleHistory.size() > 0)
    {
        for (unsigned int i = 0; i < m_vectorCycleHistory.size(); i++)
        {
            Dasher::SymbolProb item = (Dasher::SymbolProb) m_vectorCycleHistory[i];

            result += log(item.prob);
        }
        result = result * -1.0;
        result = result / log(2.0);
    }

    return result;
*/

    return m_cycleNats / log(2.0);
}

// For lightweight logging, we want a string that represents the critical
// stats for what happened between start and stop
string CUserLog::GetStartStopCycleStats()
{
    string strResult = "";


    char strNum[1024];

    double normX = 0.0;
    double normY = 0.0;
    if (m_cycleMouseCount > 0)
    {        
        normX = m_cycleMouseNormXSum / (double) m_cycleMouseCount,
        normY = m_cycleMouseNormYSum / (double) m_cycleMouseCount;
    }
    
    // Tab delimited fields are:
    //  elapsed time, symbols written, bits written, symbols deleted, 
    //  avg normalized x mouse coordinate, avg normalized y mouse
    //  coordinate, (any parameters marked to be put in cycle stats)
    //
    // tsbdxy stands for: time symbols bits deletes x y 
    sprintf(strNum, "tsbdxy:\t%0.3f\t%d\t%0.6f\t%d\t%0.3f\t%0.3f%s", 
                m_pCycleTimer->GetElapsed(), 
                m_vectorCycleHistory.size(), 
                GetCycleBits(), 
                m_cycleNumDeletes, 
                normX,
                normY, 
                GetCycleParamStats().c_str());
    strResult = strNum;

    return strResult;
}

// Helper that computes update of the simple logging's mouse 
// position tracking member variables.
void CUserLog::ComputeSimpleMousePos(int x, int y)
{
    if ((m_bSimple) && (m_bIsWriting))
    {
        // We keep a running sum of normalized X, Y coordinates
        // for use in the simple log file.
        m_cycleMouseNormXSum += CUserLocation::ComputeNormalizedX(x, 
                                                                  m_canvasCoordinates.left, 
                                                                  m_canvasCoordinates.right);

        m_cycleMouseNormYSum += CUserLocation::ComputeNormalizedY(y, 
                                                                  m_canvasCoordinates.top, 
                                                                  m_canvasCoordinates.bottom);
        m_cycleMouseCount++;
    }
}

// Resets member variables that track a cycle for simple logging
void CUserLog::ResetCycle()
{
    m_vectorCycleHistory.clear();
    m_cycleNumDeletes       = 0;
    m_cycleMouseCount       = 0;
    m_cycleMouseNormXSum    = 0.0;
    m_cycleMouseNormYSum    = 0.0;

    if (m_pCycleTimer != NULL)
    {
        delete m_pCycleTimer;
        m_pCycleTimer = NULL;
    }

    m_pCycleTimer = new CSimpleTimer();        
}

// Gets the XML that goes in the <Params> tag, but not the tags themselves.
string CUserLog::GetParamsXML()
{
    string strResult = "";

    // Make parameters with the same name appear near each other in the results
    sort(m_vectorParams.begin(), m_vectorParams.end(), CUserLogParam::ComparePtr);

    for (unsigned int i = 0; i < m_vectorParams.size(); i++)
    {
        CUserLogParam* param = (CUserLogParam*) m_vectorParams[i];

        strResult += CUserLogTrial::GetParamXML(param, "\t\t");
    }

    return strResult;
}

// Prepares a new trial for use.  Passes on the current canvas and window
// size so normalized mouse coordinates can be calculated.  Also 
// parameters can be marked to force them into the Trial object.  Looks for 
// these and push into the current Trial object.
void CUserLog::PrepareNewTrial()
{
    CUserLogTrial* trial = GetCurrentTrial();

    if (trial != NULL)
    {
        // We want to force the current value of any parameters that we marked
        // with the userLogParamForceInTrial option when created.  We can
        // do this by going backwards through the parameter vector and only
        // pushing through the first value of a given parameter name.
        VECTOR_STRING vectorFound;
        for (VECTOR_USER_LOG_PARAM_PTR_ITER iter = m_vectorParams.end() - 1; iter >= m_vectorParams.begin(); iter--)
        {
            if (((*iter) != NULL) && ((*iter)->options & userLogParamForceInTrial))
            {
                // Make sure we haven't output this one already
                VECTOR_STRING_ITER strIter;
                strIter = find(vectorFound.begin(), vectorFound.end(), (*iter)->strName);
                if (strIter == vectorFound.end())
                {
                    trial->AddParam((*iter)->strName, (*iter)->strValue, (*iter)->options);
                    vectorFound.push_back((*iter)->strName);
                }
            }
        }

        // Make sure the trial has the current canvas and window coordinate sizes
        trial->AddCanvasSize(m_canvasCoordinates.top, 
                             m_canvasCoordinates.left, 
                             m_canvasCoordinates.bottom, 
                             m_canvasCoordinates.right);
        
        trial->AddWindowSize(m_windowCoordinates.top, 
                             m_windowCoordinates.left, 
                             m_windowCoordinates.bottom, 
                             m_windowCoordinates.right);

    }
    else
        gLogger->Log("CUserLog::PrepareNewTrial, failed to create CUserLogTrial", logNORMAL);
}

// Parameters can be marked to always end them at the cycle stats in short logging.
// We'll look through our parameters and return a tab delimited list of their
// values.
string CUserLog::GetCycleParamStats()
{
    string strResult = "";
    VECTOR_STRING vectorFound;

    // We may have more than one parameter that needs to be added and we want
    // the stats line to be invariant to the order in which AddParam() was 
    // called.  So we'll sort by param name and then by time stamp (for 
    // parameters with multiple values).
    sort(m_vectorParams.begin(), m_vectorParams.end(), CUserLogParam::ComparePtr);

    // Find the last instance of any parameter marked as needed to be on
    // the cycle stats line.
    for (VECTOR_USER_LOG_PARAM_PTR_ITER iter = m_vectorParams.end() - 1; iter >= m_vectorParams.begin(); iter--)
    {
        if (((*iter) != NULL) && ((*iter)->options & userLogParamShortInCycle))
        {
            // Make sure we haven't output this one already
            VECTOR_STRING_ITER strIter;
            strIter = find(vectorFound.begin(), vectorFound.end(), (*iter)->strName);
            if (strIter == vectorFound.end())
            {
                strResult += "\t";
                strResult += (*iter)->strValue;
                vectorFound.push_back((*iter)->strName);
            }
        }
    }

    return strResult;
}

// Return a string with the operating system and product version
string CUserLog::GetVersionInfo()
{
    string strResult = "";
#ifdef _WIN32
    strResult += "win ";
    
    // TBD: getting version from resource is quite tricky and requires linking in 
    // a whole library to do.  Maybe we can just #DEFINE the product version?
#else
    strResult += "not win ";
#endif

    return strResult;
}

///////////////////////////////////////////////////////////////////////////////////////
// Below here are methods that are just used in the standalone tool that reads in 
// UserLog XML files and does cool things to them.  No real need for this code
// to be compiled into the mainline Dasher application.

#ifdef USER_LOG_TOOL

// Load the object from an XML file
CUserLog::CUserLog(string strXMLFilename) 
{
    InitMemberVars();

    // We are representing detailed logging when we create from XML
    m_bDetailed = true;

    VECTOR_STRING vectorTrials;

    // First split up various parts of the XML
    string strXML           = XMLUtil::LoadFile(strXMLFilename);        
    string strApp           = XMLUtil::GetElementString("Application", strXML, true);    
    string strParams        = XMLUtil::GetElementString("Params", strXML, true);           
    string strTrials        = XMLUtil::GetElementString("Trials", strXML, true);
    vectorTrials            = XMLUtil::GetElementStrings("Trial", strTrials, true);

//gLogger->Log("trials:\n%s", logDEBUG, strTrials.c_str());

    m_pApplicationSpan      = new CTimeSpan("Application", strApp);
    m_vectorParams          = CUserLogTrial::ParseParamsXML(strParams);

    // Now construct each of the Trial objects based on its section of XML
    for (VECTOR_STRING_ITER iter = vectorTrials.begin(); iter < vectorTrials.end(); iter++)
    {
        if (iter != NULL)
        {
            CUserLogTrial* trial = new CUserLogTrial(*iter);

            if (trial != NULL)
                m_vectorTrials.push_back(trial);
        }
    }

}

#endif

