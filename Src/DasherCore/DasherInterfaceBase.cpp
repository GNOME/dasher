// DasherInterfaceBase.cpp
//
// Copyright (c) 2002 Iain Murray

#include "../Common/Common.h"

#include "DasherInterfaceBase.h"

#include "CustomColours.h"
#include "DasherViewSquare.h"
#include "ControlManager.h"
#include "DasherScreen.h"
#include "DasherView.h"
#include "DasherInput.h"
#include "DasherModel.h"
#include "EventHandler.h"
#include "Event.h"
#include "UserLog.h"
#include "WrapperFactory.h"

// Input filters
#include "ClickFilter.h" 
#include "DefaultFilter.h"
#include "DasherButtons.h"
#include "DynamicFilter.h"
#include "EyetrackerFilter.h"
//#include "OneButtonFilter.h"
#include "OneDimensionalFilter.h"
#include "StylusFilter.h"
#include "TwoButtonDynamicFilter.h"

// STL headers
#include <iostream>
#include <memory>

// Legacy C library headers
namespace {
  #include "stdio.h"
}

// Declare our global file logging object
#include "../DasherCore/FileLogger.h"
#ifdef _DEBUG
const eLogLevel g_iLogLevel   = logDEBUG;
const int       g_iLogOptions = logTimeStamp | logDateStamp | logDeleteOldFile;    
#else
const eLogLevel g_iLogLevel   = logNORMAL;
const int       g_iLogOptions = logTimeStamp | logDateStamp;
#endif
CFileLogger* g_pLogger = NULL;

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CDasherInterfaceBase::CDasherInterfaceBase()
                  :m_Alphabet(0), m_pColours(0), m_pDasherModel(0), m_DasherScreen(0),
		   m_pDasherView(0), m_pInput(0), m_AlphIO(0), m_ColourIO(0), m_pUserLog(NULL),
		   m_pInputFilter(NULL) {
  
  m_bGlobalLock = false;
  m_bShutdownLock = false;

  m_pEventHandler = new CEventHandler(this);
 
  strCurrentContext = ". ";

  strTrainfileBuffer = "";

  // Global logging object we can use from anywhere
  g_pLogger = new CFileLogger("dasher.log",
                              g_iLogLevel,
                              g_iLogOptions);

}

void CDasherInterfaceBase::Realize() {

  // TODO: What exactly needs to have happened by the time we call Realize()?
  CreateSettingsStore();
  SetupUI();
  SetupPaths();

  std::vector<std::string> vAlphabetFiles;
  ScanAlphabetFiles(vAlphabetFiles);
  m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC), GetStringParameter(SP_USER_LOC), vAlphabetFiles);

  std::vector<std::string> vColourFiles;
  ScanColourFiles(vColourFiles);
  m_ColourIO = new CColourIO(GetStringParameter(SP_SYSTEM_LOC), GetStringParameter(SP_USER_LOC), vColourFiles);

  ChangeColours();
  ChangeAlphabet();

  // Create the user logging object if we are suppose to.  We wait
  // until now so we have the real value of the parameter and not
  // just the default.

  int iUserLogLevel = GetLongParameter(LP_USER_LOG_LEVEL_MASK);
  if (iUserLogLevel > 0) 
    m_pUserLog = new CUserLog(m_pEventHandler, m_pSettingsStore, iUserLogLevel, m_Alphabet);  

  CreateFactories();
  CreateInputFilter();
    
  // All the setup is done by now, so let the user log object know
  // that future parameter changes should be logged.
  if (m_pUserLog != NULL) 
    m_pUserLog->InitIsDone();
  
}

CDasherInterfaceBase::~CDasherInterfaceBase() {

  delete m_pDasherModel;        // The order of some of these deletions matters
  delete m_Alphabet;
  delete m_pDasherView;
  delete m_ColourIO;
  delete m_AlphIO;
  delete m_pColours;
  delete m_pInputFilter;
  // Do NOT delete Edit box or Screen. This class did not create them.

  // When we destruct on shutdown, we'll output any detailed log file
  if (m_pUserLog != NULL)
  {
    m_pUserLog->OutputFile();
    delete m_pUserLog;
    m_pUserLog = NULL;
  }

  if (g_pLogger != NULL) {
    delete g_pLogger;
    g_pLogger = NULL;
  }

  // Must delete event handler after all CDasherComponent derived classes

  delete m_pEventHandler;
}

