
#include "UserLogTrial.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CUserLogTrial::CUserLogTrial()
{
    InitMemberVars();
}

CUserLogTrial::~CUserLogTrial()
{

    for (unsigned int i = 0; i < m_vectorParams.size(); i++)
    {
        CUserLogParam* param = (CUserLogParam*) m_vectorParams[i];

        if (param != NULL)
        {
            delete param;
            param = NULL;
        }
    }

    for (unsigned int i = 0; i < m_vectorNavCycles.size(); i++)
    {
        NavCycle* cycle = (NavCycle*) m_vectorNavCycles[i];

        if (cycle != NULL)
        {
            if (cycle->pSpan != NULL)
            {
                delete cycle->pSpan;
                cycle->pSpan = NULL;
            }

            for (unsigned int i = 0; i < cycle->vectorMouseLocations.size(); i++)
            {
                CUserLocation* location = (CUserLocation*) cycle->vectorMouseLocations[i];

                if (location != NULL)
                {
                    delete location;
                    location = NULL;
                }
            }

            for (unsigned int i = 0; i < cycle->vectorNavLocations.size(); i++)
            {
                NavLocation* location = (NavLocation*) cycle->vectorNavLocations[i];
                if (location != NULL)
                {
                    CTimeSpan* span = location->span;

                    if (span != NULL)
                    {
                        delete span;
                        span = NULL;
                    }

                    Dasher::VECTOR_SYMBOL_PROB_DISPLAY* pVectorAdded = location->pVectorAdded;
                    if (pVectorAdded != NULL)
                    {
                        delete pVectorAdded;
                        pVectorAdded = NULL;
                    }

                    delete location;
                    location = NULL;
                }
            }

            delete cycle;
            cycle = NULL;
        }


    }

    if (m_pSpan!= NULL)
    {
        delete m_pSpan;
        m_pSpan = NULL;
    }

}

// Returns an XML version of all our information
string CUserLogTrial::GetXML(const string& prefix)
{
    string strResult = "";

    string strPrefixTab = prefix;
    strPrefixTab += "\t";

    string strPrefixTabTab = strPrefixTab;
    strPrefixTabTab += "\t";

    strResult += prefix;
    strResult += "<Trial>\n";

    // Summarize what happened at the beginning of the block
    strResult += GetSummaryXML(prefix);

    // Parameters that we want tracked on a per trial basis
    strResult += GetParamsXML(prefix);
    
    // Size of the window and the canvas in screen coordniates
    strResult += GetWindowCanvasXML(prefix);

    strResult += m_strCurrentTrial;

    // All the start and stop navigation events
    strResult += GetNavCyclesXML(prefix);

    strResult += prefix;
    strResult += "</Trial>\n";

    return strResult;
}

// See if any navigation has occured during this trial yet
bool CUserLogTrial::HasWritingOccured()
{
    if (m_vectorNavCycles.size() > 0) 
        return true;
    return false;
}



void CUserLogTrial::StartWriting()
{
    if (m_bWritingStart)
    {
        gLogger->Log("CUserLogTrial::StartWriting, nav already marked as started!", logNORMAL);
        return;
    }

    // Make sure our trial time span is running
    if (m_pSpan != NULL) 
        m_pSpan->Continue();

    // Start the task timer if we haven't already done so
    if (m_pSpan == NULL)
        m_pSpan = new CTimeSpan("Time", false);

    m_bWritingStart = true;

    // If we have already done some navigation, then the previous NavStop() would have stopped
    // the timer in the last NavLocation object.  We want to tell it to continue since the 
    // trial is not in fact over.
    if (m_vectorNavCycles.size() > 0)
    {
        NavLocation* lastLocation = GetCurrentNavLocation();
        if ((lastLocation != NULL) && (lastLocation->span != NULL))
            lastLocation->span->Continue();
    }

    NavCycle* newCycle = AddNavCycle();

}

