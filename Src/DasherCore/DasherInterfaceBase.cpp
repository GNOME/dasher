// DasherInterfaceBase.cpp
//
// Copyright (c) 2008 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "../Common/Common.h"

#include "DasherInterfaceBase.h"

//#include "ActionButton.h"
#include "DasherViewSquare.h"
#include "ControlManager.h"
#include "DasherScreen.h"
#include "DasherView.h"
#include "DasherInput.h"
#include "DasherModel.h"
#include "EventHandler.h"
#include "Event.h"
#include "NodeCreationManager.h"
#ifndef _WIN32_WCE
#include "UserLog.h"
#include "BasicLog.h"
#endif
#include "DasherGameMode.h"

// Input filters
#include "AlternatingDirectMode.h"
#include "ButtonMode.h"
#include "ClickFilter.h"
#include "CompassMode.h"
#include "DefaultFilter.h"

#include "OneButtonFilter.h"
#include "OneButtonDynamicFilter.h"
#include "OneDimensionalFilter.h"
#include "StylusFilter.h"
#include "TwoButtonDynamicFilter.h"
#include "TwoPushDynamicFilter.h"

// STL headers
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>

// Declare our global file logging object
#include "../DasherCore/FileLogger.h"
#ifdef _DEBUG
const eLogLevel g_iLogLevel   = logDEBUG;
const int       g_iLogOptions = logTimeStamp | logDateStamp | logDeleteOldFile;
#else
const eLogLevel g_iLogLevel   = logNORMAL;
const int       g_iLogOptions = logTimeStamp | logDateStamp;
#endif

#ifndef _WIN32_WCE
CFileLogger* g_pLogger = NULL;
#endif

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CDasherInterfaceBase::CDasherInterfaceBase() : m_pLockLabel(NULL) {

  // Ensure that pointers to 'owned' objects are set to NULL.
  m_pDasherModel = NULL;
  m_DasherScreen = NULL;
  m_pDasherView = NULL;
  m_pInput = NULL;
  m_pInputFilter = NULL;
  m_AlphIO = NULL;
  m_ColourIO = NULL;
  m_pUserLog = NULL;
  m_pNCManager = NULL;
  m_defaultPolicy = NULL;

  // Various state variables
  m_bRedrawScheduled = false;

  m_iCurrentState = ST_START;

  //  m_bGlobalLock = false;

  m_strCurrentWord = "";

  // Create an event handler.
  m_pEventHandler = new CEventHandler(this);

  m_bLastChanged = true;

#ifndef _WIN32_WCE
  // Global logging object we can use from anywhere
  g_pLogger = new CFileLogger("dasher.log",
                              g_iLogLevel,
                              g_iLogOptions);
#endif

}

void CDasherInterfaceBase::Realize() {

  // TODO: What exactly needs to have happened by the time we call Realize()?
  CreateSettingsStore();
  
  //create a view, if we have a screen...
  //if(GetLongParameter(LP_VIEW_ID) != -1)
  ChangeView();

  //create the model... (no nodes just yet)
  m_pDasherModel = new CDasherModel(m_pEventHandler, m_pSettingsStore, this);

  SetupUI();
  SetupPaths();

  std::vector<std::string> vAlphabetFiles;
  ScanAlphabetFiles(vAlphabetFiles);
  m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC), GetStringParameter(SP_USER_LOC), vAlphabetFiles);

  std::vector<std::string> vColourFiles;
  ScanColourFiles(vColourFiles);
  m_ColourIO = new CColourIO(GetStringParameter(SP_SYSTEM_LOC), GetStringParameter(SP_USER_LOC), vColourFiles);

  ChangeColours();

  // Create the user logging object if we are suppose to.  We wait
  // until now so we have the real value of the parameter and not
  // just the default.

  // TODO: Sort out log type selection

#ifndef _WIN32_WCE
  int iUserLogLevel = GetLongParameter(LP_USER_LOG_LEVEL_MASK);

  if(iUserLogLevel == 10)
    m_pUserLog = new CBasicLog(m_pEventHandler, m_pSettingsStore);
  else if (iUserLogLevel > 0)
    m_pUserLog = new CUserLog(m_pEventHandler, m_pSettingsStore, iUserLogLevel);
#else
  m_pUserLog = NULL;