void CDasherInterfaceBase::PreSetNotify(int iParameter, const std::string &sNewValue) {

  // FIXME - make this a more general 'pre-set' event in the message
  // infrastructure

  switch(iParameter) {
  case SP_ALPHABET_ID: 
    // Cycle the alphabet history
    if(GetStringParameter(SP_ALPHABET_ID) != sNewValue) {
      if(GetStringParameter(SP_ALPHABET_1) != sNewValue) {
	if(GetStringParameter(SP_ALPHABET_2) != sNewValue) {
	  if(GetStringParameter(SP_ALPHABET_3) != sNewValue)
	    SetStringParameter(SP_ALPHABET_4, GetStringParameter(SP_ALPHABET_3));
	  
	  SetStringParameter(SP_ALPHABET_3, GetStringParameter(SP_ALPHABET_2));
	}
	
	SetStringParameter(SP_ALPHABET_2, GetStringParameter(SP_ALPHABET_1));
      }
      
      SetStringParameter(SP_ALPHABET_1, GetStringParameter(SP_ALPHABET_ID));
    }

    break;
  }
}

void CDasherInterfaceBase::InterfaceEventHandler(Dasher::CEvent *pEvent) {

  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {

    case BP_COLOUR_MODE:       // Forces us to redraw the display
      // TODO: Is this variable ever used any more?
      Start();
      Redraw(true);
      break;
    case BP_OUTLINE_MODE:
      Redraw(true);
      break;
    case LP_ORIENTATION:
      if(GetLongParameter(LP_ORIENTATION) == Dasher::Opts::AlphabetDefault)
	// TODO: See comment in DasherModel.cpp about prefered values
	SetLongParameter(LP_REAL_ORIENTATION, m_Alphabet->GetOrientation());
      else
	SetLongParameter(LP_REAL_ORIENTATION, GetLongParameter(LP_ORIENTATION));
      Redraw(true);
      break;
    case SP_ALPHABET_ID:
      ChangeAlphabet();
      Start();
      Redraw(true);
      break;
    case SP_COLOUR_ID:
      ChangeColours();
      Redraw(true);
      break;
    case SP_DEFAULT_COLOUR_ID: // Delibarate fallthrough
    case BP_PALETTE_CHANGE: 
      if(GetBoolParameter(BP_PALETTE_CHANGE))
	 SetStringParameter(SP_COLOUR_ID, GetStringParameter(SP_DEFAULT_COLOUR_ID));
      break;
    case LP_LANGUAGE_MODEL_ID:
      CreateDasherModel();
      Start();
      Redraw(true);
      break;
    case LP_LINE_WIDTH:
      Redraw(false); // TODO - make this accessible everywhere
      break;
    case LP_DASHER_FONTSIZE:
      // TODO - make screen a CDasherComponent child?
      Redraw(true);
      break;
    case SP_INPUT_DEVICE:
      CreateInput();
      break;
    case SP_INPUT_FILTER:
      CreateInputFilter();
      break;
    default:
      break;
    }
  }
  else if(pEvent->m_iEventType == 2) {
    CEditEvent *pEditEvent(static_cast < CEditEvent * >(pEvent));
    
    if(pEditEvent->m_iEditType == 1) {
      strCurrentContext += pEditEvent->m_sText;
      if( strCurrentContext.size() > 20 )
	strCurrentContext = strCurrentContext.substr( strCurrentContext.size() - 20 );

      strTrainfileBuffer += pEditEvent->m_sText;
    }
    else if(pEditEvent->m_iEditType == 2) {
      strCurrentContext = strCurrentContext.substr( 0, strCurrentContext.size() - pEditEvent->m_sText.size());

      strTrainfileBuffer = strTrainfileBuffer.substr( 0, strTrainfileBuffer.size() - pEditEvent->m_sText.size());
    }
  }
  else if(pEvent->m_iEventType == EV_CONTROL) {
    CControlEvent *pControlEvent(static_cast <CControlEvent*>(pEvent));

    switch(pControlEvent->m_iID) {
    case CControlManager::CTL_STOP:
      PauseAt(0,0);
      break;
    case CControlManager::CTL_PAUSE:
      Halt();
      break;
    }
  }
  else if(pEvent->m_iEventType == EV_LOCK) {
    // TODO: 'Reference counting' for locks?
    CLockEvent *pLockEvent(static_cast<CLockEvent *>(pEvent));
    m_bGlobalLock = pLockEvent->m_bLock;
  }
}