void CUserLogTrial::StopWriting()
{
    if (!m_bWritingStart)
    {
        gLogger->Log("CUserLogTrial::StopWriting, nav already marked as stopped!", logNORMAL);
        return;
    }

    if (m_vectorNavCycles.size() <= 0)
    {
        gLogger->Log("CUserLogTrial::StopWriting, vector was empty!", logNORMAL);
        return;
    }

    NavCycle* cycle = GetCurrentNavCycle();
    if (cycle == NULL)
    {
        gLogger->Log("CUserLogTrial::StopWriting, current cycle was NULL!", logNORMAL);
        return;
    }

    CTimeSpan* span = (CTimeSpan*) cycle->pSpan;   
    if (span == NULL)
    {
        gLogger->Log("CUserLogTrial::StopWriting, span was NULL!", logNORMAL);
        return;
    }
    span->Stop();

    m_bWritingStart = false;

    // Make sure the last location timer is stopped since this could be the end of the trial and
    // we want the timestamps in the location elements to match the total trial time.
    NavLocation* lastLocation = GetCurrentNavLocation();
    if ((lastLocation != NULL) && (lastLocation->span != NULL))
        lastLocation->span->Continue();

    // Could be the last event of the trial
    if (m_pSpan != NULL) 
        m_pSpan->Stop();

    // We want to use the UserTrial info from the navigation period in Dasher.  We'll update
    // this everytime the user stops, this should make sure we get the right bit.
    GetUserTrialInfo();
}

// The user has entered one or more new symbols.  UserLog object will
// pass us the pointer to the current alphabet that is being used.
void CUserLogTrial::AddSymbols(Dasher::VECTOR_SYMBOL_PROB* vectorNewSymbolProbs, eUserLogEventType event, Dasher::CAlphabet* pCurrentAlphabet)
{
    if (pCurrentAlphabet == NULL)
    {
        gLogger->Log("CUserLogTrial::AddSymbols, pCurrentAlphabet was NULL!", logNORMAL);
        return;
    }
    if (vectorNewSymbolProbs == NULL)
    {
        gLogger->Log("CUserLogTrial::AddSymbols, vectorNewSymbolProbs was NULL!", logNORMAL);
        return;
    }

    Dasher::VECTOR_SYMBOL_PROB_DISPLAY* pVectorAdded = NULL;
    pVectorAdded = new Dasher::VECTOR_SYMBOL_PROB_DISPLAY;

    for (unsigned int i = 0; i < vectorNewSymbolProbs->size(); i++)
    {
        Dasher::SymbolProb newSymbolProb = (Dasher::SymbolProb) (*vectorNewSymbolProbs)[i];

        Dasher::SymbolProbDisplay newSymbolProbDisplay;
        newSymbolProbDisplay.sym        = newSymbolProb.sym;
        newSymbolProbDisplay.prob       = newSymbolProb.prob;
        newSymbolProbDisplay.strDisplay = pCurrentAlphabet->GetDisplayText(newSymbolProb.sym);

        pVectorAdded->push_back(newSymbolProbDisplay);

        // Add this symbol to our running total of symbols.  
        // We track the symbols and not the display text
        // since we may need to delete symbols later and 
        // a given symbol might take up multiple chars.
        // We also keep the probability around so we can
        // calculate the average bits of the history.
        m_vectorHistory.push_back(newSymbolProbDisplay);        
    }

    StopPreviousTimer();

    // Create the new NavLocation struct that record the data about this addition
    NavLocation* location = NULL;

    location                = new NavLocation;

    location->strHistory    = GetHistoryDisplay();
    location->span          = new CTimeSpan("Time", false);
    location->avgBits       = GetHistoryAvgBits();
    location->event         = event;
    location->numDeleted    = 0;
    location->pVectorAdded  = pVectorAdded;

    NavCycle* cycle = GetCurrentNavCycle();
    if (cycle != NULL)
        cycle->vectorNavLocations.push_back(location);
    else
        gLogger->Log("CUserLogTrial::AddSymbols, cycle was NULL!", logNORMAL);

}

void CUserLogTrial::DeleteSymbols(int numToDelete, eUserLogEventType event)
{
    if (numToDelete <= 0)
        return;

    for (int i = 0; i < numToDelete; i++)
    {
        // Remove the request number of symbols from our
        // ongoing list.
        m_vectorHistory.pop_back();
    }

    StopPreviousTimer();

    // Create the new NavLocation struct that record the data about this addition
    NavLocation* location = NULL;

    location                = new NavLocation;

    location->strHistory    = GetHistoryDisplay();
    location->span          = new CTimeSpan("Time", false);
    location->avgBits       = GetHistoryAvgBits();
    location->event         = event;
    location->numDeleted    = numToDelete;
    location->pVectorAdded  = NULL;

    NavCycle* cycle = GetCurrentNavCycle();
    if (cycle != NULL)
        cycle->vectorNavLocations.push_back(location);
    else
        gLogger->Log("CUserLogTrial::DeleteSymbols, cycle was NULL!", logNORMAL);

}