#endif

  CreateModules();

  CreateInput();
  CreateInputFilter();

  ChangeAlphabet(); // This creates the NodeCreationManager, the Alphabet,
  //and the tree of nodes in the model. Now we can
  // Notify the teacher of the new model...ACL TODO pending merging of new
  // game mode code; when Ryan's stuff arrives we can delete this:
  if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
    pTeacher->SetDasherModel(m_pDasherModel);

  SetupActionButtons();
  CParameterNotificationEvent oEvent(LP_NODE_BUDGET);
  InterfaceEventHandler(&oEvent);

  // FIXME - need to rationalise this sort of thing.
  // InvalidateContext(true);
  ScheduleRedraw();

#ifndef _WIN32_WCE
  // All the setup is done by now, so let the user log object know
  // that future parameter changes should be logged.
  if (m_pUserLog != NULL)
    m_pUserLog->InitIsDone();
#endif

  // TODO: Make things work when model is created latet
  ChangeState(TR_MODEL_INIT);

  using GameMode::CDasherGameMode;
  // Create the teacher singleton object.
  CDasherGameMode::CreateTeacher(m_pEventHandler, m_pSettingsStore, this);
  CDasherGameMode::GetTeacher()->SetDasherView(m_pDasherView);
  CDasherGameMode::GetTeacher()->SetDasherModel(m_pDasherModel);
}

CDasherInterfaceBase::~CDasherInterfaceBase() {
  DASHER_ASSERT(m_iCurrentState == ST_SHUTDOWN);

  // It may seem odd that InterfaceBase does not "own" the teacher.
  // This is because game mode is a different layer, in a sense.
  GameMode::CDasherGameMode::DestroyTeacher();

  delete m_pDasherModel;        // The order of some of these deletions matters
  delete m_pDasherView;
  delete m_ColourIO;
  delete m_AlphIO;
  delete m_pNCManager;
  // Do NOT delete Edit box or Screen. This class did not create them.

#ifndef _WIN32_WCE
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
#endif


  for (std::vector<CActionButton *>::iterator it=m_vLeftButtons.begin(); it != m_vLeftButtons.end(); ++it)
    delete *it;

  for (std::vector<CActionButton *>::iterator it=m_vRightButtons.begin(); it != m_vRightButtons.end(); ++it)
    delete *it;

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

  if(pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {

    case LP_OUTLINE_WIDTH:
      ScheduleRedraw();
      break;
    case BP_DRAW_MOUSE:
      ScheduleRedraw();
      break;
    case BP_CONTROL_MODE:
        //force rebuilding tree/nodes, to get new probabilities (inc/exc control node).
        // This may move the canvas around a bit, but at least manages to keep/reuse the
        // existing AlphabetManager, NCManager, etc. objects...
        SetOffset(m_pDasherModel->GetOffset(), true);
      ScheduleRedraw();
      break;
    case BP_DRAW_MOUSE_LINE:
      ScheduleRedraw();
      break;
    case LP_REAL_ORIENTATION:
      ScheduleRedraw();
      break;
    case SP_ALPHABET_ID:
      ChangeAlphabet();
      ScheduleRedraw();
      break;
    case SP_COLOUR_ID:
      ChangeColours();
      ScheduleRedraw();
      break;
    case SP_DEFAULT_COLOUR_ID: // Delibarate fallthrough
    case BP_PALETTE_CHANGE:
      if(GetBoolParameter(BP_PALETTE_CHANGE))
	 SetStringParameter(SP_COLOUR_ID, GetStringParameter(SP_DEFAULT_COLOUR_ID));
      break;
    case LP_LANGUAGE_MODEL_ID:
      CreateNCManager();
      break;
    case LP_LINE_WIDTH:
      ScheduleRedraw();
      break;
    case LP_DASHER_FONTSIZE:
      ScheduleRedraw();
      break;
    case SP_INPUT_DEVICE:
      CreateInput();
      break;
    case SP_INPUT_FILTER:
      CreateInputFilter();
      ScheduleRedraw();
      break;
    case BP_DASHER_PAUSED:
      ScheduleRedraw();
      break;
    case LP_MARGIN_WIDTH:
    case BP_NONLINEAR_Y:
    case LP_NONLINEAR_X:
    case LP_GEOMETRY:
    case LP_SHAPE_TYPE: //for platforms which actually have this as a GUI pref!
        ScheduleRedraw();
        break;
    case LP_NODE_BUDGET:
      delete m_defaultPolicy;
      m_defaultPolicy = new AmortizedPolicy(GetLongParameter(LP_NODE_BUDGET));
    default:
      break;
    }
  }
  else if(pEvent->m_iEventType == EV_EDIT && !GetBoolParameter(BP_GAME_MODE)) {
    CEditEvent *pEditEvent(static_cast < CEditEvent * >(pEvent));

    if(pEditEvent->m_iEditType == 1) {
      if (GetBoolParameter(BP_SPEAK_WORDS) && SupportsSpeech()) {
        const CAlphInfo *pAlphabet = m_pNCManager->GetAlphabet();
        if (pEditEvent->m_sText == pAlphabet->GetText(pAlphabet->GetSpaceSymbol())) {
          Speak(m_strCurrentWord, false);
          m_strCurrentWord="";
        } else
          m_strCurrentWord+=pEditEvent->m_sText;
      }
    }
    else if(pEditEvent->m_iEditType == 2) {
      if (GetBoolParameter(BP_SPEAK_WORDS))
        m_strCurrentWord = m_strCurrentWord.substr(0, max(static_cast<string::size_type>(0), m_strCurrentWord.size()-pEditEvent->m_sText.size()));
    }
  }
}

