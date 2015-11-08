#include "../Common/Common.h"

#include <cstring>
#include "UserLogTrial.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CUserLogTrial::CUserLogTrial(const string& strCurrentTrialFilename)
{
  //CFunctionLogger f1("CUserLogTrial::CUserLogTrial", g_pLogger);

  InitMemberVars();

  m_strCurrentTrialFilename = strCurrentTrialFilename;
}

CUserLogTrial::~CUserLogTrial()
{
  //CFunctionLogger f1("CUserLogTrial::~CUserLogTrial", g_pLogger);

  for (unsigned int i = 0; i < m_vpParams.size(); i++)
  {
    CUserLogParam* pParam = (CUserLogParam*) m_vpParams[i];

    if (pParam != NULL)
    {
      delete pParam;
      pParam = NULL;
    }
  }

  for (unsigned int i = 0; i < m_vpNavCycles.size(); i++)
  {
    NavCycle* pCycle = (NavCycle*) m_vpNavCycles[i];

    if (pCycle != NULL)
    {
      if (pCycle->pSpan != NULL)
      {
        delete pCycle->pSpan;
        pCycle->pSpan = NULL;
      }

      for (unsigned int i = 0; i < pCycle->vectorButtons.size(); i++)
      {
        CUserButton* pButton = (CUserButton*) pCycle->vectorButtons[i];

        if (pButton != NULL)
        {
          delete pButton;
          pButton = NULL;
        }
      }

      for (unsigned int i = 0; i < pCycle->vectorMouseLocations.size(); i++)
      {
        CUserLocation* pLocation = (CUserLocation*) pCycle->vectorMouseLocations[i];

        if (pLocation != NULL)
        {
          delete pLocation;
          pLocation = NULL;
        }
      }

      for (unsigned int i = 0; i < pCycle->vectorNavLocations.size(); i++)
      {
        NavLocation* pLocation = (NavLocation*) pCycle->vectorNavLocations[i];
        if (pLocation != NULL)
        {
          CTimeSpan* pSpan = pLocation->span;

          if (pSpan != NULL)
          {
            delete pSpan;
            pSpan = NULL;
          }

          Dasher::VECTOR_SYMBOL_PROB* pVectorAdded = pLocation->pVectorAdded;
          if (pVectorAdded != NULL)
          {
            delete pVectorAdded;
            pVectorAdded = NULL;
          }

          delete pLocation;
          pLocation = NULL;
        }
      }

      delete pCycle;
      pCycle = NULL;
    }


  }

  if (m_pSpan!= NULL)
  {
    delete m_pSpan;
    m_pSpan = NULL;
  }

}

// Returns an XML version of all our information
string CUserLogTrial::GetXML(const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetXML", g_pLogger);

  string strResult = "";

  string strPrefixTab = strPrefix;
  strPrefixTab += "\t";

  string strPrefixTabTab = strPrefixTab;
  strPrefixTabTab += "\t";

  strResult += strPrefix;
  strResult += "<Trial>\n";

  // Summarize what happened at the beginning of the block
  strResult += GetSummaryXML(strPrefix);

  // Parameters that we want tracked on a per trial basis
  strResult += GetParamsXML(strPrefix);

  // Size of the window and the canvas in screen coordniates
  strResult += GetWindowCanvasXML(strPrefix);

  strResult += m_strCurrentTrial;

  // All the start and stop navigation events
  strResult += GetNavCyclesXML(strPrefix);

  strResult += strPrefix;
  strResult += "</Trial>\n";

  return strResult;
}

// See if any navigation has occured during this trial yet
bool CUserLogTrial::HasWritingOccured()
{
  //CFunctionLogger f1("CUserLogTrial::HasWritingOccured", g_pLogger);

  if (m_vpNavCycles.size() > 0) 
    return true;
  return false;
}



void CUserLogTrial::StartWriting()
{
  //CFunctionLogger f1("CUserLogTrial::StartWriting", g_pLogger);

  if (m_bWritingStart)
  {
    g_pLogger->Log("CUserLogTrial::StartWriting, nav already marked as started!", logNORMAL);
    return;
  }

  // Make sure our trial time span is running
  if (m_pSpan != NULL) 
    m_pSpan->Continue();

  // Start the task timer if we haven't already done so
  if (m_pSpan == NULL)
    m_pSpan = new CTimeSpan("Time", false);

  if (m_pSpan == NULL) 
  {
    g_pLogger->Log("CUserLogTrial::StartWriting, m_pSpan was NULL!", logNORMAL);
    return;
  }

  m_bWritingStart = true;

  // If we have already done some navigation, then the previous NavStop() would have stopped
  // the timer in the last NavLocation object.  We want to tell it to continue since the 
  // trial is not in fact over.
  if (m_vpNavCycles.size() > 0)
  {
    NavLocation* pLastLocation = GetCurrentNavLocation();
    if ((pLastLocation != NULL) && (pLastLocation->span != NULL))
      pLastLocation->span->Continue();
  }

  //  NavCycle* newCycle = AddNavCycle();
  AddNavCycle();
}

void CUserLogTrial::StopWriting(double dBits)
{
  //CFunctionLogger f1("CUserLogTrial::StopWriting", g_pLogger);

  if (!m_bWritingStart)
  {
    g_pLogger->Log("CUserLogTrial::StopWriting, nav already marked as stopped!", logNORMAL);
    return;
  }

  if (m_vpNavCycles.size() <= 0)
  {
    g_pLogger->Log("CUserLogTrial::StopWriting, vector was empty!", logNORMAL);
    return;
  }

  NavCycle* pCycle = GetCurrentNavCycle();
  if (pCycle == NULL)
  {
    g_pLogger->Log("CUserLogTrial::StopWriting, current cycle was NULL!", logNORMAL);
    return;
  }

  CTimeSpan* pSpan = (CTimeSpan*) pCycle->pSpan;   
  if (pSpan == NULL)
  {
    g_pLogger->Log("CUserLogTrial::StopWriting, span was NULL!", logNORMAL);
    return;
  }

  pCycle->dBits = dBits;
  pSpan->Stop();

  m_bWritingStart = false;

  // Make sure the last location timer is stopped since this could be the end of the trial and
  // we want the timestamps in the location elements to match the total trial time.
  NavLocation* pLastLocation = GetCurrentNavLocation();
  if ((pLastLocation != NULL) && (pLastLocation->span != NULL))
    pLastLocation->span->Continue();

  // Could be the last event of the trial
  if (m_pSpan != NULL) 
    m_pSpan->Stop();

  // We want to use the UserTrial info from the navigation period in Dasher.  We'll update
  // this everytime the user stops, this should make sure we get the right bit.
  GetUserTrialInfo();
}