// Called by UserLog object whenever we move on to the next trial.  This lets
// our trial object finalize any timers.
void CUserLogTrial::Done()
{
    StopPreviousTimer();

    // Stop the time span that tracks the total trial time (if not already stopped)
   if ((m_pSpan != NULL) && (!m_pSpan->IsStopped()))
        m_pSpan->Stop();

}


void CUserLogTrial::AddMouseLocation(int x, int y, float nats)
{
    CUserLocation* location = NULL;

    location = new CUserLocation(x, y, nats);

    if (location != NULL)
    {
        // m_vectorMouseLocations.push_back(location);

        NavCycle* cycle = GetCurrentNavCycle();

        if (cycle != NULL)
            cycle->vectorMouseLocations.push_back(location);
        else
            gLogger->Log("CUserLogTrial::AddLocation, cycle was NULL!", logNORMAL);
    }
    else
        gLogger->Log("CUserLogTrial::AddLocation, location was NULL!", logNORMAL);

}

// Adds a normalized version of our mouse coordinates based on the size
// of the window.  Can optionally be told to store both representations.
void CUserLogTrial::AddMouseLocationNormalized(int x, int y, bool bStoreIntegerRep, float nats)
{
    CUserLocation* location = NULL;

    if ((m_canvasCoordinates.bottom == 0) &&
        (m_canvasCoordinates.left == 0) &&
        (m_canvasCoordinates.right == 0) &&
        (m_canvasCoordinates.top == 0))
        gLogger->Log("CUserLogTrial::AddMouseLocationNormalized, called before AddCanvasSize()?", logNORMAL);

    location = new CUserLocation(x, 
                                 y, 
                                 m_canvasCoordinates.top, 
                                 m_canvasCoordinates.left, 
                                 m_canvasCoordinates.bottom, 
                                 m_canvasCoordinates.right, 
                                 bStoreIntegerRep, 
                                 nats);

    if (location != NULL)
    {
        // m_vectorMouseLocations.push_back(location);
        NavCycle* cycle = GetCurrentNavCycle();

        if (cycle != NULL)
            cycle->vectorMouseLocations.push_back(location);
        else
            gLogger->Log("CUserLogTrial::AddMouseLocationNormalized, cycle was NULL!", logNORMAL);
    }
    else
        gLogger->Log("CUserLogTrial::AddLocation, location was NULL!", logNORMAL);    
}

// Sets the current window size, this includes area for the menu bar, 
// sliders, canvas, etc.
void CUserLogTrial::AddWindowSize(int top, int left, int bottom, int right)
{
    m_windowCoordinates.top     = top;
    m_windowCoordinates.left    = left;
    m_windowCoordinates.bottom  = bottom;
    m_windowCoordinates.right   = right;
}

// Sets the current canvas size
void CUserLogTrial::AddCanvasSize(int top, int left, int bottom, int right)
{
    m_canvasCoordinates.top     = top;
    m_canvasCoordinates.left    = left;
    m_canvasCoordinates.bottom  = bottom;
    m_canvasCoordinates.right   = right;
}

// Are we currently navigating?
bool CUserLogTrial::IsWriting()
{
    return m_bWritingStart;
}

////////////////////////////////////////// private methods ////////////////////////////////////////////////

void CUserLogTrial::InitMemberVars()
{
    m_bWritingStart                 = false;
    m_pSpan                         = NULL;
    m_strCurrentTrial               = "";
    
    m_windowCoordinates.bottom      = 0;
    m_windowCoordinates.top         = 0;
    m_windowCoordinates.left        = 0;
    m_windowCoordinates.right       = 0;

    m_canvasCoordinates.bottom      = 0;
    m_canvasCoordinates.top         = 0;
    m_canvasCoordinates.left        = 0;
    m_canvasCoordinates.right       = 0;
}