void CDasherInterfaceBase::SetLockStatus(const string &strText, int iPercent) {
  string newMessage; //empty - what we want if iPercent==-1 (unlock)
  if (iPercent!=-1) {
    ostringstream os;
    os << (strText.empty() ? "Training Dasher" : strText);
    if (iPercent) os << " " << iPercent << "%";
    newMessage = os.str();
  }
  if (newMessage != m_strLockMessage) {
    ScheduleRedraw();
    if (m_pLockLabel) {
      delete m_pLockLabel;
      m_pLockLabel = NULL;
    }
    m_strLockMessage = newMessage;
  }
}

void CDasherInterfaceBase::WriteTrainFileFull() {
  m_pNCManager->GetAlphabetManager()->WriteTrainFileFull(this);
}

void CDasherInterfaceBase::CreateNCManager() {

  if(!m_AlphIO || GetLongParameter(LP_LANGUAGE_MODEL_ID)==-1)
    return;

  //can't delete the old manager yet until we've deleted all its nodes...
  CNodeCreationManager *pOldMgr = m_pNCManager;

  //now create the new manager...
  m_pNCManager = new CNodeCreationManager(this, m_pEventHandler, m_pSettingsStore, m_AlphIO);

  if (m_DasherScreen) {
    m_pNCManager->ChangeScreen(m_DasherScreen);
    //and start a new tree of nodes from it (retaining old offset -
    // this will be a sensible default of 0 if no nodes previously existed).
    // This deletes the old tree of nodes...
    m_pDasherModel->SetOffset(m_pDasherModel->GetOffset(), m_pNCManager->GetAlphabetManager(), m_pDasherView, true);
  } //else, if there is no screen, the model should not contain any nodes from the old NCManager. (Assert, somehow?)

  //...so now we can delete the old manager
  delete pOldMgr;
}

CDasherInterfaceBase::TextAction::TextAction(CDasherInterfaceBase *pIntf) : m_pIntf(pIntf) {
  m_iStartOffset= (pIntf->m_pDasherModel) ? pIntf->m_pDasherModel->GetOffset() : 0;
  pIntf->m_vTextActions.insert(this);
}

CDasherInterfaceBase::TextAction::~TextAction() {
  m_pIntf->m_vTextActions.erase(this);
}

void CDasherInterfaceBase::TextAction::executeOnAll() {
  (*this)(strLast = m_pIntf->GetAllContext());
  m_iStartOffset = m_pIntf->m_pDasherModel->GetOffset();
}

void CDasherInterfaceBase::TextAction::executeOnNew() {
  int iNewOffset(m_pIntf->m_pDasherModel->GetOffset());
  (*this)(strLast = m_pIntf->GetContext(m_iStartOffset, iNewOffset-m_iStartOffset));
  m_iStartOffset=iNewOffset;
}

void CDasherInterfaceBase::TextAction::executeLast() {
  (*this)(strLast);
}

void CDasherInterfaceBase::TextAction::NotifyOffset(int iOffset) {
  m_iStartOffset = min(iOffset, m_iStartOffset);
}


bool CDasherInterfaceBase::hasStopTriggers() {
  return (GetBoolParameter(BP_COPY_ALL_ON_STOP) && SupportsClipboard())
  || (GetBoolParameter(BP_SPEAK_ALL_ON_STOP) && SupportsSpeech());
}