// The user has entered one or more new symbols.  UserLog object will
// pass us the pointer to the current alphabet that is being used.
void CUserLogTrial::AddSymbols(Dasher::VECTOR_SYMBOL_PROB* vpNewSymbolProbs, 
                               eUserLogEventType iEvent)
{
  // Add the symbols to our running total of symbols.
  
  // ACL: Old comment said "We track the symbols and not the display text
  // since we may need to delete symbols later and 
  // a given symbol might take up multiple chars."
  //   - yet stored the display text????
  
  // We also keep the probability around so we can
  // calculate the average bits of the history.
  m_vHistory.insert(m_vHistory.end(), vpNewSymbolProbs->begin(), vpNewSymbolProbs->end());

  StopPreviousTimer();

  // Create the new NavLocation struct that record the data about this addition
  NavLocation* pLocation  = NULL;
  pLocation               = new NavLocation;

  if (pLocation == NULL)
  {
    g_pLogger->Log("CUserLogTrial::AddSymbols, failed to create location!", logNORMAL);
    return;
  }

  pLocation->strHistory    = GetHistoryDisplay();
  pLocation->span          = new CTimeSpan("Time", false);
  pLocation->avgBits       = GetHistoryAvgBits();
  pLocation->event         = iEvent;
  pLocation->numDeleted    = 0;
  pLocation->pVectorAdded  = new std::vector<Dasher::SymbolProb>(*vpNewSymbolProbs);

  NavCycle* pCycle = GetCurrentNavCycle();
  if (pCycle != NULL)
    pCycle->vectorNavLocations.push_back(pLocation);
  else
    g_pLogger->Log("CUserLogTrial::AddSymbols, cycle was NULL!", logNORMAL);

}

void CUserLogTrial::DeleteSymbols(int iNumToDelete, eUserLogEventType iEvent)
{
  //CFunctionLogger f1("CUserLogTrial::DeleteSymbols", g_pLogger);

  if (iNumToDelete <= 0)
    return;

  // Be careful not to pop more things than we have (this will hork the
  // memory up on linux but not windows).
  int iActualNumToDelete = min((int) m_vHistory.size(), iNumToDelete);

  for (int i = 0; i < iActualNumToDelete; i++)
  {
    // Remove the request number of symbols from our
    // ongoing list.
    m_vHistory.pop_back();
  }

  StopPreviousTimer();

  // Create the new NavLocation struct that record the data about this addition
  NavLocation* pLocation   = NULL;
  pLocation                = new NavLocation;

  if (pLocation == NULL)
  {
    g_pLogger->Log("CUserLogTrial::DeleteSymbols, failed to create location!", logNORMAL);
    return;
  }

  pLocation->strHistory    = GetHistoryDisplay();
  pLocation->span          = new CTimeSpan("Time", false);
  pLocation->avgBits       = GetHistoryAvgBits();
  pLocation->event         = iEvent;
  pLocation->numDeleted    = iNumToDelete;
  pLocation->pVectorAdded  = NULL;

  NavCycle* pCycle = GetCurrentNavCycle();
  if (pCycle != NULL)
    pCycle->vectorNavLocations.push_back(pLocation);
  else
    g_pLogger->Log("CUserLogTrial::DeleteSymbols, cycle was NULL!", logNORMAL);

}


// Called by UserLog object whenever we move on to the next trial.  This lets
// our trial object finalize any timers.
void CUserLogTrial::Done()
{
  //CFunctionLogger f1("CUserLogTrial::Done", g_pLogger);

  StopPreviousTimer();

  // Stop the time span that tracks the total trial time (if not already stopped)
  if ((m_pSpan != NULL) && (!m_pSpan->IsStopped()))
    m_pSpan->Stop();

}


void CUserLogTrial::AddMouseLocation(int iX, int iY, float dNats)
{
  //CFunctionLogger f1("CUserLogTrial::AddMouseLocation", g_pLogger);

  CUserLocation* pLocation = NULL;

  pLocation = new CUserLocation(iX, iY, dNats);

  if (pLocation != NULL)
  {
    // m_vectorMouseLocations.push_back(location);

    NavCycle* pCycle = GetCurrentNavCycle();

    if (pCycle != NULL)
      pCycle->vectorMouseLocations.push_back(pLocation);
    else
      g_pLogger->Log("CUserLogTrial::AddLocation, cycle was NULL!", logNORMAL);
  }
  else
    g_pLogger->Log("CUserLogTrial::AddLocation, location was NULL!", logNORMAL);

}

// Adds a normalized version of our mouse coordinates based on the size
// of the window.  Can optionally be told to store both representations.
void CUserLogTrial::AddMouseLocationNormalized(int iX, int iY, bool bStoreIntegerRep, float dNats)
{
  //CFunctionLogger f1("CUserLogTrial::AddMouseLocationNormalized", g_pLogger);

  CUserLocation* pLocation = NULL;

  if ((m_sCanvasCoordinates.bottom == 0) &&
    (m_sCanvasCoordinates.left == 0) &&
    (m_sCanvasCoordinates.right == 0) &&
    (m_sCanvasCoordinates.top == 0))
    g_pLogger->Log("CUserLogTrial::AddMouseLocationNormalized, called before AddCanvasSize()?", logNORMAL);

  pLocation = new CUserLocation(iX, 
                                iY, 
                                m_sCanvasCoordinates.top, 
                                m_sCanvasCoordinates.left, 
                                m_sCanvasCoordinates.bottom, 
                                m_sCanvasCoordinates.right, 
                                bStoreIntegerRep, 
                                dNats);

  if (pLocation != NULL)
  {
    // m_vectorMouseLocations.push_back(location);
    NavCycle* pCycle = GetCurrentNavCycle();

    if (pCycle != NULL)
      pCycle->vectorMouseLocations.push_back(pLocation);
    else
      g_pLogger->Log("CUserLogTrial::AddMouseLocationNormalized, cycle was NULL!", logNORMAL);
  }
  else
    g_pLogger->Log("CUserLogTrial::AddLocation, location was NULL!", logNORMAL);    
}