// Obtain information that is being passed in from the UserTrial standalone application.
// This information tell us what the user is actually trying to enter.
void CUserLogTrial::GetUserTrialInfo()
{
    m_strCurrentTrial = "";

    try
    {

        fstream fin("CurrentTrial.xml", ios::in);       // We want ios::nocreate, but not available in .NET 2003, arrgh
        
        // Make sure we successfully opened before we start reading it
        if (fin.is_open())
        {
            char str[1024];
            while(!fin.eof()) 
            {
                fin.getline(str, 1024);
                if (strlen(str) > 0)
                {
                    m_strCurrentTrial += "\t\t\t";
                    m_strCurrentTrial += str;
                    m_strCurrentTrial += "\n";
                }        
            }
            fin.close();
        }
    } catch (...)
    {
        // The application might not be running in which case the read will fail.
    }
}

// Returns the concatenation of all our symbol history using
// the display text that the alphabet at the time of the 
// symbol being added gave us.  
string CUserLogTrial::GetHistoryDisplay()
{
    string strResult = "";

    for (unsigned int i = 0; i < m_vectorHistory.size(); i++)
    {
        Dasher::SymbolProbDisplay item = (Dasher::SymbolProbDisplay) m_vectorHistory[i];
        strResult += item.strDisplay;
    }

    return strResult;
}

double CUserLogTrial::GetHistoryAvgBits()
{
    double result = 0.0;

    if (m_vectorHistory.size() > 0)
    {
        for (unsigned int i = 0; i < m_vectorHistory.size(); i++)
        {
            Dasher::SymbolProbDisplay item = (Dasher::SymbolProbDisplay) m_vectorHistory[i];

            result += log(item.prob);
        }
        result = result * -1.0;
        result = result / log(2.0);
        result = result / m_vectorHistory.size();
    }

    return result;
}

void CUserLogTrial::StopPreviousTimer()
{
    // Make sure the previous time span (if any) has had its timer stopped
    if (m_vectorNavCycles.size() > 0)
    {
        NavLocation* lastLocation = GetCurrentNavLocation();
        if ((lastLocation != NULL) && (lastLocation->span != NULL))
            lastLocation->span->Stop();
    }

}

// Gets XML string for a given NavLocation struct
string CUserLogTrial::GetLocationXML(NavLocation* location, const string& prefix)
{
    string strResult = "";
    if (location == NULL)
    {
        gLogger->Log("CUserLogTrial::GetLocationXML, location was NULL!", logNORMAL);
        return strResult;
    }

    char strNum[256];

    strResult += prefix;
    strResult += "<Location>\n";

    strResult += prefix;
    strResult += "\t<History>";
    strResult += location->strHistory;
    strResult += "</History>\n";

    strResult += prefix;
    strResult += "\t<AvgBits>";
    sprintf(strNum, "%0.6f", location->avgBits);
    strResult += strNum;
    strResult += "</AvgBits>\n";

    // Only output the event if it is interesting type, not normal mouse navigation
    if (location->event != userLogEventMouse)
    {
        strResult += prefix;
        strResult += "\t\t<Event>";
        sprintf(strNum, "%d", (int) location->event);
        strResult += strNum;
        strResult += "</Event>\n";                
    }

    if ((location->pVectorAdded != NULL) && (location->pVectorAdded->size() > 0))
    {
        strResult += prefix;
        strResult += "\t<NumAdded>";
        sprintf(strNum, "%d", location->pVectorAdded->size());
        strResult += strNum;
        strResult += "</NumAdded>\n";

        Dasher::VECTOR_SYMBOL_PROB_DISPLAY* pVectorAdded = location->pVectorAdded;

        // Output the details of each add
        for (unsigned int j = 0; j < pVectorAdded->size(); j++)
        {            
            Dasher::SymbolProbDisplay item = (Dasher::SymbolProbDisplay) (*pVectorAdded)[j];

            strResult += prefix;
            strResult += "\t<Add>\n";
            
            strResult += prefix;
            strResult += "\t\t<Text>";
            strResult += item.strDisplay;
            strResult += "</Text>\n";

            strResult += prefix;
            strResult += "\t\t<Prob>";
            sprintf(strNum, "%0.6f", item.prob);
            strResult += strNum;
            strResult += "</Prob>\n";

            strResult += prefix;
            strResult += "\t</Add>\n";
        }

    }

    if (location->numDeleted > 0)
    {
        strResult += prefix;
        strResult += "\t<NumDeleted>";
        sprintf(strNum, "%d", location->numDeleted);
        strResult += strNum;
        strResult += "</NumDeleted>\n";
    }

    string strPrefixTabTabTab = prefix;
    strPrefixTabTabTab += "\t";

    strResult += location->span->GetXML(strPrefixTabTabTab);

    strResult += prefix;
    strResult += "</Location>\n";

    return strResult;
}