void CDasherInterfaceBase::WriteTrainFileFull() {
  WriteTrainFile(strTrainfileBuffer);
  strTrainfileBuffer = "";
}

void CDasherInterfaceBase::WriteTrainFilePartial() {
  // TODO: what if we're midway through a unicode character?
  WriteTrainFile(strTrainfileBuffer.substr(0,100));
  strTrainfileBuffer = strTrainfileBuffer.substr(100);
}

void CDasherInterfaceBase::CreateDasherModel() 
{
  if(!m_AlphIO)
    return;
  
  // TODO: Move training into model?
  // TODO: Do we really need to check for a valid language model?
  int lmID = GetLongParameter(LP_LANGUAGE_MODEL_ID);
  if( lmID != -1 ) {

    // Train the new language model
    CLockEvent *pEvent;
    
    pEvent = new CLockEvent("Training Dasher", true, 0);
    m_pEventHandler->InsertEvent(pEvent);
    delete pEvent;

    // Delete the old model and create a new one
    if(m_pDasherModel) {
      delete m_pDasherModel;
      m_pDasherModel = 0;
    }

    if(m_deGameModeStrings.size() == 0)
      m_pDasherModel = new CDasherModel(m_pEventHandler, m_pSettingsStore, this, m_AlphIO);
    else {
      m_pDasherModel = new CDasherModel(m_pEventHandler, m_pSettingsStore, this, m_AlphIO, true, m_deGameModeStrings[0]);
      //      m_deGameModeStrings.pop_front();
    }

    m_Alphabet = m_pDasherModel->GetAlphabetNew();
    
    string T = m_Alphabet->GetTrainingFile();

    int iTotalBytes = 0;
    iTotalBytes += GetFileSize(GetStringParameter(SP_SYSTEM_LOC) + T);
    iTotalBytes += GetFileSize(GetStringParameter(SP_USER_LOC) + T);

    if(iTotalBytes > 0) {
      int iOffset;
      iOffset = TrainFile(GetStringParameter(SP_SYSTEM_LOC) + T, iTotalBytes, 0);
      TrainFile(GetStringParameter(SP_USER_LOC) + T, iTotalBytes, iOffset);
    }
    else {
      CMessageEvent oEvent("No training text is avilable for the selected alphabet. Dasher will function, but it may be difficult to enter text.\nPlease see http://www.dasher.org.uk/alphabets/ for more information.", 0, 0);
      m_pEventHandler->InsertEvent(&oEvent);
    }

    pEvent = new CLockEvent("Training Dasher", false, 0);
    m_pEventHandler->InsertEvent(pEvent);
    delete pEvent;
  }
}

void CDasherInterfaceBase::Start() {
  // TODO: Clarify the relationship between Start() and
  // InvalidateContext() - I believe that they essentially do the same
  // thing
  PauseAt(0, 0);
  if(m_pDasherModel != 0) {
    m_pDasherModel->Start();
  }
  if(m_pDasherView != 0) {
    m_pDasherView->ResetSum();
    m_pDasherView->ResetSumCounter();
    m_pDasherView->ResetYAutoOffset();
  }
}

void CDasherInterfaceBase::PauseAt(int MouseX, int MouseY) {
  SetBoolParameter(BP_DASHER_PAUSED, true);

  // Request a full redraw at the next time step.
  SetBoolParameter(BP_REDRAW, true);

  Dasher::CStopEvent oEvent;
  m_pEventHandler->InsertEvent(&oEvent);

  if (m_pUserLog != NULL)
    m_pUserLog->StopWriting((float) GetNats());
}

void CDasherInterfaceBase::Halt() {
  SetBoolParameter(BP_DASHER_PAUSED, true);

  if(GetBoolParameter(BP_MOUSEPOS_MODE)) {
    SetLongParameter(LP_MOUSE_POS_BOX, 1);
  }

  // This will cause us to reinitialise the frame rate counter - ie we start off slowly
  if(m_pDasherModel != 0)
    m_pDasherModel->Halt();
}