void CUserLogTrial::AddKeyDown(int iId, int iType, int iEffect) {
  CUserButton* pButton = new CUserButton(iId, iType, iEffect);

  if(pButton) {
    NavCycle* pCycle = GetCurrentNavCycle();

    if(pCycle)
      pCycle->vectorButtons.push_back(pButton);
    else
      g_pLogger->Log("CUserLogTrial::AddLocation, cycle was NULL!", logNORMAL);
  }
  else
    g_pLogger->Log("CUserLogTrial::AddLocation, location was NULL!", logNORMAL);
}

// Sets the current window size, this includes area for the menu bar, 
// sliders, canvas, etc.
void CUserLogTrial::AddWindowSize(int iTop, int iLeft, int iBottom, int iRight)
{
  //CFunctionLogger f1("CUserLogTrial::AddWindowSize", g_pLogger);

  m_sWindowCoordinates.top     = iTop;
  m_sWindowCoordinates.left    = iLeft;
  m_sWindowCoordinates.bottom  = iBottom;
  m_sWindowCoordinates.right   = iRight;
}

// Sets the current canvas size
void CUserLogTrial::AddCanvasSize(int iTop, int iLeft, int iBottom, int iRight)
{
  //CFunctionLogger f1("CUserLogTrial::AddCanvasSize", g_pLogger);

  m_sCanvasCoordinates.top     = iTop;
  m_sCanvasCoordinates.left    = iLeft;
  m_sCanvasCoordinates.bottom  = iBottom;
  m_sCanvasCoordinates.right   = iRight;
}

// Are we currently navigating?
bool CUserLogTrial::IsWriting()
{
  return m_bWritingStart;
}

////////////////////////////////////////// private methods ////////////////////////////////////////////////

void CUserLogTrial::InitMemberVars()
{
  //CFunctionLogger f1("CUserLogTrial::InitMemberVars", g_pLogger);

  m_bWritingStart                 = false;
  m_pSpan                         = NULL;
  m_strCurrentTrial               = "";

  m_sWindowCoordinates.bottom      = 0;
  m_sWindowCoordinates.top         = 0;
  m_sWindowCoordinates.left        = 0;
  m_sWindowCoordinates.right       = 0;

  m_sCanvasCoordinates.bottom      = 0;
  m_sCanvasCoordinates.top         = 0;
  m_sCanvasCoordinates.left        = 0;
  m_sCanvasCoordinates.right       = 0;
}

// Obtain information that is being passed in from the UserTrial standalone application.
// This information tell us what the user is actually trying to enter.
void CUserLogTrial::GetUserTrialInfo()
{
  //CFunctionLogger f1("CUserLogTrial::GetUserTrialInfo", g_pLogger);

  m_strCurrentTrial = "";

  if (m_strCurrentTrialFilename.length() > 0)
  {
    // We want ios::nocreate, but not available in .NET 2003, arrgh
    fstream fin(m_strCurrentTrialFilename.c_str(), ios::in);

    // Make sure we successfully opened before we start reading it
    if (fin.is_open())
    {
      while (!fin.eof())
      {
        fin.getline(m_szTempBuffer, TEMP_BUFFER_SIZE);
        if (strlen(m_szTempBuffer) > 0)
        {
          m_strCurrentTrial += "\t\t\t";
          m_strCurrentTrial += m_szTempBuffer;
          m_strCurrentTrial += "\n";
        }
      }
      fin.close();
    }
  }
}

// Returns the concatenation of all our symbol history using
// the display text that the alphabet at the time of the 
// symbol being added gave us.  
string CUserLogTrial::GetHistoryDisplay()
{
  //CFunctionLogger f1("CUserLogTrial::GetHistoryDisplay", g_pLogger);

  string strResult = "";

  for (unsigned int i = 0; i < m_vHistory.size(); i++)
  {
    Dasher::SymbolProb sItem = (Dasher::SymbolProb) m_vHistory[i];
    strResult += sItem.strDisplay;
  }

  return strResult;
}

double CUserLogTrial::GetHistoryAvgBits()
{
  //CFunctionLogger f1("CUserLogTrial::GetHistoryAvgBits", g_pLogger);

  double dResult = 0.0;

  if (m_vHistory.size() > 0)
  {
    for (unsigned int i = 0; i < m_vHistory.size(); i++)
    {
      Dasher::SymbolProb sItem = (Dasher::SymbolProb) m_vHistory[i];

      dResult += log(sItem.prob);
    }
    dResult = dResult * -1.0;
    dResult = dResult / log(2.0);
    dResult = dResult / m_vHistory.size();
  }

  return dResult;
}

void CUserLogTrial::StopPreviousTimer()
{
  //CFunctionLogger f1("CUserLogTrial::StopPreviousTimer", g_pLogger);

  // Make sure the previous time span (if any) has had its timer stopped
  if (m_vpNavCycles.size() > 0)
  {
    NavLocation* pLastLocation = GetCurrentNavLocation();
    if ((pLastLocation != NULL) && (pLastLocation->span != NULL))
      pLastLocation->span->Stop();
  }

}