// Output the XML for the summary section of XML
string CUserLogTrial::GetSummaryXML(const string& prefix)
{
    string strResult = "";

    strResult += prefix;
    strResult += "\t<Summary>\n";

    // Figure out what the user ended up writing and how fast they did it
    string strText = "";
    double avgBits = 0.0;

    NavLocation* location = GetCurrentNavLocation();
    if (location != NULL)
    {
        strText = GetHistoryDisplay();
        avgBits = location->avgBits;
    }

    strResult += GetStatsXML(prefix, strText, m_pSpan, avgBits);

    strResult += prefix;
    strResult += "\t</Summary>\n";

    return strResult;
}

// Calculates the various summary stats we output
string CUserLogTrial::GetStatsXML(const string& prefix, const string& strText, CTimeSpan* pSpan, double avgBits)
{
    string strResult = "";
    char strNum[256];

    if (pSpan == NULL)
    {
        gLogger->Log("CUserLogTrial::GetStatsXML, pSpan = NULL!", logNORMAL);
        return strResult;
    }

    strResult += prefix;
    strResult += "\t\t<Text>";
    strResult += strText;
    strResult += "</Text>\n";

    // Average number of bits along the path to the final string
    strResult += prefix;
    strResult += "\t\t<AvgBits>";
    sprintf(strNum, "%0.6f", avgBits);
    strResult += strNum;
    strResult += "</AvgBits>\n";

    // Calculate the number of words and characters
    strResult += prefix;
    strResult += "\t\t<Chars>";
    
    // We want the number of symbols which might differ
    // from the actual length of the text history.
    int numChars = m_vectorHistory.size();
    sprintf(strNum, "%d", numChars);
    strResult += strNum;
    strResult += "</Chars>\n";

    strResult += prefix;
    strResult += "\t\t<Words>";
    double numWords = (double) numChars / (double) 5;
    sprintf(strNum, "%0.2f", numWords);
    strResult += strNum;
    strResult += "</Words>\n";

    double wpm  = (double) numWords / (m_pSpan->GetElapsed() / 60.0);
    double cpm  = (double) numChars / (m_pSpan->GetElapsed() / 60.0);

    strResult += prefix;
    strResult += "\t\t<WPM>";
    sprintf(strNum, "%0.3f", wpm);
    strResult += strNum;
    strResult += "</WPM>\n";

    strResult += prefix;
    strResult += "\t\t<CPM>";
    sprintf(strNum, "%0.3f", cpm);
    strResult += strNum;
    strResult += "</CPM>\n";

    string strPrefixTabTab = prefix;
    strPrefixTabTab += "\t\t";

    strResult += m_pSpan->GetXML(strPrefixTabTab);

    return strResult;
}

string CUserLogTrial::GetWindowCanvasXML(const string& prefix)
{
    string strResult = "";
    char strNum[256];

    // Log the window location and size that was last used during this trial
    strResult += prefix;
    strResult += "\t<WindowCoordinates>\n";

    strResult += prefix;
    sprintf(strNum, "\t\t<Top>%d</Top>\n", m_windowCoordinates.top);
    strResult += strNum;

    strResult += prefix;
    sprintf(strNum, "\t\t<Bottom>%d</Bottom>\n", m_windowCoordinates.bottom);
    strResult += strNum;

    strResult += prefix;
    sprintf(strNum, "\t\t<Left>%d</Left>\n", m_windowCoordinates.left);
    strResult += strNum;

    strResult += prefix;
    sprintf(strNum, "\t\t<Right>%d</Right>\n", m_windowCoordinates.right);
    strResult += strNum;

    strResult += prefix;
    strResult += "\t</WindowCoordinates>\n";

    // Log the canvas location and size that was last used during this trial
    strResult += prefix;
    strResult += "\t<CanvasCoordinates>\n";

    strResult += prefix;
    sprintf(strNum, "\t\t<Top>%d</Top>\n", m_canvasCoordinates.top);
    strResult += strNum;

    strResult += prefix;
    sprintf(strNum, "\t\t<Bottom>%d</Bottom>\n", m_canvasCoordinates.bottom);
    strResult += strNum;

    strResult += prefix;
    sprintf(strNum, "\t\t<Left>%d</Left>\n", m_canvasCoordinates.left);
    strResult += strNum;

    strResult += prefix;
    sprintf(strNum, "\t\t<Right>%d</Right>\n", m_canvasCoordinates.right);
    strResult += strNum;

    strResult += prefix;
    strResult += "\t</CanvasCoordinates>\n";

    return strResult;
}