void CDasherInterfaceBase::Unpause(unsigned long Time) {
  SetBoolParameter(BP_DASHER_PAUSED, false);

  if(m_pDasherModel != 0) {
    m_pDasherModel->Reset_framerate(Time);
    //m_pDasherModel->Set_paused(m_Paused);
  }
  if(m_pDasherView != 0) {
    m_pDasherView->ResetSum();
    m_pDasherView->ResetSumCounter();
  }

  Dasher::CStartEvent oEvent;
  m_pEventHandler->InsertEvent(&oEvent);

  ResetNats();
  if (m_pUserLog != NULL)
	  m_pUserLog->StartWriting();
}

void CDasherInterfaceBase::CreateInput() {

  // FIXME - this shouldn't be the model used here - we should just change a parameter and work from the appropriate listener

  if(m_pInput) {
    m_pInput->Deactivate();
    m_pInput->Unref();
  }

  m_pInput = (CDasherInput *)GetModuleByName(GetStringParameter(SP_INPUT_DEVICE));

  if(m_pInput) {
    m_pInput->Ref();
    m_pInput->Activate();
  }

  if(m_pDasherView != 0)
    m_pDasherView->SetInput(m_pInput);
}

void CDasherInterfaceBase::NewFrame(unsigned long iTime) {
  // Fail if Dasher is locked
  if(m_bGlobalLock || m_bShutdownLock)
    return;

  bool bChanged(false);

  if(m_pDasherView != 0) {
    if(!GetBoolParameter(BP_TRAINING)) {
      if (m_pUserLog != NULL) {
	
	Dasher::VECTOR_SYMBOL_PROB vAdded;
	int iNumDeleted = 0;
	
	if(m_pInputFilter) {
	  bChanged = m_pInputFilter->Timer(iTime, m_pDasherView, m_pDasherModel); // FIXME - need logging stuff here
	}
	
	if (iNumDeleted > 0)
	  m_pUserLog->DeleteSymbols(iNumDeleted);
	if (vAdded.size() > 0)
	  m_pUserLog->AddSymbols(&vAdded);
	
      }
      else {
	if(m_pInputFilter) {
	  bChanged = m_pInputFilter->Timer(iTime, m_pDasherView, m_pDasherModel);
	}
      }
      
      m_pDasherModel->CheckForNewRoot(m_pDasherView);
    }
  }

  Redraw(bChanged);

  // This just passes the time through to the framerate tracker, so we
  // know how often new frames are being drawn.
  if(m_pDasherModel != 0)
    m_pDasherModel->NewFrame(iTime);
}

/// Full redraw - renders model, decorations and blits onto display
/// buffer. Not recommended if nothing has changed with the model,
/// otherwise we're wasting effort.

void CDasherInterfaceBase::Redraw(bool bRedrawNodes) {
  if(!m_pDasherView || !m_pDasherModel)
    return;
  
  if(bRedrawNodes) {
    m_pDasherView->Screen()->SendMarker(0);
    m_pDasherModel->RenderToView(m_pDasherView, true);
  }
  
  m_pDasherView->Screen()->SendMarker(1);
  
  bool bDecorationsChanged(false);
  
  if(m_pInputFilter) {
    bDecorationsChanged = m_pInputFilter->DecorateView(m_pDasherView);
  }
  
  if(bRedrawNodes || bDecorationsChanged)
    m_pDasherView->Display();
}

void CDasherInterfaceBase::ChangeAlphabet() {

  if(GetStringParameter(SP_ALPHABET_ID) == "") {
    SetStringParameter(SP_ALPHABET_ID, m_AlphIO->GetDefault());
    // This will result in ChangeAlphabet() being called again, so
    // exit from the first recursion
    return;
  }
  
  // Send a lock event

  WriteTrainFileFull();

  // Lock Dasher to prevent changes from happening while we're training.

  SetBoolParameter( BP_TRAINING, true );

//   m_AlphInfo = m_AlphIO->GetInfo(NewAlphabetID);

//   //AlphabetID = m_AlphInfo.AlphID.c_str();

//   //  std::auto_ptr < CAlphabet > ptrOld(m_Alphabet);       // So we can delete the old alphabet later

//   m_Alphabet = new CAlphabet(m_AlphInfo);

  delete m_pDasherModel;
  m_pDasherModel = 0;
  CreateDasherModel();

  // Let our user log object know about the new alphabet since
  // it needs to convert symbols into text for the log file.
  if (m_pUserLog != NULL)
    m_pUserLog->SetAlphabetPtr(m_Alphabet);

  // Apply options from alphabet

  SetBoolParameter( BP_TRAINING, false );

  Start();

  //}
}