// Gets XML string for a given NavLocation struct
string CUserLogTrial::GetLocationXML(NavLocation* pLocation, const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetLocationXML", g_pLogger);

  string strResult = "";
  if (pLocation == NULL)
  {
    g_pLogger->Log("CUserLogTrial::GetLocationXML, location was NULL!", logNORMAL);
    return strResult;
  }

  strResult += strPrefix;
  strResult += "<Location>\n";

  strResult += strPrefix;
  strResult += "\t<History>";
  strResult += pLocation->strHistory;
  strResult += "</History>\n";

  strResult += strPrefix;
  strResult += "\t<AvgBits>";
  sprintf(m_szTempBuffer, "%0.6f", pLocation->avgBits);
  strResult += m_szTempBuffer;
  strResult += "</AvgBits>\n";

  // Only output the event if it is interesting type, not normal mouse navigation
  if (pLocation->event != userLogEventMouse)
  {
    strResult += strPrefix;
    strResult += "\t\t<Event>";
    sprintf(m_szTempBuffer, "%d", (int) pLocation->event);
    strResult += m_szTempBuffer;
    strResult += "</Event>\n";                
  }

  if ((pLocation->pVectorAdded != NULL) && (pLocation->pVectorAdded->size() > 0))
  {
    strResult += strPrefix;
    strResult += "\t<NumAdded>";
    sprintf(m_szTempBuffer, "%zu", pLocation->pVectorAdded->size());
    strResult += m_szTempBuffer;
    strResult += "</NumAdded>\n";

    Dasher::VECTOR_SYMBOL_PROB* pVectorAdded = pLocation->pVectorAdded;

    if (pVectorAdded != NULL)
    {
      // Output the details of each add
      for (unsigned int j = 0; j < pVectorAdded->size(); j++)
      {            
        Dasher::SymbolProb sItem = (Dasher::SymbolProb) (*pVectorAdded)[j];

        strResult += strPrefix;
        strResult += "\t<Add>\n";

        strResult += strPrefix;
        strResult += "\t\t<Text>";
        strResult += sItem.strDisplay;
        strResult += "</Text>\n";

        strResult += strPrefix;
        strResult += "\t\t<Prob>";
        sprintf(m_szTempBuffer, "%0.6f", sItem.prob);
        strResult += m_szTempBuffer;
        strResult += "</Prob>\n";

        strResult += strPrefix;
        strResult += "\t</Add>\n";
      }
    }
  }

  if (pLocation->numDeleted > 0)
  {
    strResult += strPrefix;
    strResult += "\t<NumDeleted>";
    sprintf(m_szTempBuffer, "%d", pLocation->numDeleted);
    strResult += m_szTempBuffer;
    strResult += "</NumDeleted>\n";
  }

  if (pLocation->span != NULL)
  {
    string strPrefixTabTabTab = strPrefix;
    strPrefixTabTabTab += "\t";

    strResult += pLocation->span->GetXML(strPrefixTabTabTab);
  }

  strResult += strPrefix;
  strResult += "</Location>\n";

  return strResult;
}

// Output the XML for the summary section of XML
string CUserLogTrial::GetSummaryXML(const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetSummaryXML", g_pLogger);

  string strResult = "";

  strResult += strPrefix;
  strResult += "\t<Summary>\n";

  // Figure out what the user ended up writing and how fast they did it
  string strText = "";
  double dAvgBits = 0.0;

  NavLocation* pLocation = GetCurrentNavLocation();
  if (pLocation != NULL)
  {
    strText = GetHistoryDisplay();
    dAvgBits = pLocation->avgBits;
  }

  int iButtonCount = GetButtonCount();
  double dTotalBits = GetTotalBits();

  strResult += GetStatsXML(strPrefix, strText, m_pSpan, dAvgBits, iButtonCount, dTotalBits);

  strResult += strPrefix;
  strResult += "\t</Summary>\n";

  return strResult;
}

// Calculates the various summary stats we output
string CUserLogTrial::GetStatsXML(const string& strPrefix, const string& strText, CTimeSpan* pSpan, double dAvgBits, int iButtonCount, double dTotalBits)
{
  //CFunctionLogger f1("CUserLogTrial::GetStatsXML", g_pLogger);

  string strResult = "";

  if (pSpan == NULL)
  {
    g_pLogger->Log("CUserLogTrial::GetStatsXML, pSpan = NULL!", logNORMAL);
    return strResult;
  }

  strResult += strPrefix;
  strResult += "\t\t<Text>";
  strResult += strText;
  strResult += "</Text>\n";

  // Average number of bits along the path to the final string
  strResult += strPrefix;
  strResult += "\t\t<AvgBits>";
  sprintf(m_szTempBuffer, "%0.6f", dAvgBits);
  strResult += m_szTempBuffer;
  strResult += "</AvgBits>\n";

  strResult += strPrefix;
  strResult += "\t\t<TotalBits>";
  sprintf(m_szTempBuffer, "%0.6f", dTotalBits);
  strResult += m_szTempBuffer;
  strResult += "</TotalBits>\n";

  
  strResult += strPrefix;
  strResult += "\t\t<ButtonCount>";
  sprintf(m_szTempBuffer, "%d", iButtonCount);
  strResult += m_szTempBuffer;
  strResult += "</ButtonCount>\n";

  // Calculate the number of words and characters
  strResult += strPrefix;
  strResult += "\t\t<Chars>";

  // We want the number of symbols which might differ
  // from the actual length of the text history.
  int iNumChars = m_vHistory.size();
  sprintf(m_szTempBuffer, "%d", iNumChars);
  strResult += m_szTempBuffer;
  strResult += "</Chars>\n";

  strResult += strPrefix;
  strResult += "\t\t<Words>";
  double dNumWords = (double) iNumChars / (double) 5;
  sprintf(m_szTempBuffer, "%0.2f", dNumWords);
  strResult += m_szTempBuffer;
  strResult += "</Words>\n";

  double dWPM = 0.0;
  double dCPM = 0.0;

  if (m_pSpan != NULL)
  {
    dWPM  = (double) dNumWords / (m_pSpan->GetElapsed() / 60.0);
    dCPM  = (double) iNumChars / (m_pSpan->GetElapsed() / 60.0);    
  }

  strResult += strPrefix;
  strResult += "\t\t<WPM>";
  sprintf(m_szTempBuffer, "%0.3f", dWPM);
  strResult += m_szTempBuffer;
  strResult += "</WPM>\n";

  strResult += strPrefix;
  strResult += "\t\t<CPM>";
  sprintf(m_szTempBuffer, "%0.3f", dCPM);
  strResult += m_szTempBuffer;
  strResult += "</CPM>\n";

  string strPrefixTabTab = strPrefix;
  strPrefixTabTab += "\t\t";

  if (m_pSpan != NULL)
    strResult += m_pSpan->GetXML(strPrefixTabTab);

  return strResult;
}