string CUserLogTrial::GetParamsXML(const string& prefix)
{
    string strResult = "";

    // Make parameters with the same name appear near each other in the results
    sort(m_vectorParams.begin(), m_vectorParams.end(), CUserLogParam::ComparePtr);    

    if (m_vectorParams.size() > 0)
    {
        strResult += prefix;
        strResult += "\t<Params>\n";

        string strPrefixPlusTabTab = prefix;
        strPrefixPlusTabTab += "\t\t";

        for (unsigned int i = 0; i < m_vectorParams.size(); i++)
        {
            CUserLogParam* param = (CUserLogParam*) m_vectorParams[i];

            strResult += GetParamXML(param, strPrefixPlusTabTab);
        }

        strResult += prefix;
        strResult += "\t</Params>\n";
    }

    return strResult;

}

// Parameters can optionally be specified to be added to the Trial objects.
// This allows us to easily see what a certain parameter value was used
// in a given trial.  
void CUserLogTrial::AddParam(const string& strName, const string& strValue, int optionMask)
{
    bool bTrackMultiple     = false;

    if (optionMask & userLogParamTrackMultiple)
        bTrackMultiple = true;

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

    // Parameters that can have multiple values logged will also log when they were changed
    if (bTrackMultiple)
        newParam->strTimeStamp  = CTimeSpan::GetTimeStamp();

    m_vectorParams.push_back(newParam);
}

// Static method that generates the XML representation of a 
// single param name value set.  Used both to output params
// for a trial and for the parent UserLog object.
string CUserLogTrial::GetParamXML(CUserLogParam* param, const string& strPrefix)
{
    string strResult = "";

    if (param != NULL)
    {
        strResult += strPrefix;
        strResult += "<";
        strResult += param->strName;
        strResult += ">";

        if (param->strTimeStamp.length() > 0)
        {
            strResult += "\n";
            strResult += strPrefix;
            strResult += "\t<Value>";
            strResult += param->strValue;
            strResult += "</Value>\n";

            strResult += strPrefix;
            strResult += "\t<Time>";
            strResult += param->strTimeStamp;
            strResult += "</Time>\n";

            strResult += strPrefix;
        }
        else 
        {
            strResult += param->strValue;
        }

        strResult += "</";
        strResult += param->strName;
        strResult += ">\n";
    }

    return strResult;
}

// Returns a pointer to the currently active navigation cycle
NavCycle* CUserLogTrial::GetCurrentNavCycle()
{
    if (m_vectorNavCycles.size() <= 0)
        return NULL;
    return m_vectorNavCycles[m_vectorNavCycles.size() - 1];
}

// Gets a pointer to the last NavLocation object
// in the current navication cycle.
NavLocation* CUserLogTrial::GetCurrentNavLocation()
{
    NavCycle* cycle = GetCurrentNavCycle();

    if (cycle == NULL)
        return NULL;

    if (cycle->vectorNavLocations.size() <= 0)
        return NULL;

    return (NavLocation*) cycle->vectorNavLocations[cycle->vectorNavLocations.size() - 1];
}

// Adds a new navgiation cycle to our collection
NavCycle* CUserLogTrial::AddNavCycle()
{
    NavCycle* newCycle = new NavCycle;
    newCycle->pSpan = new CTimeSpan("Time", false);

    m_vectorNavCycles.push_back(newCycle);
    return newCycle;
}

