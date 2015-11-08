#include "../Common/Common.h"

#include "UserLog.h"
#include <fstream>
#include <cstring>

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

using namespace Dasher;
using namespace Dasher::Settings;

static UserLogParamMask s_UserLogParamMaskTable [] = {
  {SP_ALPHABET_ID,          userLogParamOutputToSimple},
  {SP_COLOUR_ID,            userLogParamOutputToSimple},
  {LP_MAX_BITRATE,          userLogParamOutputToSimple | 
                            userLogParamTrackMultiple | 
                            userLogParamTrackInTrial |
                            userLogParamForceInTrial |
                            userLogParamShortInCycle},
  {BP_CONTROL_MODE,         userLogParamOutputToSimple},
  {LP_UNIFORM,              userLogParamOutputToSimple},
  {LP_YSCALE,               userLogParamOutputToSimple},
  {LP_LANGUAGE_MODEL_ID,    userLogParamOutputToSimple},
  {LP_LM_MAX_ORDER,         userLogParamOutputToSimple},
  {LP_LM_EXCLUSION,         userLogParamOutputToSimple},
  {LP_LM_UPDATE_EXCLUSION,  userLogParamOutputToSimple},
  {LP_LM_ALPHA,             userLogParamOutputToSimple},
  {LP_LM_BETA,              userLogParamOutputToSimple},
  {LP_LM_MIXTURE,           userLogParamOutputToSimple},
  {LP_LM_WORD_ALPHA,        userLogParamOutputToSimple},
  {-1, -1}  // Flag value that should always be at the end
};

CUserLog::CUserLog(CSettingsUser *pCreateFrom,
                   Observable<const CEditEvent *> *pObsv, int iLogTypeMask)
: CUserLogBase(pObsv), CSettingsUserObserver(pCreateFrom) {
  //CFunctionLogger f1("CUserLog::CUserLog", g_pLogger);

  InitMemberVars();

  m_iLevelMask    = iLogTypeMask;

  InitUsingMask(iLogTypeMask);

  if ((m_bSimple) && (m_pSimpleLogger != NULL))
    m_pSimpleLogger->Log("start, %s", logDEBUG, GetVersionInfo().c_str());

  SetOuputFilename();
  m_pApplicationSpan = new CTimeSpan("Application", true);

  if (m_pApplicationSpan == NULL)
    g_pLogger->Log("CUserLog::CUserLog, failed to create m_pApplicationSpan!", logNORMAL);

  // TODO: for the load test harness, we apparently need to create the object directly
  // without a settings store (which will break CSettingsObserver, etc.); and then,
  // don't call the following:
  AddInitialParam();
}