string CUserLogTrial::GetWindowCanvasXML(const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetWindowCanvasXML", g_pLogger);

  string strResult = "";

  // Log the window location and size that was last used during this trial
  strResult += strPrefix;
  strResult += "\t<WindowCoordinates>\n";

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Top>%d</Top>\n", m_sWindowCoordinates.top);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Bottom>%d</Bottom>\n", m_sWindowCoordinates.bottom);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Left>%d</Left>\n", m_sWindowCoordinates.left);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Right>%d</Right>\n", m_sWindowCoordinates.right);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  strResult += "\t</WindowCoordinates>\n";

  // Log the canvas location and size that was last used during this trial
  strResult += strPrefix;
  strResult += "\t<CanvasCoordinates>\n";

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Top>%d</Top>\n", m_sCanvasCoordinates.top);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Bottom>%d</Bottom>\n", m_sCanvasCoordinates.bottom);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Left>%d</Left>\n", m_sCanvasCoordinates.left);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  sprintf(m_szTempBuffer, "\t\t<Right>%d</Right>\n", m_sCanvasCoordinates.right);
  strResult += m_szTempBuffer;

  strResult += strPrefix;
  strResult += "\t</CanvasCoordinates>\n";

  return strResult;
}

string CUserLogTrial::GetParamsXML(const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetParamsXML", g_pLogger);

  string strResult = "";

  if (m_vpParams.size() > 0)
  {
    // Make parameters with the same name appear near each other in the results
    sort(m_vpParams.begin(), m_vpParams.end(), CUserLogParam::ComparePtr);    

    strResult += strPrefix;
    strResult += "\t<Params>\n";

    string strPrefixPlusTabTab = strPrefix;
    strPrefixPlusTabTab += "\t\t";

    for (unsigned int i = 0; i < m_vpParams.size(); i++)
    {
      CUserLogParam* pParam = (CUserLogParam*) m_vpParams[i];

      strResult += GetParamXML(pParam, strPrefixPlusTabTab);
    }

    strResult += strPrefix;
    strResult += "\t</Params>\n";
  }

  return strResult;

}

int CUserLogTrial::GetButtonCount() {
  int iCount(0);

  for(VECTOR_NAV_CYCLE_PTR::iterator it(m_vpNavCycles.begin()); it != m_vpNavCycles.end(); ++it)
    for(VECTOR_USER_BUTTON_PTR::iterator it2((*it)->vectorButtons.begin()); it2 != (*it)->vectorButtons.end(); ++it2)
      iCount += (*it2)->GetCount();

  return iCount;
}

double CUserLogTrial::GetTotalBits() {
  double dBits(0.0);

  for(VECTOR_NAV_CYCLE_PTR::iterator it(m_vpNavCycles.begin()); it != m_vpNavCycles.end(); ++it)
    dBits += (*it)->dBits;

  return dBits;
}

// Parameters can optionally be specified to be added to the Trial objects.
// This allows us to easily see what a certain parameter value was used
// in a given trial.  
void CUserLogTrial::AddParam(const string& strName, const string& strValue, int iOptionMask)
{
  //CFunctionLogger f1("CUserLogTrial::AddParam", g_pLogger);

  bool bTrackMultiple     = false;

  if (iOptionMask & userLogParamTrackMultiple)
    bTrackMultiple = true;

  // See if this matches an existing parameter value that we may want to 
  // overwrite.  But only if we aren't suppose to keep track of multiple changes.
  if (!bTrackMultiple)
  {
    for (unsigned int i = 0; i < m_vpParams.size(); i++)
    {
      CUserLogParam* pParam = (CUserLogParam*) m_vpParams[i];

      if (pParam != NULL)
      {
        if (pParam->strName.compare(strName) == 0)
        {
          pParam->strValue = strValue;
          return;
        }
      }
    }
  }
  // We need to add a new param
  CUserLogParam* pNewParam = new CUserLogParam;
  if (pNewParam == NULL)
  {
    g_pLogger->Log("CUserLogTrial::AddParam, newParam was NULL!", logNORMAL);
    return;
  }

  pNewParam->strName           = strName;
  pNewParam->strValue          = strValue;
  pNewParam->strTimeStamp      = "";

  // Parameters that can have multiple values logged will also log when they were changed
  if (bTrackMultiple)
    pNewParam->strTimeStamp  = CTimeSpan::GetTimeStamp();

  m_vpParams.push_back(pNewParam);
}

// Static method that generates the XML representation of a 
// single param name value set.  Used both to output params
// for a trial and for the parent UserLog object.
string CUserLogTrial::GetParamXML(CUserLogParam* pParam, const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetParamXML", g_pLogger);

  string strResult = "";

  if (pParam != NULL)
  {
    strResult += strPrefix;
    strResult += "<";
    strResult += pParam->strName;
    strResult += ">";

    if (pParam->strTimeStamp.length() > 0)
    {
      strResult += "\n";
      strResult += strPrefix;
      strResult += "\t<Value>";
      strResult += pParam->strValue;
      strResult += "</Value>\n";

      strResult += strPrefix;
      strResult += "\t<Time>";
      strResult += pParam->strTimeStamp;
      strResult += "</Time>\n";

      strResult += strPrefix;
    }
    else 
    {
      strResult += pParam->strValue;
    }

    strResult += "</";
    strResult += pParam->strName;
    strResult += ">\n";
  }

  return strResult;
}