void CDasherInterfaceBase::Stop() {
  if (GetBoolParameter(BP_DASHER_PAUSED)) return; //already paused, no need to do anything.
  SetBoolParameter(BP_DASHER_PAUSED, true);

  // Request a full redraw at the next time step.
  SetBoolParameter(BP_REDRAW, true);

#ifndef _WIN32_WCE
  if (m_pUserLog != NULL)
    m_pUserLog->StopWriting((float) GetNats());
#endif

  if (GetBoolParameter(BP_COPY_ALL_ON_STOP) && SupportsClipboard()) {
    CopyToClipboard(GetAllContext());
  }
  if (GetBoolParameter(BP_SPEAK_ALL_ON_STOP) && SupportsSpeech()) {
    Speak(GetAllContext(), true);
  }
}

void CDasherInterfaceBase::GameMessageIn(int message, void* messagedata) {
  GameMode::CDasherGameMode::GetTeacher()->Message(message, messagedata);
}

void CDasherInterfaceBase::Unpause(unsigned long Time) {
  if (!GetBoolParameter(BP_DASHER_PAUSED)) return; //already running, no need to do anything
  SetBoolParameter(BP_DASHER_PAUSED, false);

  if(m_pDasherModel != 0)
    m_pDasherModel->Reset_framerate(Time);

#ifndef _WIN32_WCE
  if (m_pUserLog != NULL)
    m_pUserLog->StartWriting();
#endif
}

void CDasherInterfaceBase::CreateInput() {
  if(m_pInput) {
    m_pInput->Deactivate();
  }

  m_pInput = (CDasherInput *)GetModuleByName(GetStringParameter(SP_INPUT_DEVICE));

  if (m_pInput == NULL)
    m_pInput = m_oModuleManager.GetDefaultInputDevice();

  if(m_pInput) {
    m_pInput->Activate();
  }
}

void CDasherInterfaceBase::NewFrame(unsigned long iTime, bool bForceRedraw) {
  // Prevent NewFrame from being reentered. This can happen occasionally and
  // cause crashes.
  static bool bReentered=false;
  if (bReentered) {
#ifdef DEBUG
    std::cout << "CDasherInterfaceBase::NewFrame was re-entered" << std::endl;
#endif
    return;
  }
  bReentered=true;

  // Fail if Dasher is locked
  // if(m_iCurrentState != ST_NORMAL)
  //  return;

  bool bChanged(false), bWasPaused(GetBoolParameter(BP_DASHER_PAUSED));
  CExpansionPolicy *pol=m_defaultPolicy;
  if(m_pDasherView != 0) {
    if (isLocked()) {
      //Hmmm. If we're locked, NewFrame is never actually called - the thread
      // that would be rendering frames, is the same one doing the training.
      // So the following is never actually executed atm, but may be a simple
      // template if/when we ever implement multithreading widely/properly...
      m_DasherScreen->SendMarker(0); //this replaces the nodes...
      const screenint iSW = m_DasherScreen->GetWidth(), iSH = m_DasherScreen->GetHeight();
      m_DasherScreen->DrawRectangle(0,0,iSW,iSH,0,0,0); //fill in colour 0 = white
      unsigned int iSize(GetLongParameter(LP_MESSAGE_FONTSIZE));
      if (!m_pLockLabel) m_pLockLabel = m_DasherScreen->MakeLabel(m_strLockMessage, iSize);
      pair<screenint,screenint> dims = m_DasherScreen->TextSize(m_pLockLabel, iSize);
      m_DasherScreen->DrawString(m_pLockLabel, (iSW-dims.first)/2, (iSH-dims.second)/2, iSize, 4);
      m_DasherScreen->SendMarker(1); //decorations - don't draw any
      m_DasherScreen->Display();
    } else {
      if (m_pUserLog != NULL) {
        //ACL note that as of 15/5/09, splitting UpdatePosition into two,
        //DasherModel no longer guarantees to empty these two if it didn't do anything.
        //So initialise appropriately...
        Dasher::VECTOR_SYMBOL_PROB vAdded;
        int iNumDeleted = 0;

        if(m_pInputFilter) {
          bChanged = m_pInputFilter->Timer(iTime, m_pDasherView, m_pInput, m_pDasherModel, &vAdded, &iNumDeleted, &pol);
        }

      #ifndef _WIN32_WCE
        if (iNumDeleted > 0)
          m_pUserLog->DeleteSymbols(iNumDeleted);
        if (vAdded.size() > 0)
          m_pUserLog->AddSymbols(&vAdded);
      #endif

      } else {
        if(m_pInputFilter) {
          bChanged = m_pInputFilter->Timer(iTime, m_pDasherView, m_pInput, m_pDasherModel, 0, 0, &pol);
        }
      }
      //check: if we were paused before, and the input filter didn't unpause,
      // then nothing can have changed:
      DASHER_ASSERT(!bWasPaused || !GetBoolParameter(BP_DASHER_PAUSED) || !bChanged);
     
      // Flags at this stage:
      //
      // - bChanged = the display was updated, so needs to be rendered to the display
      // - m_bLastChanged = bChanged was true last time around
      // - m_bRedrawScheduled = Display invalidated internally
      // - bForceRedraw = Display invalidated externally
     
      // TODO: Would be good to sort out / check through the redraw logic properly
     
      bForceRedraw |= m_bLastChanged || bChanged || m_bRedrawScheduled;
      m_bLastChanged = bChanged; //will also be set in Redraw if any nodes were expanded.
     
      Redraw(bForceRedraw, *pol);
     
      m_bRedrawScheduled = false;
     
      // This just passes the time through to the framerate tracker, so we
      // know how often new frames are being drawn.
      if(m_pDasherModel != 0)
        m_pDasherModel->RecordFrame(iTime);
    }
  }

  bReentered=false;
}