// std::string CDasherInterfaceBase::GetCurrentAlphabet() {
//   return GetStringParameter(SP_ALPHABET_ID);
// }

void CDasherInterfaceBase::ChangeColours() {
  if(!m_ColourIO)
    return;

//   // delete old colours on editing function (ending function? - not really sure what the point of this is - I guess we might fail)
//   std::auto_ptr < CCustomColours > ptrColours(m_pColours);

  if(m_pColours) {
    delete m_pColours;
    m_pColours = 0;
  }

 
  CColourIO::ColourInfo oColourInfo(m_ColourIO->GetInfo(GetStringParameter(SP_COLOUR_ID)));
  m_pColours = new CCustomColours(oColourInfo);

  if(m_DasherScreen != 0) {
    m_DasherScreen->SetColourScheme(m_pColours);
  }
}

void CDasherInterfaceBase::ChangeScreen(CDasherScreen *NewScreen) {
  m_DasherScreen = NewScreen;
  m_DasherScreen->SetColourScheme(m_pColours);

  if(m_pDasherView != 0) {
    m_pDasherView->ChangeScreen(m_DasherScreen);
  } else {
    if(GetLongParameter(LP_VIEW_ID) != -1)
      ChangeView();
  }

  Redraw(true);
}

void CDasherInterfaceBase::ChangeView() {
  // TODO: Actually respond to LP_VIEW_ID parameter (although there is only one view at the moment)

  if(m_DasherScreen != 0 && m_pDasherModel != 0) 
  {
	  delete m_pDasherView;
// 	  if(m_pAutoSpeedControl)
// 	    delete m_pAutoSpeedControl;
 	  m_pDasherView = new CDasherViewSquare(m_pEventHandler, m_pSettingsStore, m_DasherScreen);


	  if (m_pInput)
	    m_pDasherView->SetInput(m_pInput);
  }
}

void CDasherInterfaceBase::GetAlphabets(std::vector <std::string >*AlphabetList) {
  m_AlphIO->GetAlphabets(AlphabetList);
}

const CAlphIO::AlphInfo & CDasherInterfaceBase::GetInfo(const std::string &AlphID) {
  return m_AlphIO->GetInfo(AlphID);
}

void CDasherInterfaceBase::SetInfo(const CAlphIO::AlphInfo &NewInfo) {
  m_AlphIO->SetInfo(NewInfo);
}

void CDasherInterfaceBase::DeleteAlphabet(const std::string &AlphID) {
  m_AlphIO->Delete(AlphID);
}

void CDasherInterfaceBase::GetColours(std::vector <std::string >*ColourList) {
  m_ColourIO->GetColours(ColourList);
}

/*
	I've used C style I/O because I found that C++ style I/O bloated
	the Win32 code enormously. The overhead of loading the buffer into
	the string instead of reading straight into a string seems to be
	negligible compared to huge requirements elsewhere.
*/
int CDasherInterfaceBase::TrainFile(string Filename, int iTotalBytes, int iOffset) {
  if(Filename == "")
    return 0;
  
  FILE *InputFile;
  if((InputFile = fopen(Filename.c_str(), "r")) == (FILE *) 0)
    return 0;

  const int BufferSize = 1024;
  char InputBuffer[BufferSize];
  string StringBuffer;
  int NumberRead;
  int iTotalRead(0);

  vector < symbol > Symbols;

  CDasherModel::CTrainer * pTrainer = m_pDasherModel->GetTrainer();
  do {
    NumberRead = fread(InputBuffer, 1, BufferSize - 1, InputFile);
    InputBuffer[NumberRead] = '\0';
    StringBuffer += InputBuffer;
    bool bIsMore = false;
    if(NumberRead == (BufferSize - 1))
      bIsMore = true;

    Symbols.clear();
    m_Alphabet->GetSymbols(&Symbols, &StringBuffer, bIsMore);

    pTrainer->Train(Symbols);
    iTotalRead += NumberRead;
  
    // TODO: No reason for this to be a pointer (other than cut/paste laziness!)
    CLockEvent *pEvent;
    pEvent = new CLockEvent("Training Dasher", true, static_cast<int>((100.0 * (iTotalRead + iOffset))/iTotalBytes));
    m_pEventHandler->InsertEvent(pEvent);
    delete pEvent;

  } while(NumberRead == BufferSize - 1);

  delete pTrainer;

  fclose(InputFile);

  return iTotalRead;
}