// Returns a pointer to the currently active navigation cycle
NavCycle* CUserLogTrial::GetCurrentNavCycle()
{
  //CFunctionLogger f1("CUserLogTrial::GetCurrentNavCycle", g_pLogger);

  if (m_vpNavCycles.size() <= 0)
    return NULL;
  return m_vpNavCycles[m_vpNavCycles.size() - 1];
}

// Gets a pointer to the last NavLocation object
// in the current navication cycle.
NavLocation* CUserLogTrial::GetCurrentNavLocation()
{
  //CFunctionLogger f1("CUserLogTrial::GetCurrentNavLocation", g_pLogger);

//   NavCycle* pCycle = GetCurrentNavCycle();

//   if (pCycle == NULL)
//     return NULL;

//   if (pCycle->vectorNavLocations.size() <= 0)
//     return NULL;

//   return (NavLocation*) pCycle->vectorNavLocations[pCycle->vectorNavLocations.size() - 1];

  // New version - reverse iterate through the list and find the last nav cycle which has any locations

  for(VECTOR_NAV_CYCLE_PTR::reverse_iterator it(m_vpNavCycles.rbegin()); it != m_vpNavCycles.rend(); ++it) {
    if((*it)->vectorNavLocations.size() > 0)
      return (NavLocation*) (*it)->vectorNavLocations[(*it)->vectorNavLocations.size() - 1];
  }

  return NULL;
}

// Adds a new navgiation cycle to our collection
NavCycle* CUserLogTrial::AddNavCycle()
{
  //CFunctionLogger f1("CUserLogTrial::AddNavCycle", g_pLogger);

  NavCycle* pNewCycle = new NavCycle;
  if (pNewCycle == NULL)
  {
    g_pLogger->Log("CUserLogTrial::AddNavCycle, failed to create NavCycle!", logNORMAL);
    return NULL;
  }

  pNewCycle->pSpan = new CTimeSpan("Time", false);

  m_vpNavCycles.push_back(pNewCycle);
  return pNewCycle;
}

string CUserLogTrial::GetNavCyclesXML(const string& strPrefix)
{
  //CFunctionLogger f1("CUserLogTrial::GetNavCyclesXML", g_pLogger);

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

  for (unsigned int i = 0; i < m_vpNavCycles.size(); i++)
  {
    NavCycle* pCycle = (NavCycle*) m_vpNavCycles[i];

    if (pCycle != NULL)
    {
      strResult += strPrefixTabTab;
      strResult += "<Nav>\n";

      if (pCycle->pSpan != NULL)
        strResult += pCycle->pSpan->GetXML(strPrefixTabTabTab);

      if (pCycle->vectorNavLocations.size() > 0)
      {
        strResult += strPrefixTabTabTab;
        strResult += "<Locations>\n";

        for (unsigned int i = 0; i < pCycle->vectorNavLocations.size(); i++)
        {
          NavLocation* pLocation = (NavLocation*) pCycle->vectorNavLocations[i];

          if (pLocation != NULL)
            strResult += GetLocationXML(pLocation, strPrefixTabTabTabTab);
        }
        strResult += strPrefixTabTabTab;
        strResult += "</Locations>\n";
      }

      if (pCycle->vectorMouseLocations.size() > 0)
      {
        strResult += strPrefixTabTabTab;
        strResult += "<MousePositions>\n";

        for (unsigned int i = 0; i < pCycle->vectorMouseLocations.size(); i++)
        {
          CUserLocation* pLocation = (CUserLocation*) pCycle->vectorMouseLocations[i];

          if (pLocation != NULL)
          {
            strResult += pLocation->GetXML(strPrefixTabTabTabTab);
          }
        }

        strResult += strPrefixTabTabTab;
        strResult += "</MousePositions>\n";
      }

      if (pCycle->vectorButtons.size() > 0)
      {
        strResult += strPrefixTabTabTab;
        strResult += "<Buttons>\n";

        for (unsigned int i = 0; i < pCycle->vectorButtons.size(); i++)
        {
          CUserButton* pButton = (CUserButton*) pCycle->vectorButtons[i];

          if(pButton)
          {
            strResult += pButton->GetXML(strPrefixTabTabTabTab);
          }
        }

        strResult += strPrefixTabTabTab;
        strResult += "</Buttons>\n";
      }

      strResult += strPrefixTabTab;
      strResult += "</Nav>\n";

    }
  }

  strResult += strPrefixTab;
  strResult += "</Navs>\n";

  return strResult;
}