void CDasherInterfaceBase::Redraw(bool bRedrawNodes, CExpansionPolicy &policy) {
  // No point continuing if there's nothing to draw on...
  if(!m_pDasherView)
    return;

  // Draw the nodes
  if(bRedrawNodes) {
    m_pDasherView->Screen()->SendMarker(0);
	if (m_pDasherModel) {
		m_pDasherModel->RenderToView(m_pDasherView,policy);
		m_bLastChanged |= policy.apply(m_pNCManager, m_pDasherModel);
        }
  }

  // Draw the decorations
  m_pDasherView->Screen()->SendMarker(1);

  if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
    pTeacher->DrawGameDecorations(m_pDasherView);

  bool bDecorationsChanged(false);

  if(m_pInputFilter) {
    bDecorationsChanged = m_pInputFilter->DecorateView(m_pDasherView, m_pInput);
  }

  bool bActionButtonsChanged(false);
#ifdef EXPERIMENTAL_FEATURES
  bActionButtonsChanged = DrawActionButtons();
#endif

  // Only blit the image to the display if something has actually changed
  if(bRedrawNodes || bDecorationsChanged || bActionButtonsChanged)
    m_pDasherView->Screen()->Display();
}

void CDasherInterfaceBase::ChangeAlphabet() {
  if(GetStringParameter(SP_ALPHABET_ID) == "") {
    SetStringParameter(SP_ALPHABET_ID, m_AlphIO->GetDefault());
    // This will result in ChangeAlphabet() being called again, so
    // exit from the first recursion
    return;
  }

  if (m_pNCManager) WriteTrainFileFull(); //can't/don't before creating first NCManager

  // Send a lock event

  // Lock Dasher to prevent changes from happening while we're training.

  CreateNCManager();

  // Apply options from alphabet

  //}
}

void CDasherInterfaceBase::ChangeColours() {
  if(!m_ColourIO || !m_DasherScreen)
    return;

  // TODO: Make fuction return a pointer directly
  m_DasherScreen->SetColourScheme(&(m_ColourIO->GetInfo(GetStringParameter(SP_COLOUR_ID))));
}

void CDasherInterfaceBase::ChangeScreen(CDasherScreen *NewScreen) {
  
  m_DasherScreen = NewScreen;
  ChangeColours();
  
  if(m_pDasherView != 0) {
    m_pDasherView->ChangeScreen(NewScreen);
    ScreenResized(NewScreen);
  } else if (m_pEventHandler && m_pSettingsStore) {
    //no screen, but other essential components (created in Realize) present.
    // IOW, (assume) we were delaying creating a View, until we had a screen...
    ChangeView();
  }
  
  if (m_pNCManager) {
    m_pNCManager->ChangeScreen(m_DasherScreen);
    if (m_pDasherModel)
      m_pDasherModel->SetOffset(m_pDasherModel->GetOffset(), m_pNCManager->GetAlphabetManager(), m_pDasherView, true);
  }
}