string CUserLogTrial::GetNavCyclesXML(const string& strPrefix)
{
    string strResult = "";

    string strPrefixTab = strPrefix;
    strPrefixTab += "\t";

    string strPrefixTabTab = strPrefixTab;
    strPrefixTabTab += "\t";

    string strPrefixTabTabTab = strPrefixTabTab;
    strPrefixTabTabTab += "\t";

    string strPrefixTabTabTabTab = strPrefixTabTabTab;
    strPrefixTabTabTabTab += "\t";

    strResult += strPrefixTab;
    strResult += "<Navs>\n";

    for (unsigned int i = 0; i < m_vectorNavCycles.size(); i++)
    {
        NavCycle* cycle = (NavCycle*) m_vectorNavCycles[i];

        if (cycle != NULL)
        {
            strResult += strPrefixTabTab;
            strResult += "<Nav>\n";

            if (cycle->pSpan != NULL)
                strResult += cycle->pSpan->GetXML(strPrefixTabTabTab);

            if (cycle->vectorNavLocations.size() > 0)
            {
                strResult += strPrefixTabTabTab;
                strResult += "<Locations>\n";

                for (unsigned int i = 0; i < cycle->vectorNavLocations.size(); i++)
                {
                    NavLocation* location = (NavLocation*) cycle->vectorNavLocations[i];

                    if (location != NULL)
                        strResult += GetLocationXML(location, strPrefixTabTabTabTab);
                }
                strResult += strPrefixTabTabTab;
                strResult += "</Locations>\n";
            }

            if (cycle->vectorMouseLocations.size() > 0)
            {
                strResult += strPrefixTabTabTab;
                strResult += "<MousePositions>\n";

                for (unsigned int i = 0; i < cycle->vectorMouseLocations.size(); i++)
                {
                    CUserLocation* location = (CUserLocation*) cycle->vectorMouseLocations[i];

                    if (location != NULL)
                    {
                        strResult += location->GetXML(strPrefixTabTabTabTab);
                    }
                }

                strResult += strPrefixTabTabTab;
                strResult += "</MousePositions>\n";
            }


            strResult += strPrefixTabTab;
            strResult += "</Nav>\n";

        }
    }

    strResult += strPrefixTab;
    strResult += "</Navs>\n";

    return strResult;
}

#ifdef USER_LOG_TOOL

// Construct based on some XML
CUserLogTrial::CUserLogTrial(const string& strXML)
{
    InitMemberVars();
    VECTOR_STRING vectorNavs;

    string strParams        = XMLUtil::GetElementString("Params", strXML, true);           
    string strWindow        = XMLUtil::GetElementString("WindowCoordinates", strXML, true);           
    string strCanvas        = XMLUtil::GetElementString("CanvasCoordinates", strXML, true);           
    string strNavs          = XMLUtil::GetElementString("Navs", strXML, true);           
    string strSummary       = XMLUtil::GetElementString("Summary", strXML, true);
    string strSummaryTime   = XMLUtil::GetElementString("Time", strSummary, true);
    vectorNavs              = XMLUtil::GetElementStrings("Nav", strNavs, true);

    m_vectorParams          = ParseParamsXML(strParams);
    m_windowCoordinates     = ParseWindowXML(strWindow);
    m_canvasCoordinates     = ParseWindowXML(strCanvas);

    m_pSpan                 = new CTimeSpan("Time", strSummaryTime);

    // Process each <Nav> tag
    string strTime              = "";
    string strLocations         = "";
    string strMousePositions    = "";

    VECTOR_STRING vectorLocations;
    VECTOR_STRING vectorMousePositions;
    VECTOR_STRING vectorAdded;

    for (VECTOR_STRING_ITER iter = vectorNavs.begin(); iter < vectorNavs.end(); iter++)
    {
        strTime              = "";
        strLocations         = "";
        strMousePositions    = "";
        vectorLocations.erase(vectorLocations.begin(), vectorLocations.end());
        vectorMousePositions.erase(vectorMousePositions.begin(), vectorMousePositions.end());

        if (iter != NULL)
        {
            strTime             = XMLUtil::GetElementString("Time", *iter, true);
            strLocations        = XMLUtil::GetElementString("Locations", *iter, true);
            strMousePositions   = XMLUtil::GetElementString("MousePositions", *iter, true);
        }

        NavCycle* cycle = new NavCycle();
        cycle->pSpan = NULL;

        if (strTime.length() > 0)
        {
            cycle->pSpan = new CTimeSpan("Time", strTime);
        }
        if (strLocations.length() > 0)
        {
            vectorLocations = XMLUtil::GetElementStrings("Location", strLocations, true);

            for (VECTOR_STRING_ITER iter2 = vectorLocations.begin(); iter2 < vectorLocations.end(); iter2++)
            {
                vectorAdded.erase(vectorAdded.begin(), vectorAdded.end());

                NavLocation* location = new NavLocation();

                location->strHistory    = XMLUtil::GetElementString("History", *iter2);
                location->avgBits       = (double) XMLUtil::GetElementFloat("AvgBits", *iter2);
                location->event         = (eUserLogEventType) XMLUtil::GetElementInt("Event", *iter2);
                location->numDeleted    = XMLUtil::GetElementInt("NumDeleted", *iter2);

                location->span          = NULL;
                string strTime          = XMLUtil::GetElementString("Time", *iter2);
                location->span          = new CTimeSpan("Time", strTime);

                // Handle the multiple <Add> tags that might exist
                vectorAdded             = XMLUtil::GetElementStrings("Add", *iter2);                
                location->pVectorAdded  = new Dasher::VECTOR_SYMBOL_PROB_DISPLAY;
                
                for (VECTOR_STRING_ITER iter3 = vectorAdded.begin(); iter3 < vectorAdded.end(); iter3++)
                {
                    Dasher::SymbolProbDisplay add;

                    add.prob           = XMLUtil::GetElementFloat("Prob", *iter3);
                    add.strDisplay     = XMLUtil::GetElementString("Text", *iter3);
                    add.sym            = 0;    // We don't have the original integer symbol index

                    if (location->pVectorAdded != NULL)
                        location->pVectorAdded->push_back(add);

                    // Also track it in one complete vector of all the adds
                    m_vectorHistory.push_back(add);        
                }

                // If this was a deleted event, then we need to erase some stuff from the running history
                for (int i = 0; i < location->numDeleted; i++)
                    m_vectorHistory.pop_back();

                cycle->vectorNavLocations.push_back(location);
            }
        }
        
        if (strMousePositions.length() > 0)
        {
            vectorMousePositions = XMLUtil::GetElementStrings("Pos", strMousePositions, true);
            for (VECTOR_STRING_ITER iter2 = vectorMousePositions.begin(); iter2 < vectorMousePositions.end(); iter2++)
            {
                if (iter2 != NULL)
                {
                    CUserLocation* location = new CUserLocation(*iter2);
                    cycle->vectorMouseLocations.push_back(location);
                }
            }

        }

        m_vectorNavCycles.push_back(cycle);
    }

}