// Construct based on some XML, second parameter is just to make signature
// different from the normal constructor.
CUserLogTrial::CUserLogTrial(const string& strXML, int iIgnored)
{
  //CFunctionLogger f1("CUserLogTrial::CUserLogTrial(XML)", g_pLogger);

  InitMemberVars();
  VECTOR_STRING vNavs;

  string strParams        = XMLUtil::GetElementString("Params", strXML, true);           
  string strWindow        = XMLUtil::GetElementString("WindowCoordinates", strXML, true);           
  string strCanvas        = XMLUtil::GetElementString("CanvasCoordinates", strXML, true);           
  string strNavs          = XMLUtil::GetElementString("Navs", strXML, true);           
  string strSummary       = XMLUtil::GetElementString("Summary", strXML, true);
  string strSummaryTime   = XMLUtil::GetElementString("Time", strSummary, true);
  vNavs                   = XMLUtil::GetElementStrings("Nav", strNavs, true);

  string strCurrentTrial  = XMLUtil::GetElementString("CurrentTrial", strXML, false);
  if (strCurrentTrial.length() > 0)
  {
    // We copied the XML string directly into the member variable
    // including the start/end tag, so we need to reproduce the
    // tags ourselves.
    m_strCurrentTrial       =  "\t\t\t<CurrentTrial>\n";
    m_strCurrentTrial       += strCurrentTrial;
    m_strCurrentTrial       += "</CurrentTrial>\n";
  }

  m_vpParams                = ParseParamsXML(strParams);
  m_sWindowCoordinates      = ParseWindowXML(strWindow);
  m_sCanvasCoordinates      = ParseWindowXML(strCanvas);
  m_pSpan                   = new CTimeSpan("Time", strSummaryTime);

  // Process each <Nav> tag
  string strTime              = "";
  string strLocations         = "";
  string strMousePositions    = "";

  VECTOR_STRING vLocations;
  VECTOR_STRING vMousePositions;
  VECTOR_STRING vAdded;

  for (VECTOR_STRING_ITER iter = vNavs.begin(); iter < vNavs.end(); iter++)
  {
    strTime              = "";
    strLocations         = "";
    strMousePositions    = "";
    vLocations.erase(vLocations.begin(), vLocations.end());
    vMousePositions.erase(vMousePositions.begin(), vMousePositions.end());

    strTime             = XMLUtil::GetElementString("Time", *iter, true);
    strLocations        = XMLUtil::GetElementString("Locations", *iter, true);
    strMousePositions   = XMLUtil::GetElementString("MousePositions", *iter, true);

    NavCycle* pCycle = new NavCycle();
    if (pCycle == NULL)
    {
      g_pLogger->Log("CUserLogTrial::CUserLogTrial, failed to create NavCycle!", logNORMAL);
      return;
    }

    pCycle->pSpan = NULL;

    if (strTime.length() > 0)
    {
      pCycle->pSpan = new CTimeSpan("Time", strTime);
    }
    if (strLocations.length() > 0)
    {
      vLocations = XMLUtil::GetElementStrings("Location", strLocations, true);

      for (VECTOR_STRING_ITER iter2 = vLocations.begin(); iter2 < vLocations.end(); iter2++)
      {
        vAdded.erase(vAdded.begin(), vAdded.end());

        NavLocation* pLocation = new NavLocation();
        if (pLocation == NULL)
        {
          g_pLogger->Log("CUserLogTrial::CUserLogTrial, failed to create NavLocation!", logNORMAL);
          return;
        }                

        pLocation->strHistory     = XMLUtil::GetElementString("History", *iter2);
        pLocation->avgBits        = (double) XMLUtil::GetElementFloat("AvgBits", *iter2);
        pLocation->event          = (eUserLogEventType) XMLUtil::GetElementInt("Event", *iter2);
        pLocation->numDeleted     = XMLUtil::GetElementInt("NumDeleted", *iter2);

        pLocation->span           = NULL;
        string strTime            = XMLUtil::GetElementString("Time", *iter2);
        pLocation->span           = new CTimeSpan("Time", strTime);

        // Handle the multiple <Add> tags that might exist
        vAdded                    = XMLUtil::GetElementStrings("Add", *iter2);                
        pLocation->pVectorAdded   = new Dasher::VECTOR_SYMBOL_PROB;

        for (VECTOR_STRING_ITER iter3 = vAdded.begin(); iter3 < vAdded.end(); iter3++)
        {
          Dasher::SymbolProb sAdd(0, // We don't have the original integer symbol index
                                  XMLUtil::GetElementString("Text", *iter3),
                                  XMLUtil::GetElementFloat("Prob", *iter3));

          if (pLocation->pVectorAdded != NULL)
            pLocation->pVectorAdded->push_back(sAdd);

          // Also track it in one complete vector of all the adds
          m_vHistory.push_back(sAdd);        
        }

        // If this was a deleted event, then we need to erase some stuff from the running history
        // Be careful not to pop more things than we have (this will hork the
        // memory up on linux but not windows).
        int iActualNumToDelete = min((int) m_vHistory.size(), pLocation->numDeleted);
        for (int i = 0; i < iActualNumToDelete; i++)
          m_vHistory.pop_back();

        pCycle->vectorNavLocations.push_back(pLocation);
      }
    }

    if (strMousePositions.length() > 0)
    {
      vMousePositions = XMLUtil::GetElementStrings("Pos", strMousePositions, true);
      for (VECTOR_STRING_ITER iter2 = vMousePositions.begin(); iter2 < vMousePositions.end(); iter2++)
      {
        CUserLocation* pLocation = new CUserLocation(*iter2);
        pCycle->vectorMouseLocations.push_back(pLocation);
      }

    }

    m_vpNavCycles.push_back(pCycle);
  }

}

// Helper that parses parameters out of the XML block, used by UserLog 
// and by UserLogTrial to do the same thing.
VECTOR_USER_LOG_PARAM_PTR CUserLogTrial::ParseParamsXML(const string& strXML)
{
  //CFunctionLogger f1("CUserLogTrial::ParseParamsXML", g_pLogger);

  VECTOR_USER_LOG_PARAM_PTR   vResult;
  VECTOR_NAME_VALUE_PAIR      vParams;

  vParams = XMLUtil::GetNameValuePairs(strXML, true);

  // Handle adding all the name/value parameter pairs.  XML looks like:
  //      <Eyetracker>0</Eyetracker>
  //      <MaxBitRate>
  //	    	<Value>7.0100</Value>
  //			<Time>15:48:53.140</Time>
  //  	</MaxBitRate>
  for (VECTOR_NAME_VALUE_PAIR_ITER iter = vParams.begin(); iter < vParams.end(); iter++)
  {
    CUserLogParam* pParam = new CUserLogParam();

    if (pParam != NULL) 
    {
      pParam->strName      = iter->strName;

      // See if we have a type that has a timestamp
      string strValue     = XMLUtil::GetElementString("Value", iter->strValue, true);
      string strTime      = XMLUtil::GetElementString("Time", iter->strValue, true);

      if ((strValue.length() > 0)  || (strTime.length() > 0))
      {
        pParam->strValue     = strValue;
        pParam->strTimeStamp = strTime;
      }
      else
        pParam->strValue     = iter->strValue;

      pParam->options = 0;

      vResult.push_back(pParam);
    }
  }

  return vResult;
}