void CDasherInterfaceBase::GetFontSizes(std::vector <int >*FontSizes) const {
  FontSizes->push_back(20);
  FontSizes->push_back(14);
  FontSizes->push_back(11);
  FontSizes->push_back(40);
  FontSizes->push_back(28);
  FontSizes->push_back(22);
  FontSizes->push_back(80);
  FontSizes->push_back(56);
  FontSizes->push_back(44);
}

double CDasherInterfaceBase::GetCurCPM() {
  //
  return 0;
}

double CDasherInterfaceBase::GetCurFPS() {
  //
  return 0;
}

int CDasherInterfaceBase::GetAutoOffset() {
  if(m_pDasherView != 0) {
    return m_pDasherView->GetAutoOffset();
  }
  return -1;
}

double CDasherInterfaceBase::GetNats() const {
  if(m_pDasherModel)
    return m_pDasherModel->GetNats();
  else
    return 0.0;
}

void CDasherInterfaceBase::ResetNats() {
  if(m_pDasherModel)
    m_pDasherModel->ResetNats();
}


void CDasherInterfaceBase::InvalidateContext(bool bForceStart) {

  

  std::cout << "Invalidating context" << std::endl;



  m_pDasherModel->m_strContextBuffer = "";

  Dasher::CEditContextEvent oEvent(10);
  m_pEventHandler->InsertEvent(&oEvent);

   std::string strNewContext(m_pDasherModel->m_strContextBuffer);

  // We keep track of an internal context and compare that to what
  // we are given - don't restart Dasher if nothing has changed.
  // This should really be integrated with DasherModel, which
  // probably will be the case when we start to deal with being able
  // to back off indefinitely. For now though we'll keep it in a
  // separate string.

   int iContextLength( 6 ); // The 'important' context length - should really get from language model

   // FIXME - use unicode lengths

   if(bForceStart || (strNewContext.substr( std::max(static_cast<int>(strNewContext.size()) - iContextLength, 0)) != strCurrentContext.substr( std::max(static_cast<int>(strCurrentContext.size()) - iContextLength, 0)))) {

     if(m_pDasherModel != NULL) {
       if(m_pDasherModel->m_bContextSensitive || bForceStart) {
 	m_pDasherModel->SetContext(strNewContext);
 	PauseAt(0,0);
       }
     }
    
     strCurrentContext = strNewContext;
     WriteTrainFileFull();
   }

  

   if(m_pDasherView)
     while( m_pDasherModel->CheckForNewRoot(m_pDasherView) ) {
       // Do nothing
     }

   Redraw(true);
}


void CDasherInterfaceBase::SetContext(std::string strNewContext) {
  m_pDasherModel->m_strContextBuffer = strNewContext;
}

// Control mode stuff

void CDasherInterfaceBase::RegisterNode( int iID, const std::string &strLabel, int iColour ) {
  m_pDasherModel->RegisterNode(iID, strLabel, iColour);
}

void CDasherInterfaceBase::ConnectNode(int iChild, int iParent, int iAfter) {
  m_pDasherModel->ConnectNode(iChild, iParent, iAfter);
}

void CDasherInterfaceBase::DisconnectNode(int iChild, int iParent) {
  m_pDasherModel->DisconnectNode(iChild, iParent);
}

void CDasherInterfaceBase::SetBoolParameter(int iParameter, bool bValue) {
  m_pSettingsStore->SetBoolParameter(iParameter, bValue);
};

void CDasherInterfaceBase::SetLongParameter(int iParameter, long lValue) { 
  m_pSettingsStore->SetLongParameter(iParameter, lValue);
};

void CDasherInterfaceBase::SetStringParameter(int iParameter, const std::string & sValue) {
  PreSetNotify(iParameter, sValue);
  m_pSettingsStore->SetStringParameter(iParameter, sValue);
};

bool CDasherInterfaceBase::GetBoolParameter(int iParameter) {
  return m_pSettingsStore->GetBoolParameter(iParameter);
}

long CDasherInterfaceBase::GetLongParameter(int iParameter) {
  return m_pSettingsStore->GetLongParameter(iParameter);
}