CUserLog::~CUserLog()
{
  //CFunctionLogger f1("CUserLog::~CUserLog", g_pLogger);

  if ((m_bSimple) && (m_pSimpleLogger != NULL))
    m_pSimpleLogger->Log("stop", logDEBUG);

  if (m_pApplicationSpan != NULL)
  {
    delete m_pApplicationSpan;
    m_pApplicationSpan = NULL;
  }

  for (unsigned int i = 0; i < m_vpTrials.size(); i++)
  {
    CUserLogTrial* pTrial = (CUserLogTrial*) m_vpTrials[i];

    if (pTrial != NULL)
    {
      delete pTrial;
      pTrial = NULL;
    }
  }

  for (unsigned int i = 0; i < m_vParams.size(); i++)
  {
    CUserLogParam* pParam = (CUserLogParam*) m_vParams[i];

    if (pParam != NULL)
    {
      delete pParam;
      pParam = NULL;
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

// Do initialization of member variables based on the user log level mask
void CUserLog::InitUsingMask(int iLogLevelMask)
{
  //CFunctionLogger f1("CUserLog::InitUsingMask", g_pLogger);

  m_bInitIsDone = false;

  if (iLogLevelMask & userLogSimple)
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

    if (m_pSimpleLogger == NULL)
      m_pSimpleLogger = new CFileLogger(USER_LOG_SIMPLE_FILENAME, logDEBUG, logTimeStamp | logDateStamp);
  }

  if (iLogLevelMask & userLogDetailed)
    m_bDetailed = true;        

}

// Called when we want to output the log file (usually on exit of dasher)
void CUserLog::OutputFile()
{
  //CFunctionLogger f1("CUserLog::OutputFile", g_pLogger);

  if (m_bDetailed)
  {
    // Let the last pTrial object know we are done with it, this lets it do
    // any final calculations.
    if (m_vpTrials.size() > 0)
    {
      CUserLogTrial* pTrial = m_vpTrials[m_vpTrials.size() - 1];

      if (pTrial != NULL)
        pTrial->Done();
    }

    // Output our data to an XML file before we destruct
    WriteXML();
  }
}

void CUserLog::StartWriting()
{
  //CFunctionLogger f1("CUserLog::StartWriting", g_pLogger);

  if (m_bSimple)
  {
    // The canvas size changes multiple times as a user resizes it.  We just want to write
    // one short log entry for the final position the next time they start writing.
    if ((m_bNeedToWriteCanvas) && (m_pSimpleLogger != NULL))
    {
      m_pSimpleLogger->Log("canvas:\t%d\t%d\t%d\t%d", 
        logDEBUG, 
        m_sCanvasCoordinates.top, 
        m_sCanvasCoordinates.left, 
        m_sCanvasCoordinates.bottom, 
        m_sCanvasCoordinates.right);
      m_bNeedToWriteCanvas = false;
    }

    // We log what happened between StartWriting() and StopWriting()
    // so clear out any previous history.
    ResetCycle();
  }

  if (m_bDetailed)
  {
    CUserLogTrial* pTrial = GetCurrentTrial();

    // This could be the first use in this pTrial, create a new one if needed
    if (pTrial == NULL)
      pTrial = AddTrial();

    if (pTrial != NULL)
      pTrial->StartWriting();
    else
      g_pLogger->Log("CUserLog::StartWriting, failed to create new pTrial!", logNORMAL);
  }

  m_bIsWriting = true;
}

// This version should be called at the end of navigation with the dNats
// value under the current mouse position.  This would be more accurate
// then the last value from a mouse event since some time may have 
// elapsed.
void CUserLog::StopWriting(float dNats)
{
  //CFunctionLogger f1("CUserLog::StopWriting", g_pLogger);

  if (m_bIsWriting)
  {
    m_dCycleNats = (double) dNats;
    StopWriting();
  }
}

void CUserLog::StopWriting()
{
  //CFunctionLogger f1("CUserLog::StopWriting", g_pLogger);

  if (m_bIsWriting)
  {
    m_bIsWriting = false;

    // In simple logging mode, we'll output the stats for this navigation cycle
    if ((m_bSimple) && (m_pSimpleLogger != NULL))
      m_pSimpleLogger->Log("%s", logDEBUG, GetStartStopCycleStats().c_str());

    if (m_bDetailed)
    {
      CUserLogTrial* pTrial = GetCurrentTrial();

      if (pTrial == NULL)
      {
        g_pLogger->Log("CUserLog::StopWriting, pTrial was NULL!", logNORMAL);
        return;
      }

      pTrial->StopWriting(GetCycleBits());
    }
  }
}

void CUserLog::AddSymbols(Dasher::VECTOR_SYMBOL_PROB* vpNewSymbols, eUserLogEventType iEvent)
{
  //CFunctionLogger f1("CUserLog::AddSymbols", g_pLogger);

  if (!m_bIsWriting)
  {
    // StartWriting() wasn't called, so we'll do it implicitly now
    g_pLogger->Log("CUserLog::AddSymbols, StartWriting() not called?", logDEBUG);
    StartWriting();
  }

  if (vpNewSymbols == NULL)
  {
    g_pLogger->Log("CUserLog::AddSymbols, vpNewSymbols was NULL!", logNORMAL);
    return;
  }

  if (m_bSimple)
  {
    // Also store a copy in a vector that gets cleared 
    // time StartWriting() is called.
    m_vCycleHistory.insert(m_vCycleHistory.end(), vpNewSymbols->begin(), vpNewSymbols->end());
  }

  if (m_bDetailed)
  {
    CUserLogTrial* pTrial = GetCurrentTrial();

    // We should have a pTrial object since StartWriting() should have been called before us
    if (pTrial == NULL)
    {
      g_pLogger->Log("CUserLog::AddSymbols, pTrial was NULL!", logNORMAL);
      return;
    }

    pTrial->AddSymbols(vpNewSymbols, iEvent);
  }
}

void CUserLog::DeleteSymbols(int iNumToDelete, eUserLogEventType iEvent)
{
  //CFunctionLogger f1("CUserLog::DeleteSymbols", g_pLogger);

  if (iNumToDelete <= 0)
    return;

  if (!m_bIsWriting)
  {
    // StartWriting() wasn't called, so we'll do it implicitly now
    g_pLogger->Log("CUserLog::DeleteSymbols, StartWriting() not called?", logDEBUG);
    StartWriting();
  }

  if (m_bSimple)
  {
    m_iCycleNumDeletes += iNumToDelete;

    // Be careful not to pop more things than we have (this will hork the
    // memory up on linux but not windows).
    int iActualNumToDelete = min((int) m_vCycleHistory.size(), iNumToDelete);
    for (int i = 0; i < iActualNumToDelete; i++)
      m_vCycleHistory.pop_back();
  }   

  if (m_bDetailed)
  {
    CUserLogTrial* pTrial = GetCurrentTrial();

    // We should have a pTrial object since StartWriting() should have been called before us
    if (pTrial == NULL)
    {
      g_pLogger->Log("CUserLog::DeleteSymbols, pTrial was NULL!", logNORMAL);
      return;
    }

    pTrial->DeleteSymbols(iNumToDelete, iEvent);
  }
}

void CUserLog::NewTrial()
{
  //CFunctionLogger f1("CUserLog::NewTrial", g_pLogger);

  if (m_bIsWriting)
  {
    // We should have called StopWriting(), but we'll do it here implicitly
    g_pLogger->Log("CUserLog::NewTrial, StopWriting() not called?", logDEBUG);        
    StopWriting();
  }

  // For safety we can dump the XML to file after each pTrial is done.  This
  // might be a good idea for long user pTrial sessions just in case Dasher
  // were to do something completely crazy like crash.
  if (USER_LOG_DUMP_AFTER_TRIAL)
    WriteXML();

  if (m_bDetailed)
  {
    CUserLogTrial* pTrial = GetCurrentTrial();

    if (pTrial == NULL)
    {
      // Not an error, they may just hit new doc before anything else at start
      return;
    }

    if (pTrial->HasWritingOccured())
    {
      // Create a new pTrial if the existing one has already been used
      pTrial = AddTrial();
    }
  }
}

// Overloaded version that converts a double to a string
void CUserLog::AddParam(const string& strName, double dValue, int iOptionMask)
{
  sprintf(m_szTempBuffer, "%0.4f", dValue);
  AddParam(strName, m_szTempBuffer, iOptionMask);
}

// Overloaded version that converts a int to a string
void CUserLog::AddParam(const string& strName, int iValue, int iOptionMask)
{
  sprintf(m_szTempBuffer, "%d", iValue);
  AddParam(strName, m_szTempBuffer, iOptionMask);
}

// Adds a general parameter to our XML.  This lets various Dasher components 
// record what they are set at.  Optionally can be set to track multiple
// values for the same parameter or to always output a line to the simple
// log file when the parameter is set.
void CUserLog::AddParam(const string& strName, const string& strValue, int iOptionMask)
{
  //CFunctionLogger f1("CUserLog::AddParam", g_pLogger);

  bool bOutputToSimple    = false;
  bool bTrackMultiple     = false;
  bool bTrackInTrial      = false;
  bool bForceInTrial      = false;
  bool bShortInCycle      = false;

  if (iOptionMask & userLogParamOutputToSimple)
    bOutputToSimple = true;
  if (iOptionMask & userLogParamTrackMultiple)
    bTrackMultiple = true;
  if (iOptionMask & userLogParamTrackInTrial)
    bTrackInTrial = true;
  if (iOptionMask & userLogParamForceInTrial)
    bForceInTrial = true;
  if (iOptionMask & userLogParamShortInCycle)
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

  // See if this matches an existing parameter value that we may want to 
  // overwrite.  But only if we aren't suppose to keep track of multiple changes.
  if (!bTrackMultiple)
  {
    for (unsigned int i = 0; i < m_vParams.size(); i++)
    {
      CUserLogParam* pParam = (CUserLogParam*) m_vParams[i];

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
    g_pLogger->Log("CUserLog::AddParam, failed to create CUserLogParam object!", logNORMAL);
    return;
  }

  pNewParam->strName           = strName;
  pNewParam->strValue          = strValue;
  pNewParam->strTimeStamp      = "";
  pNewParam->options           = iOptionMask;

  // Parameters that can have multiple values logged will also log when they were changed
  if (bTrackMultiple)
    pNewParam->strTimeStamp = CTimeSpan::GetTimeStamp();

  m_vParams.push_back(pNewParam);

  if ((bTrackInTrial) && (m_bDetailed))
  {
    // See if we need to pass the parameter onto the current pTrial object
    CUserLogTrial* pTrial = GetCurrentTrial();

    if (pTrial != NULL)
      pTrial->AddParam(strName, strValue, iOptionMask);
  }
}


// Adds a new point in our tracking of mouse locations
void CUserLog::AddMouseLocation(int iX, int iY, float dNats)
{
  //CFunctionLogger f1("CUserLog::AddMouseLocation", g_pLogger);

  // Check to see if it is time to actually push a mouse location update
  if (UpdateMouseLocation())
  {
    if (m_bDetailed)
    {
      CUserLogTrial* pTrial = GetCurrentTrial();

      if (pTrial == NULL)
      {
        // Only track during an actual pTrial
        return;
      }

      // Only record mouse locations during navigation
      if (pTrial->IsWriting())
        pTrial->AddMouseLocation(iX, iY, dNats);
    }

    // Keep track of the dNats for the current mouse position
    if (m_bIsWriting)
      m_dCycleNats = dNats;
  }
}

// Adds the size of the current window
void CUserLog::AddWindowSize(int iTop, int iLeft, int iBottom, int iRight)
{
  //CFunctionLogger f1("CUserLog::AddWindowSize", g_pLogger);

  m_sWindowCoordinates.top     = iTop;
  m_sWindowCoordinates.left    = iLeft;
  m_sWindowCoordinates.bottom  = iBottom;
  m_sWindowCoordinates.right   = iRight;

  if (m_bDetailed)
  {
    CUserLogTrial* pTrial = GetCurrentTrial();

    if (pTrial == NULL)
    {
      // Only track during an actual pTrial
      return;
    }

    pTrial->AddWindowSize(iTop, iLeft, iBottom, iRight);
  }
}

// Adds the size of the current canvas, this should be called when our
// window is initially created and whenever somebody mucks with the 
// size.
void CUserLog::AddCanvasSize(int iTop, int iLeft, int iBottom, int iRight)
{
  //CFunctionLogger f1("CUserLog::AddCanvasSize", g_pLogger);

  // Only log to simple log object if the coordinates are different from 
  // what we had prior to now.
  if ((m_bSimple) && 
    (m_pSimpleLogger != NULL) &&
    ((m_sCanvasCoordinates.top    != iTop) ||
    (m_sCanvasCoordinates.left   != iLeft) ||
    (m_sCanvasCoordinates.bottom != iBottom) ||
    (m_sCanvasCoordinates.right  != iRight)))
    m_bNeedToWriteCanvas = true;

  m_sCanvasCoordinates.top     = iTop;
  m_sCanvasCoordinates.left    = iLeft;
  m_sCanvasCoordinates.bottom  = iBottom;
  m_sCanvasCoordinates.right   = iRight;

  if (m_bDetailed)
  {
    CUserLogTrial* pTrial = GetCurrentTrial();

    if (pTrial == NULL)
    {
      // Only track during an actual pTrial
      return;
    }

    pTrial->AddCanvasSize(iTop, iLeft, iBottom, iRight);
  }
}

// We may want to use a noramlized version of mouse coordinates, this way it is
// invariant to changes in the window size before, during, or after navigation.
// The caller must send us both the x, y coordinates and the current window size.
void CUserLog::AddMouseLocationNormalized(int iX, int iY, bool bStoreIntegerRep, float dNats)
{
  //CFunctionLogger f1("CUserLog::AddMouseLocationNormalized", g_pLogger);

  // Check to see if it is time to actually push a mouse location update
  if (UpdateMouseLocation())
  {
    if ((m_sCanvasCoordinates.bottom == 0) &&
      (m_sCanvasCoordinates.left == 0) &&
      (m_sCanvasCoordinates.right == 0) &&
      (m_sCanvasCoordinates.top == 0))
    {
      g_pLogger->Log("CUserLog::AddMouseLocationNormalized, called before AddCanvasSize()?", logNORMAL);
      return;
    }

    ComputeSimpleMousePos(iX, iY);

    if (m_bDetailed)
    {
      CUserLogTrial* pTrial = GetCurrentTrial();

      if (pTrial == NULL)
      {
        // Only track during an actual pTrial
        return;
      }

      // Only record mouse locations during navigation
      if (pTrial->IsWriting())
        pTrial->AddMouseLocationNormalized(iX, iY, bStoreIntegerRep, dNats);
    }

    // Keep track of the dNats for the current mouse position
    if (m_bIsWriting)
      m_dCycleNats = dNats;
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
  //CFunctionLogger f1("CUserLog::InitIsDone", g_pLogger);

  m_bInitIsDone = true;
}

// Sets our output filename based on the current date and time.
// Or if a parameter is passed in, use that as the output name.
void CUserLog::SetOuputFilename(const string& strFilename)
{
  //CFunctionLogger f1("CUserLog::SetOuputFilename", g_pLogger);

  if (strFilename.length() > 0)
  {
    m_strFilename = strFilename;
  }
  else
  {
    m_strFilename = USER_LOG_DETAILED_PREFIX;
    char* szTimeLine = NULL;
    time_t t;

    t = time(NULL);
    szTimeLine = ctime(&t);

    if ((szTimeLine != NULL) && (strlen(szTimeLine) > 18))
    {
      for (int i = 4; i < 19; i++)
      {
        if (szTimeLine[i] == ' ')
          m_strFilename += "_";
        else if (szTimeLine[i] != ':')
          m_strFilename += szTimeLine[i];
      }
    }

    m_strFilename += ".xml";
  }

  // Make sure we store a fully qualified form, to prevent movent
  // if the working directory changes
  m_strFilename = CFileLogger::GetFullFilenamePath(m_strFilename);
}

// Find out what level mask this object was created with
int CUserLog::GetLogLevelMask()
{
  //CFunctionLogger f1("CUserLog::GetLogLevelMask", g_pLogger);

  return m_iLevelMask;
}

void CUserLog::KeyDown(int iId, int iType, int iEffect) {
  CUserLogTrial* pTrial = GetCurrentTrial();
  
  if(pTrial)
    pTrial->AddKeyDown(iId, iType, iEffect);
}
  
// This gets called whenever parameters get changed that we are tracking
void CUserLog::HandleEvent(int iParameter)
{  
  int i = 0;

  // Go through each of the parameters in our lookup table from UserLogParam.h.
  // If the key matches the notification event, then we want to push the
  // parameter change to the logging object.
  while (s_UserLogParamMaskTable[i].key != -1)
  {
    if (s_UserLogParamMaskTable[i].key == iParameter)
    {
      int iOptionMask = s_UserLogParamMaskTable[i].mask;

      UpdateParam(iParameter, iOptionMask);
      return;
    }

    i++;

  } // end while (s_UserLogParamMaskTable[i].key != -1)
}

////////////////////////////////////////// private methods ////////////////////////////////////////////////

// Just inits all our member variables, called by the constructors
void CUserLog::InitMemberVars()
{
  //CFunctionLogger f1("CUserLog::InitMemberVars", g_pLogger);

  m_strFilename           = "";
  m_pApplicationSpan      = NULL;
  m_dLastMouseUpdate       = 0.0;
  m_bSimple               = false;
  m_bDetailed             = false;
  m_pSimpleLogger         = NULL;    
  m_bIsWriting            = false;
  m_bInitIsDone           = false;
  m_bNeedToWriteCanvas    = false;

  m_pCycleTimer           = NULL;
  m_iCycleNumDeletes      = 0;
  m_iCycleMouseCount      = 0;
  m_dCycleMouseNormXSum   = 0.0;
  m_dCycleMouseNormYSum   = 0.0;
  m_dCycleNats            = 0.0;

  m_sCanvasCoordinates.bottom  = 0;
  m_sCanvasCoordinates.top     = 0;
  m_sCanvasCoordinates.right   = 0;
  m_sCanvasCoordinates.left    = 0;

  m_sWindowCoordinates.bottom  = 0;
  m_sWindowCoordinates.top     = 0;
  m_sWindowCoordinates.right   = 0;
  m_sWindowCoordinates.left    = 0;

  // We want to use a fully qualified path so that we always
  // look in the same spot, regardless of if the working
  // directory has moved during runtime.
  m_strCurrentTrialFilename   = CFileLogger::GetFullFilenamePath(USER_LOG_CURRENT_TRIAL_FILENAME);

}

// Write this objects XML out  
bool CUserLog::WriteXML()
{
	fstream fout(m_strFilename.c_str(), ios::trunc | ios::out);
	fout << GetXML();
	fout.close();

	return true;
}

// Serializes our data to XML
string CUserLog::GetXML()
{
  //CFunctionLogger f1("CUserLog::GetXML", g_pLogger);

  string strResult = "";
  strResult.reserve(USER_LOG_DEFAULT_SIZE_TRIAL_XML * (m_vpTrials.size() + 1));

  strResult += "<?xml version=\"1.0\"?>\n";

  strResult += "<UserLog>\n";
  if (m_pApplicationSpan != NULL)
    strResult += m_pApplicationSpan->GetXML("\t");

  strResult += "\t<Params>\n";
  strResult += GetParamsXML();
  strResult += "\t</Params>\n";

  strResult += "\t<Trials>\n";
  for (unsigned int i = 0; i < m_vpTrials.size(); i++)
  {
    CUserLogTrial* pTrial = (CUserLogTrial*) m_vpTrials[i];

   // Only log trials that actually had some writing in it
    if ((pTrial != NULL) && (pTrial->HasWritingOccured()))
    {
      strResult += pTrial->GetXML("\t\t");
    }
  }
  strResult += "\t</Trials>\n";

  strResult += "</UserLog>\n";

  return strResult;
}

// Returns pointer to the current user pTrial, NULL if we don't have one yet
CUserLogTrial* CUserLog::GetCurrentTrial()
{
  //CFunctionLogger f1("CUserLog::GetCurrentTrial", g_pLogger);

  if (m_vpTrials.size() <= 0)
    return NULL;
  return m_vpTrials[m_vpTrials.size() - 1];
}

// Creates a new pTrial, adds to our vector and returns the pointer
CUserLogTrial* CUserLog::AddTrial()
{
  //CFunctionLogger f1("CUserLog::AddTrial", g_pLogger);

  // Let the last pTrial object know we are done with it
  if (m_vpTrials.size() > 0)
  {
    CUserLogTrial* pTrial = m_vpTrials[m_vpTrials.size() - 1];

    if (pTrial != NULL)
      pTrial->Done();
  }

  CUserLogTrial* pTrial = new CUserLogTrial(m_strCurrentTrialFilename);
  if (pTrial != NULL)
  {
    m_vpTrials.push_back(pTrial);
    PrepareNewTrial();
  }
  else
    g_pLogger->Log("CUserLog::AddTrial, failed to create CUserLogTrialSpeech!", logNORMAL);

  return pTrial;
}

// See if the specified number of milliseconds has elapsed since the last mouse location update
bool CUserLog::UpdateMouseLocation()
{
  //CFunctionLogger f1("CUserLog::UpdateMouseLocation", g_pLogger);

#ifdef _WIN32
  struct timeb sTimeBuffer;
#else
  struct timeval sTimeBuffer;
  struct timezone sTimezoneBuffer;
#endif

#ifdef _WIN32
  ftime(&sTimeBuffer);
  double dTime = (sTimeBuffer.time * 1000.0) + sTimeBuffer.millitm;
#else
  gettimeofday(&sTimeBuffer, &sTimezoneBuffer);
  double dTime = (sTimeBuffer.tv_sec * 1000.0) + (int)(sTimeBuffer.tv_usec / 1000);
#endif

  
  if ((dTime - m_dLastMouseUpdate) > LOG_MOUSE_EVERY_MS)
  {
    m_dLastMouseUpdate = dTime;
    return true;
  }
  return false;
}

// Calculate how many bits entered in the last Start/Stop cycle
double CUserLog::GetCycleBits()
{
  //CFunctionLogger f1("CUserLog::GetCycleBits", g_pLogger);

  return m_dCycleNats / log(2.0);
}

// For lightweight logging, we want a string that represents the critical
// stats for what happened between start and stop
string CUserLog::GetStartStopCycleStats()
{
  //CFunctionLogger f1("CUserLog::GetStartStopCycleStats", g_pLogger);

  string strResult = "";

  double dNormX = 0.0;
  double dNormY = 0.0;
  if (m_iCycleMouseCount > 0)
  {        
    dNormX = m_dCycleMouseNormXSum / (double) m_iCycleMouseCount,
      dNormY = m_dCycleMouseNormYSum / (double) m_iCycleMouseCount;
  }

  if (m_pCycleTimer == NULL)
  {
    g_pLogger->Log("CUserLog::GetStartStopCycleStats, cycle timer was NULL!", logNORMAL);
    return "";
  }

  // Tab delimited fields are:
  //  elapsed time, symbols written, bits written, symbols deleted, 
  //  avg normalized x mouse coordinate, avg normalized y mouse
  //  coordinate, (any parameters marked to be put in cycle stats)
  //
  // tsbdxym stands for: time symbols bits deletes x y maxbitrate
  sprintf(m_szTempBuffer, 
          "tsbdxym:\t%0.3f\t%zu\t%0.6f\t%d\t%0.3f\t%0.3f%s",
          m_pCycleTimer->GetElapsed(), 
          m_vCycleHistory.size(), 
          GetCycleBits(), 
          m_iCycleNumDeletes, 
          dNormX,
          dNormY, 
          GetCycleParamStats().c_str());
          strResult = m_szTempBuffer;

  return strResult;
}

// Helper that computes update of the simple logging's mouse 
// position tracking member variables.
void CUserLog::ComputeSimpleMousePos(int iX, int iY)
{
  //CFunctionLogger f1("CUserLog::ComputeSimpleMousePos", g_pLogger);

  if ((m_bSimple) && (m_bIsWriting))
  {
    // We keep a running sum of normalized X, Y coordinates
    // for use in the simple log file.
    m_dCycleMouseNormXSum += CUserLocation::ComputeNormalizedX(iX, 
      m_sCanvasCoordinates.left, 
      m_sCanvasCoordinates.right);

    m_dCycleMouseNormYSum += CUserLocation::ComputeNormalizedY(iY, 
      m_sCanvasCoordinates.top, 
      m_sCanvasCoordinates.bottom);
    m_iCycleMouseCount++;
  }
}

// Resets member variables that track a cycle for simple logging
void CUserLog::ResetCycle()
{
  //CFunctionLogger f1("CUserLog::ResetCycle", g_pLogger);

  m_vCycleHistory.clear();
  m_iCycleNumDeletes       = 0;
  m_iCycleMouseCount       = 0;
  m_dCycleMouseNormXSum    = 0.0;
  m_dCycleMouseNormYSum    = 0.0;

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
  //CFunctionLogger f1("CUserLog::GetParamsXML", g_pLogger);

  string strResult = "";

  // Make parameters with the same name appear near each other in the results
  sort(m_vParams.begin(), m_vParams.end(), CUserLogParam::ComparePtr);

  for (unsigned int i = 0; i < m_vParams.size(); i++)
  {
    CUserLogParam* pParam = (CUserLogParam*) m_vParams[i];

    strResult += CUserLogTrial::GetParamXML(pParam, "\t\t");
  }

  return strResult;
}

// Prepares a new pTrial for use.  Passes on the current canvas and window
// size so normalized mouse coordinates can be calculated.  Also 
// parameters can be marked to force them into the Trial object.  Looks for 
// these and push into the current Trial object.
void CUserLog::PrepareNewTrial()
{
  //CFunctionLogger f1("CUserLog::PrepareNewTrial", g_pLogger);

  CUserLogTrial* pTrial = GetCurrentTrial();

  if (pTrial != NULL)
  {
    // We want to force the current value of any parameters that we marked
    // with the userLogParamForceInTrial option when created.  We can
    // do this by going backwards through the parameter vector and only
    // pushing through the first value of a given parameter name.
    VECTOR_STRING vFound;
    if (m_vParams.size() > 0)
    {
	  for (VECTOR_USER_LOG_PARAM_PTR_REV_ITER iter = m_vParams.rbegin(); iter != m_vParams.rend(); ++iter)
	  {
        if (((*iter) != NULL) && ((*iter)->options & userLogParamForceInTrial))
        {
          // Make sure we haven't output this one already
          VECTOR_STRING_ITER strIter;
          strIter = find(vFound.begin(), vFound.end(), (*iter)->strName);
          if (strIter == vFound.end())
          {
            pTrial->AddParam((*iter)->strName, (*iter)->strValue, (*iter)->options);
            vFound.push_back((*iter)->strName);
          }
        }
      }
    }

    // Make sure the pTrial has the current canvas and window coordinate sizes
    pTrial->AddCanvasSize(m_sCanvasCoordinates.top, 
      m_sCanvasCoordinates.left, 
      m_sCanvasCoordinates.bottom, 
      m_sCanvasCoordinates.right);

    pTrial->AddWindowSize(m_sWindowCoordinates.top, 
      m_sWindowCoordinates.left, 
      m_sWindowCoordinates.bottom, 
      m_sWindowCoordinates.right);

  }
  else
    g_pLogger->Log("CUserLog::PrepareNewTrial, failed to create CUserLogTrial", logNORMAL);
}

// Parameters can be marked to always end them at the cycle stats in short logging.
// We'll look through our parameters and return a tab delimited list of their
// values.
string CUserLog::GetCycleParamStats()
{
  //CFunctionLogger f1("CUserLog::GetCycleParamStats", g_pLogger);

  string strResult = "";
  VECTOR_STRING vFound;

  if (m_vParams.size() <= 0)
    return strResult;

  // We may have more than one parameter that needs to be added and we want
  // the stats line to be invariant to the order in which AddParam() was 
  // called.  So we'll sort by param name and then by time stamp (for 
  // parameters with multiple values).
  sort(m_vParams.begin(), m_vParams.end(), CUserLogParam::ComparePtr);

  // Find the last instance of any parameter marked as needed to be on
  // the cycle stats line.
  for (VECTOR_USER_LOG_PARAM_PTR_REV_ITER iter = m_vParams.rbegin(); iter != m_vParams.rend(); ++iter)
  {
    if (((*iter) != NULL) && ((*iter)->options & userLogParamShortInCycle))
    {
      // Make sure we haven't output this one already
      VECTOR_STRING_ITER strIter;
      strIter = find(vFound.begin(), vFound.end(), (*iter)->strName);
      if (strIter == vFound.end())
      {
        strResult += "\t";
        strResult += (*iter)->strValue;
        vFound.push_back((*iter)->strName);
      }
    }
  }

  return strResult;
}

// Return a string with the operating system and product version
string CUserLog::GetVersionInfo()
{
  //CFunctionLogger f1("CUserLog::GetVersionInfo", g_pLogger);

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

// Forces all the parameters we are tracking to be intially set, used when the
// object is first starting up.
void CUserLog::AddInitialParam()
{
  int i = 0;
  while (s_UserLogParamMaskTable[i].key != -1)
  {
    UpdateParam(s_UserLogParamMaskTable[i].key, s_UserLogParamMaskTable[i].mask);
    i++;
  }
}

// Helper method that takes a parameter ID a la Parameters.h and
// looks up its type, name and value and pushes into our object
// using the specified options mask.
void CUserLog::UpdateParam(int iParameter, int iOptionMask)
{
  string  strParamName  = GetParameterName(iParameter);

  // What method we call depends on the type of the parameter
  switch (GetParameterType(iParameter))
  {
  case (ParamBool):
    {
      // Convert bool to a integer
      int iValue = 0;
      if (GetBoolParameter(iParameter))
        iValue = 1;
      AddParam(strParamName, iValue, iOptionMask);
      return;
      break;
    }
  case (ParamLong):
    {
      AddParam(strParamName, (int) GetLongParameter(iParameter), iOptionMask);
      return;
      break;
    }
  case (ParamString):
    {
      AddParam(strParamName, GetStringParameter(iParameter), iOptionMask);
      return;
      break;
    }       
  default:
    {
      g_pLogger->Log("CUserLog::UpdateParam, matched parameter %d but unknown type %d", logNORMAL, iParameter, GetParameterType(iParameter));
      break;
    }
  };
}


///////////////////////////////////////////////////////////////////////////////////////
// Below here are methods that are just used in the standalone tool that reads in 
// UserLog XML files and does cool things to them.  
// TODO these are broken by settings rewrite. Fix???

// Load the object from an XML file
CUserLog::CUserLog(string strXMLFilename)
: CUserLogBase(NULL), CSettingsUserObserver(NULL) {
  //CFunctionLogger f1("CUserLog::CUserLog(XML)", g_pLogger);

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

  m_pApplicationSpan      = new CTimeSpan("Application", strApp);
  m_vParams               = CUserLogTrial::ParseParamsXML(strParams);

  // Now construct each of the Trial objects based on its section of XML
  for (VECTOR_STRING_ITER iter = vectorTrials.begin(); iter < vectorTrials.end(); iter++)
  {
    CUserLogTrial* pTrial = new CUserLogTrial(*iter, 0);

    if (pTrial != NULL)
      m_vpTrials.push_back(pTrial);
  }

}

// Returns a vector that contains vectors of strings which each 
// contain a tab delimited list of mouse coordinates for each
// navigation cycle.
VECTOR_VECTOR_STRING CUserLog::GetTabMouseXY(bool bReturnNormalized)
{
  //CFunctionLogger f1("CUserLog::GetTabMouseXY", g_pLogger);

  VECTOR_VECTOR_STRING vResult;

  for (VECTOR_USER_LOG_TRIAL_PTR_ITER iter = m_vpTrials.begin();
    iter < m_vpTrials.end();
    iter++)
  {
    if (*iter != NULL)
    {
      VECTOR_STRING vectorTrial = (*iter)->GetTabMouseXY(bReturnNormalized);
      vResult.push_back(vectorTrial);
    }
  }

  return vResult;
}

// Returns a vector that contains a vector of density grids.
VECTOR_VECTOR_DENSITY_GRIDS CUserLog::GetMouseDensity(int iGridSize)
{
  //CFunctionLogger f1("CUserLog::GetMouseDensity", g_pLogger);

  VECTOR_VECTOR_DENSITY_GRIDS vResult;
  for (VECTOR_USER_LOG_TRIAL_PTR_ITER iter = m_vpTrials.begin();
    iter < m_vpTrials.end();
    iter++)
  {
    if (*iter != NULL)
    {
      VECTOR_DENSITY_GRIDS vTrial = (*iter)->GetMouseDensity(iGridSize);
      vResult.push_back(vTrial);
    }
  }

  return vResult;
}