void CDasherInterfaceBase::ScreenResized(CDasherScreen *pScreen) {
  DASHER_ASSERT(pScreen == m_DasherScreen);
  if (!m_pDasherView) return;
  m_pDasherView->ScreenResized(m_DasherScreen);

  PositionActionButtons();
  BudgettingPolicy pol(GetLongParameter(LP_NODE_BUDGET)); //maintain budget, but allow arbitrary amount of work.
  Redraw(true, pol); // (we're assuming resolution changes are occasional, i.e.
	// we don't need to worry about maintaining the frame rate, so we can do
	// as much work as necessary. However, it'd probably be better still to
  // get a node queue from the input filter, as that might have a different
  // policy / budget.
}

void CDasherInterfaceBase::ChangeView() {
  // TODO: Actually respond to LP_VIEW_ID parameter (although there is only one view at the moment)

  // removed condition that m_pDasherModel != 0. Surely the view can exist without the model?-pconlon
  if(m_DasherScreen != 0 /*&& m_pDasherModel != 0*/) {
    delete m_pDasherView;

    m_pDasherView = new CDasherViewSquare(m_pEventHandler, m_pSettingsStore, m_DasherScreen);

    // Tell the Teacher which view we are using
    if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
      pTeacher->SetDasherView(m_pDasherView);
  }
}

const CAlphInfo *CDasherInterfaceBase::GetInfo(const std::string &AlphID) {
  return m_AlphIO->GetInfo(AlphID);
}

double CDasherInterfaceBase::GetCurCPM() {
  //
  return 0;
}

double CDasherInterfaceBase::GetCurFPS() {
  //
  return 0;
}

// int CDasherInterfaceBase::GetAutoOffset() {
//   if(m_pDasherView != 0) {
//     return m_pDasherView->GetAutoOffset();
//   }
//   return -1;
// }

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

void CDasherInterfaceBase::ClearAllContext() {
  ctrlDelete(true, CControlManager::EDIT_FILE);
  ctrlDelete(false, CControlManager::EDIT_FILE);
  SetBuffer(0);
}

void CDasherInterfaceBase::SetBoolParameter(int iParameter, bool bValue) {
  m_pSettingsStore->SetBoolParameter(iParameter, bValue);
}

void CDasherInterfaceBase::SetLongParameter(int iParameter, long lValue) {
  m_pSettingsStore->SetLongParameter(iParameter, lValue);
}

void CDasherInterfaceBase::SetStringParameter(int iParameter, const std::string & sValue) {
  PreSetNotify(iParameter, sValue);
  m_pSettingsStore->SetStringParameter(iParameter, sValue);
}

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
CUserLogBase* CDasherInterfaceBase::GetUserLogPtr() {
  return m_pUserLog;
}

void CDasherInterfaceBase::KeyDown(int iTime, int iId, bool bPos, int iX, int iY) {
  if(m_iCurrentState != ST_NORMAL || isLocked())
    return;

  if(m_pInputFilter) {
    m_pInputFilter->KeyDown(iTime, iId, m_pDasherView, m_pInput, m_pDasherModel, m_pUserLog, bPos, iX, iY);
  }

  if(m_pInput) {
    m_pInput->KeyDown(iTime, iId);
  }
}

void CDasherInterfaceBase::KeyUp(int iTime, int iId, bool bPos, int iX, int iY) {
  if(m_iCurrentState != ST_NORMAL || isLocked())
    return;

  if(m_pInputFilter) {
    m_pInputFilter->KeyUp(iTime, iId, m_pDasherView, m_pInput, m_pDasherModel, bPos, iX, iY);
  }

  if(m_pInput) {
    m_pInput->KeyUp(iTime, iId);
  }
}

void CDasherInterfaceBase::CreateInputFilter()
{
  if(m_pInputFilter) {
    m_pInputFilter->Deactivate();
    m_pInputFilter = NULL;
  }

#ifndef _WIN32_WCE
  m_pInputFilter = (CInputFilter *)GetModuleByName(GetStringParameter(SP_INPUT_FILTER));
#endif

  if (m_pInputFilter == NULL)
    m_pInputFilter = m_oModuleManager.GetDefaultInputMethod();

  m_pInputFilter->Activate();
}

CDasherModule *CDasherInterfaceBase::RegisterModule(CDasherModule *pModule) {
    return m_oModuleManager.RegisterModule(pModule);
}