std::string CDasherInterfaceBase::GetStringParameter(int iParameter) {
  return m_pSettingsStore->GetStringParameter(iParameter);
}

void CDasherInterfaceBase::ResetParameter(int iParameter) {
  m_pSettingsStore->ResetParameter(iParameter);
}

// We need to be able to get at the UserLog object from outside the interface
CUserLog* CDasherInterfaceBase::GetUserLogPtr() {
  return m_pUserLog;
}

void CDasherInterfaceBase::KeyDown(int iTime, int iId) {
  if(m_pInputFilter && !GetBoolParameter(BP_TRAINING)) {
    m_pInputFilter->KeyDown(iTime, iId, m_pDasherModel);
  }
}

void CDasherInterfaceBase::KeyUp(int iTime, int iId) {
  if(m_pInputFilter && !GetBoolParameter(BP_TRAINING)) {
    m_pInputFilter->KeyUp(iTime, iId, m_pDasherModel);
  }
}

void CDasherInterfaceBase::CreateInputFilter()
{
  if(m_pInputFilter) {
    m_pInputFilter->Deactivate();
    m_pInputFilter->Unref();
    m_pInputFilter = NULL;
  }

  m_pInputFilter = (CInputFilter *)GetModuleByName(GetStringParameter(SP_INPUT_FILTER));

  if(m_pInputFilter) {
    m_pInputFilter->Ref();
    m_pInputFilter->Activate();
  }
}

void CDasherInterfaceBase::RegisterFactory(CModuleFactory *pFactory) {
  m_oModuleManager.RegisterFactory(pFactory);
}
 
CDasherModule *CDasherInterfaceBase::GetModule(long long int iID) {
   return m_oModuleManager.GetModule(iID);
}

CDasherModule *CDasherInterfaceBase::GetModuleByName(const std::string &strName) {
   return m_oModuleManager.GetModuleByName(strName);
}


void CDasherInterfaceBase::CreateFactories() {
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDefaultFilter(m_pEventHandler, m_pSettingsStore, this, m_pDasherModel,3, "Normal Control")));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new COneDimensionalFilter(m_pEventHandler, m_pSettingsStore, this, m_pDasherModel)));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CEyetrackerFilter(m_pEventHandler, m_pSettingsStore, this, m_pDasherModel)));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CClickFilter(m_pEventHandler, m_pSettingsStore, this)));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDynamicFilter(m_pEventHandler, m_pSettingsStore, this)));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CTwoButtonDynamicFilter(m_pEventHandler, m_pSettingsStore, this, 14, 1, "Two Button Dynamic Mode")));
  // TODO: specialist factory for button mode
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDasherButtons(m_pEventHandler, m_pSettingsStore, this, 5, 1, true,8, "Menu Mode")));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDasherButtons(m_pEventHandler, m_pSettingsStore, this, 3, 0, false,10, "Direct Mode")));
  //  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDasherButtons(m_pEventHandler, m_pSettingsStore, this, 4, 0, false,11, "Buttons 3")));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDasherButtons(m_pEventHandler, m_pSettingsStore, this, 3, 3, false,12, "Alternating Direct Mode")));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDasherButtons(m_pEventHandler, m_pSettingsStore, this, 4, 2, false,13, "Compass Mode")));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CStylusFilter(m_pEventHandler, m_pSettingsStore, this, m_pDasherModel,15, "Stylus Control")));

}

void CDasherInterfaceBase::GetPermittedValues(int iParameter, std::vector<std::string> &vList) {
  // TODO: Deprecate direct calls to these functions
  switch (iParameter) {
  case SP_ALPHABET_ID:
    GetAlphabets(&vList);
    break;
  case SP_COLOUR_ID:
    GetColours(&vList);
    break;
  case SP_INPUT_FILTER:
    m_oModuleManager.ListModules(1, vList);
    break;
  case SP_INPUT_DEVICE:
    m_oModuleManager.ListModules(0, vList);
    break;
  }
}

void CDasherInterfaceBase::StartShutdown() {
  m_bShutdownLock = true;
}

bool CDasherInterfaceBase::GetModuleSettings(const std::string &strName, SModuleSettings **pSettings, int *iCount) {
  return GetModuleByName(strName)->GetSettings(pSettings, iCount);
}