// Parse our window or canvas coorindates from XML
WindowSize CUserLogTrial::ParseWindowXML(const string& strXML)
{
  //CFunctionLogger f1("CUserLogTrial::ParseWindowXML", g_pLogger);

  WindowSize sResult;

  sResult.top      = XMLUtil::GetElementInt("Top", strXML);
  sResult.bottom   = XMLUtil::GetElementInt("Bottom", strXML);
  sResult.left     = XMLUtil::GetElementInt("Left", strXML);
  sResult.right    = XMLUtil::GetElementInt("Right", strXML);

  return sResult;
}

// Returns a vector that contains the tab delimited mouse
// coordinates for each of our navigation cycles.
VECTOR_STRING CUserLogTrial::GetTabMouseXY(bool bReturnNormalized)
{
  //CFunctionLogger f1("CUserLogTrial::GetTabMouseXY", g_pLogger);

  VECTOR_STRING vResult;
  for (VECTOR_NAV_CYCLE_PTR_ITER iter = m_vpNavCycles.begin(); iter < m_vpNavCycles.end(); iter++)
  {
    string strResult = "";

    if (*iter != NULL)
    {
      for (VECTOR_USER_LOCATION_PTR_ITER iter2 = (*iter)->vectorMouseLocations.begin(); 
        iter2 < (*iter)->vectorMouseLocations.end();
        iter2++)
      {
        if (*iter2 != NULL)
          strResult += (*iter2)->GetTabMouseXY(bReturnNormalized);
      }
    }

    vResult.push_back(strResult);
  }

  return vResult;
}

// Calculates the mouse density with a given grid size number of buckets.
// Each element of the vector is a 2D array of double values from 0.0 - 1.0.
//
// NOTE: We allocate the memory here for the double**, our caller must 
// handle freeing it!
VECTOR_DENSITY_GRIDS CUserLogTrial::GetMouseDensity(int iGridSize)
{
  //CFunctionLogger f1("CUserLogTrial::GetMouseDensity", g_pLogger);

  VECTOR_DENSITY_GRIDS vResult;

  for (VECTOR_NAV_CYCLE_PTR_ITER iter = m_vpNavCycles.begin(); iter < m_vpNavCycles.end(); iter++)
  {
    if (*iter != NULL)
    {
      DENSITY_GRID ppGrid;

      // Init the grid with all 0.0 values
      ppGrid = new double*[iGridSize];
      for (int i = 0; i < iGridSize; i++)
        ppGrid[i] = new double[iGridSize];

      for (int i = 0; i < iGridSize; i++)
        for (int j = 0; j < iGridSize; j++)
          ppGrid[i][j] = 0.0;

      unsigned int iCount = 0;

      // Assign each mouse to location to one of the buckets and increment 
      // the count on that bucket.
      for (VECTOR_USER_LOCATION_PTR_ITER iter2 = (*iter)->vectorMouseLocations.begin(); 
        iter2 < (*iter)->vectorMouseLocations.end();
        iter2++)
      {
        if (*iter2 != NULL)
        {
          int i = 0;
          int j = 0;
          (*iter2)->GetMouseGridLocation(iGridSize, &i, &j);
          // Increment the count on this location, we'll throw away points
          // that were outside the canvas grid.
          if ((i < iGridSize) && (j < iGridSize) && (i >= 0) && (j >= 0))
          {
            // We reverse j and i to get x to increase left top right
            // and y from top to bottom
            ppGrid[j][i] = ppGrid[j][i] + 1.0;
            iCount++;
          }
        }
      }
      // Now normalize everything so each grid location is a 
      // percentage of the time we spent in that location.
      for (int i = 0; i < iGridSize; i++)
      {
        {
          for (int j = 0; j < iGridSize; j++)
          {
            ppGrid[i][j] = ppGrid[i][j] / (double) iCount;
          }
        }
      }

      vResult.push_back(ppGrid);
    }
  }

  return vResult;
}

// Merge the density of two grids together.  This is done but adding their values
// and dividing by two.  If either pointer is NULL, then we return the other grid
// values intact.  We free the memory in our parameter grids.
DENSITY_GRID CUserLogTrial::MergeGrids(int iGridSize, DENSITY_GRID ppGridA, DENSITY_GRID ppGridB)
{
  //CFunctionLogger f1("CUserLogTrial::MergeGrids", g_pLogger);

  DENSITY_GRID ppResult;
  ppResult = new double*[iGridSize];
  for (int i = 0; i < iGridSize; i++)
    ppResult[i] = new double[iGridSize];

  for (int i = 0; i < iGridSize; i++)
    for (int j = 0; j < iGridSize; j++)
      ppResult[i][j] = 0.0;

  // Both NULL, then return grid of all 0.0's
  if ((ppGridA == NULL) && (ppGridB == NULL))
    return ppResult;

  if (ppGridA == NULL)
  {
    // grid A is NULL, return copy of grid B
    for (int i = 0; i < iGridSize; i++)
    {
      for (int j = 0; j < iGridSize; j++)
        ppResult[i][j] = ppGridB[i][j];
    }    
  }    
  else if (ppGridB == NULL)
  {
    // grid B is NULL, return copy of grid A
    for (int i = 0; i < iGridSize; i++)
    {
      for (int j = 0; j < iGridSize; j++)
        ppResult[i][j] = ppGridA[i][j];
    }

  }
  else
  {
    // Normal case, merge the two density grids
    for (int i = 0; i < iGridSize; i++)
    {
      for (int j = 0; j < iGridSize; j++)
        ppResult[i][j] = (ppGridA[i][j] + ppGridB[i][j]) / 2.0;
    }
  }

  if (ppGridA != NULL)
  {
    for (int i = 0; i < iGridSize; i++)
    {
      if (ppGridA[i] != NULL)
      {
        delete ppGridA[i];
        ppGridA[i] = NULL;
      }
    }
    delete ppGridA;
    ppGridA = NULL;

  }
  if (ppGridB != NULL)
  {
    for (int i = 0; i < iGridSize; i++)
    {
      if (ppGridB[i] != NULL)
      {
        delete ppGridB[i];
        ppGridB[i] = NULL;
      }
    }
    delete ppGridB;
    ppGridB = NULL;
  }

  return ppResult;
}