// Helper that parses parameters out of the XML block, used by UserLog 
// and by UserLogTrial to do the same thing.
VECTOR_USER_LOG_PARAM_PTR CUserLogTrial::ParseParamsXML(const string& strXML)
{
    VECTOR_USER_LOG_PARAM_PTR   vectorResult;
    VECTOR_NAME_VALUE_PAIR      vectorParams;

    vectorParams = XMLUtil::GetNameValuePairs(strXML, true);

    // Handle adding all the name/value parameter pairs.  XML looks like:
    //      <Eyetracker>0</Eyetracker>
    //      <MaxBitRate>
	//	    	<Value>7.0100</Value>
    //			<Time>15:48:53.140</Time>
	//  	</MaxBitRate>
    for (VECTOR_NAME_VALUE_PAIR_ITER iter = vectorParams.begin(); iter < vectorParams.end(); iter++)
    {
        CUserLogParam* param = new CUserLogParam();
        
        if ((param != NULL) && (iter != NULL))
        {
            param->strName      = iter->strName;
        
            // See if we have a type that has a timestamp
            string strValue     = XMLUtil::GetElementString("Value", iter->strValue, true);
            string strTime      = XMLUtil::GetElementString("Time", iter->strValue, true);

            if ((strValue.length() > 0)  || (strTime.length() > 0))
            {
                param->strValue     = strValue;
                param->strTimeStamp = strTime;
            }
            else
                param->strValue     = iter->strValue;
            
            param->options = 0;

            vectorResult.push_back(param);
        }
    }

    return vectorResult;
}

// Parse our window or canvas coorindates from XML
WindowSize CUserLogTrial::ParseWindowXML(const string& strXML)
{
    WindowSize result;

    result.top      = XMLUtil::GetElementInt("Top", strXML);
    result.bottom   = XMLUtil::GetElementInt("Bottom", strXML);
    result.left     = XMLUtil::GetElementInt("Left", strXML);
    result.right    = XMLUtil::GetElementInt("Right", strXML);

    return result;
}

#endif