CDasherModule *CDasherInterfaceBase::GetModule(ModuleID_t iID) {
    return m_oModuleManager.GetModule(iID);
}

CDasherModule *CDasherInterfaceBase::GetModuleByName(const std::string &strName) {
    return m_oModuleManager.GetModuleByName(strName);
}

void CDasherInterfaceBase::SetDefaultInputDevice(CDasherInput *pModule) {
    m_oModuleManager.SetDefaultInputDevice(pModule);
}

void CDasherInterfaceBase::SetDefaultInputMethod(CInputFilter *pModule) {
    m_oModuleManager.SetDefaultInputMethod(pModule);
}

void CDasherInterfaceBase::CreateModules() {
  CInputFilter *defFil = new CDefaultFilter(m_pEventHandler, m_pSettingsStore, this, 3, _("Normal Control"));
  RegisterModule(defFil);
  SetDefaultInputMethod(defFil);
  RegisterModule(new COneDimensionalFilter(m_pEventHandler, m_pSettingsStore, this));
#ifndef _WIN32_WCE
  RegisterModule(new CClickFilter(m_pEventHandler, m_pSettingsStore, this));
#else
  SetDefaultInputMethod(
    RegisterModule(new CClickFilter(m_pEventHandler, m_pSettingsStore, this));
  );
#endif
  RegisterModule(new COneButtonFilter(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new COneButtonDynamicFilter(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CTwoButtonDynamicFilter(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CTwoPushDynamicFilter(m_pEventHandler, m_pSettingsStore, this));
  // TODO: specialist factory for button mode
  RegisterModule(new CButtonMode(m_pEventHandler, m_pSettingsStore, this, true, 8, _("Menu Mode")));
  RegisterModule(new CButtonMode(m_pEventHandler, m_pSettingsStore, this, false,10, _("Direct Mode")));
  //  RegisterModule(new CDasherButtons(m_pEventHandler, m_pSettingsStore, this, 4, 0, false,11, "Buttons 3"));
  RegisterModule(new CAlternatingDirectMode(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CCompassMode(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CStylusFilter(m_pEventHandler, m_pSettingsStore, this, 15, _("Stylus Control")));
}

void CDasherInterfaceBase::GetPermittedValues(int iParameter, std::vector<std::string> &vList) {
  // TODO: Deprecate direct calls to these functions
  switch (iParameter) {
  case SP_ALPHABET_ID:
    if(m_AlphIO)
      m_AlphIO->GetAlphabets(&vList);
    break;
  case SP_COLOUR_ID:
    if(m_ColourIO)
      m_ColourIO->GetColours(&vList);
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
  ChangeState(TR_SHUTDOWN);
}

bool CDasherInterfaceBase::GetModuleSettings(const std::string &strName, SModuleSettings **pSettings, int *iCount) {
  return GetModuleByName(strName)->GetSettings(pSettings, iCount);
}

void CDasherInterfaceBase::SetupActionButtons() {
  m_vLeftButtons.push_back(new CActionButton(this, "Exit", true));
  m_vLeftButtons.push_back(new CActionButton(this, "Preferences", false));
  m_vLeftButtons.push_back(new CActionButton(this, "Help", false));
  m_vLeftButtons.push_back(new CActionButton(this, "About", false));
}

void CDasherInterfaceBase::DestroyActionButtons() {
  // TODO: implement and call this
}

void CDasherInterfaceBase::PositionActionButtons() {
  if(!m_DasherScreen)
    return;

  int iCurrentOffset(16);

  for(std::vector<CActionButton *>::iterator it(m_vLeftButtons.begin()); it != m_vLeftButtons.end(); ++it) {
    (*it)->SetPosition(16, iCurrentOffset, 32, 32);
    iCurrentOffset += 48;
  }

  iCurrentOffset = 16;

  for(std::vector<CActionButton *>::iterator it(m_vRightButtons.begin()); it != m_vRightButtons.end(); ++it) {
    (*it)->SetPosition(m_DasherScreen->GetWidth() - 144, iCurrentOffset, 128, 32);
    iCurrentOffset += 48;
  }
}

bool CDasherInterfaceBase::DrawActionButtons() {
  if(!m_DasherScreen)
    return false;

  bool bVisible(GetBoolParameter(BP_DASHER_PAUSED));

  bool bRV(bVisible != m_bOldVisible);
  m_bOldVisible = bVisible;

  for(std::vector<CActionButton *>::iterator it(m_vLeftButtons.begin()); it != m_vLeftButtons.end(); ++it)
    (*it)->Draw(m_DasherScreen, bVisible);

  for(std::vector<CActionButton *>::iterator it(m_vRightButtons.begin()); it != m_vRightButtons.end(); ++it)
    (*it)->Draw(m_DasherScreen, bVisible);

  return bRV;
}


void CDasherInterfaceBase::HandleClickUp(int iTime, int iX, int iY) {
#ifdef EXPERIMENTAL_FEATURES
  bool bVisible(GetBoolParameter(BP_DASHER_PAUSED));

  for(std::vector<CActionButton *>::iterator it(m_vLeftButtons.begin()); it != m_vLeftButtons.end(); ++it) {
    if((*it)->HandleClickUp(iTime, iX, iY, bVisible))
      return;
  }

  for(std::vector<CActionButton *>::iterator it(m_vRightButtons.begin()); it != m_vRightButtons.end(); ++it) {
    if((*it)->HandleClickUp(iTime, iX, iY, bVisible))
      return;
  }
#endif

  KeyUp(iTime, 100, true, iX, iY);
}

void CDasherInterfaceBase::HandleClickDown(int iTime, int iX, int iY) {
#ifdef EXPERIMENTAL_FEATURES
  bool bVisible(GetBoolParameter(BP_DASHER_PAUSED));

  for(std::vector<CActionButton *>::iterator it(m_vLeftButtons.begin()); it != m_vLeftButtons.end(); ++it) {
    if((*it)->HandleClickDown(iTime, iX, iY, bVisible))
      return;
  }

  for(std::vector<CActionButton *>::iterator it(m_vRightButtons.begin()); it != m_vRightButtons.end(); ++it) {
    if((*it)->HandleClickDown(iTime, iX, iY, bVisible))
      return;
  }
#endif

  KeyDown(iTime, 100, true, iX, iY);
}

void CDasherInterfaceBase::AddActionButton(const std::string &strName) {
  m_vRightButtons.push_back(new CActionButton(this, strName, false));
}


void CDasherInterfaceBase::OnUIRealised() {
  StartTimer();
  ChangeState(TR_UI_INIT);
}


void CDasherInterfaceBase::ChangeState(ETransition iTransition) {
  static EState iTransitionTable[ST_NUM][TR_NUM] = {
    {ST_MODEL, ST_UI, ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN},//ST_START
    {ST_FORBIDDEN, ST_NORMAL, ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN},//ST_MODEL
    {ST_NORMAL, ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN},//ST_UI
    {ST_FORBIDDEN, ST_FORBIDDEN, ST_LOCKED, ST_FORBIDDEN, ST_SHUTDOWN},//ST_NORMAL
    {ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN, ST_NORMAL, ST_FORBIDDEN},//ST_LOCKED
    {ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN, ST_FORBIDDEN}//ST_SHUTDOWN
  //TR_MODEL_INIT, TR_UI_INIT,   TR_LOCK,      TR_UNLOCK,    TR_SHUTDOWN
  };

  EState iNewState(iTransitionTable[m_iCurrentState][iTransition]);

  if(iNewState != ST_FORBIDDEN) {
    if (iNewState == ST_SHUTDOWN) {
      ShutdownTimer();
      WriteTrainFileFull();
    }

    m_iCurrentState = iNewState;
  }
}

void CDasherInterfaceBase::SetOffset(int iOffset, bool bForce) {
  m_pDasherModel->SetOffset(iOffset, m_pNCManager->GetAlphabetManager(), m_pDasherView, bForce);
  //ACL TODO note that CTL_MOVE, etc., do not come here (that would probably
  // rebuild the model / violently repaint the screen every time!). But we
  // still want to notifyOffset all text actions, so the "New" suboption sees
  // all the editing the user's done...

  for (set<TextAction *>::iterator it = m_vTextActions.begin(); it!=m_vTextActions.end(); it++) {
    (*it)->NotifyOffset(iOffset);
  }
}

// Returns 0 on success, an error string on failure.
const char* CDasherInterfaceBase::ClSet(const std::string &strKey, const std::string &strValue) {
  if(m_pSettingsStore)
    return m_pSettingsStore->ClSet(strKey, strValue);
  return 0;
}


void
CDasherInterfaceBase::ImportTrainingText(const std::string &strPath) {
  if(m_pNCManager)
    m_pNCManager->ImportTrainingText(strPath);
}

