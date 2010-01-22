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

#include "EyetrackerFilter.h"
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

CDasherInterfaceBase::CDasherInterfaceBase() {

  // Ensure that pointers to 'owned' objects are set to NULL.
  m_Alphabet = NULL;
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

  // TODO: Are these actually needed?
  strCurrentContext = ". ";
  strTrainfileBuffer = "";

  // Create an event handler.
  m_pEventHandler = new CEventHandler(this);

  m_bLastChanged = true;

#ifndef _WIN32_WCE
  // Global logging object we can use from anywhere
  g_pLogger = new CFileLogger("dasher.log",
                              g_iLogLevel,
                              g_iLogOptions);
#endif

  m_iNextLockID = 1;

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
  ChangeAlphabet(); // This creates the NodeCreationManager, the Alphabet

  // Create the user logging object if we are suppose to.  We wait
  // until now so we have the real value of the parameter and not
  // just the default.

  // TODO: Sort out log type selection

#ifndef _WIN32_WCE
  int iUserLogLevel = GetLongParameter(LP_USER_LOG_LEVEL_MASK);

  if(iUserLogLevel == 10)
    m_pUserLog = new CBasicLog(m_pEventHandler, m_pSettingsStore);
  else if (iUserLogLevel > 0)
    m_pUserLog = new CUserLog(m_pEventHandler, m_pSettingsStore, iUserLogLevel, m_Alphabet);
#else
  m_pUserLog = NULL;
#endif

  CreateModules();

  CreateInput();
  CreateInputFilter();
  SetupActionButtons();
  CParameterNotificationEvent oEvent(LP_NODE_BUDGET);
  InterfaceEventHandler(&oEvent);

  // Set up real orientation to match selection
  if(GetLongParameter(LP_ORIENTATION) == Dasher::Opts::AlphabetDefault)
    SetLongParameter(LP_REAL_ORIENTATION, m_Alphabet->GetOrientation());
  else
    SetLongParameter(LP_REAL_ORIENTATION, GetLongParameter(LP_ORIENTATION));


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
  delete m_Alphabet;
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

    case BP_OUTLINE_MODE:
      ScheduleRedraw();
      break;
    case BP_DRAW_MOUSE:
      ScheduleRedraw();
      break;
    case BP_CONTROL_MODE:
      ScheduleRedraw();
      break;
    case BP_DRAW_MOUSE_LINE:
      ScheduleRedraw();
      break;
    case LP_ORIENTATION:
      if(GetLongParameter(LP_ORIENTATION) == Dasher::Opts::AlphabetDefault)
	// TODO: See comment in DasherModel.cpp about prefered values
	SetLongParameter(LP_REAL_ORIENTATION, m_Alphabet->GetOrientation());
      else
	SetLongParameter(LP_REAL_ORIENTATION, GetLongParameter(LP_ORIENTATION));
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
      strCurrentContext += pEditEvent->m_sText;
      if( strCurrentContext.size() > 20 )
	strCurrentContext = strCurrentContext.substr( strCurrentContext.size() - 20 );
      if(GetBoolParameter(BP_LM_ADAPTIVE))
	 strTrainfileBuffer += pEditEvent->m_sText;
    }
    else if(pEditEvent->m_iEditType == 2) {
      strCurrentContext = strCurrentContext.substr( 0, strCurrentContext.size() - pEditEvent->m_sText.size());
      if(GetBoolParameter(BP_LM_ADAPTIVE))
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
  //Halt Dasher - without a stop event, so does not result in speech etc.
      SetBoolParameter(BP_DASHER_PAUSED, true);

      m_pDasherModel->TriggerSlowdown();
    }
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

void CDasherInterfaceBase::CreateModel(int iOffset) {
  // Creating a model without a node creation manager is a bad plan
  if(!m_pNCManager)
    return;

  if(m_pDasherModel) {
    delete m_pDasherModel;
    m_pDasherModel = 0;
  }

  m_pDasherModel = new CDasherModel(m_pEventHandler, m_pSettingsStore, m_pNCManager, this, m_pDasherView, iOffset);

  // Notify the teacher of the new model
  if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
    pTeacher->SetDasherModel(m_pDasherModel);

}

void CDasherInterfaceBase::CreateNCManager() {
  // TODO: Try and make this work without necessarilty rebuilding the model

  if(!m_AlphIO)
    return;

  int lmID = GetLongParameter(LP_LANGUAGE_MODEL_ID);

  if( lmID == -1 )
    return;

    int iOffset;

    if(m_pDasherModel)
      iOffset = m_pDasherModel->GetOffset();
    else
      iOffset = 0; // TODO: Is this right?

    // Delete the old model and create a new one
    if(m_pDasherModel) {
      delete m_pDasherModel;
      m_pDasherModel = 0;
    }

    if(m_pNCManager) {
      delete m_pNCManager;
      m_pNCManager = 0;
    }

    m_pNCManager = new CNodeCreationManager(this, m_pEventHandler, m_pSettingsStore, m_AlphIO);

    m_Alphabet = m_pNCManager->GetAlphabet();

    // TODO: Eventually we'll not have to pass the NC manager to the model...
    CreateModel(iOffset);
}

void CDasherInterfaceBase::PauseAt(int MouseX, int MouseY) {
  SetBoolParameter(BP_DASHER_PAUSED, true);

  // Request a full redraw at the next time step.
  SetBoolParameter(BP_REDRAW, true);

  Dasher::CStopEvent oEvent;
  m_pEventHandler->InsertEvent(&oEvent);

#ifndef _WIN32_WCE
  if (m_pUserLog != NULL)
    m_pUserLog->StopWriting((float) GetNats());
#endif
}

void CDasherInterfaceBase::GameMessageIn(int message, void* messagedata) {
  GameMode::CDasherGameMode::GetTeacher()->Message(message, messagedata);
}

void CDasherInterfaceBase::Unpause(unsigned long Time) {
  SetBoolParameter(BP_DASHER_PAUSED, false);

  if(m_pDasherModel != 0)
    m_pDasherModel->Reset_framerate(Time);

  Dasher::CStartEvent oEvent;
  m_pEventHandler->InsertEvent(&oEvent);

  // Commenting this out, can't see a good reason to ResetNats,
  // just because we are not paused anymore - pconlon
  // ResetNats();

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
    m_pInput = (CDasherInput *)GetDefaultInputDevice();

  if(m_pInput) {
    m_pInput->Activate();
  }

  if(m_pDasherView != 0)
    m_pDasherView->SetInput(m_pInput);
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

  bool bChanged(false);
  CExpansionPolicy *pol=m_defaultPolicy;
  if(m_pDasherView != 0) {
    if(!GetBoolParameter(BP_TRAINING)) {
      if (m_pUserLog != NULL) {
	//ACL note that as of 15/5/09, splitting UpdatePosition into two,
	//DasherModel no longer guarantees to empty these two if it didn't do anything.
	//So initialise appropriately...
	Dasher::VECTOR_SYMBOL_PROB vAdded;
	int iNumDeleted = 0;

	if(m_pInputFilter) {
	  bChanged = m_pInputFilter->Timer(iTime, m_pDasherView, m_pDasherModel, &vAdded, &iNumDeleted, &pol);
	}

#ifndef _WIN32_WCE
	if (iNumDeleted > 0)
	  m_pUserLog->DeleteSymbols(iNumDeleted);
	if (vAdded.size() > 0)
	  m_pUserLog->AddSymbols(&vAdded);
#endif

      }
      else {
	if(m_pInputFilter) {
	  bChanged = m_pInputFilter->Timer(iTime, m_pDasherView, m_pDasherModel, 0, 0, &pol);
	}
      }

      m_pDasherModel->CheckForNewRoot(m_pDasherView);
    }
  }

  // Flags at this stage:
  //
  // - bChanged = the display was updated, so needs to be rendered to the display
  // - m_bLastChanged = bChanged was true last time around
  // - m_bRedrawScheduled = Display invalidated internally
  // - bForceRedraw = Display invalidated externally

  // TODO: This is a bit hacky - we really need to sort out the redraw logic
  if((!bChanged && m_bLastChanged) || m_bRedrawScheduled || bForceRedraw) {
    m_pDasherView->Screen()->SetCaptureBackground(true);
    m_pDasherView->Screen()->SetLoadBackground(true);
  }

  bForceRedraw |= m_bLastChanged;
  m_bLastChanged = bChanged; //will also be set in Redraw if any nodes were expanded.

  Redraw(bChanged || m_bRedrawScheduled || bForceRedraw, *pol);

  m_bRedrawScheduled = false;

  // This just passes the time through to the framerate tracker, so we
  // know how often new frames are being drawn.
  if(m_pDasherModel != 0)
    m_pDasherModel->NewFrame(iTime);

  bReentered=false;
}

void CDasherInterfaceBase::Redraw(bool bRedrawNodes, CExpansionPolicy &policy) {
  // No point continuing if there's nothing to draw on...
  if(!m_pDasherView)
    return;

  // Draw the nodes
  if(bRedrawNodes) {
    m_pDasherView->Screen()->SendMarker(0);
	if (m_pDasherModel) m_bLastChanged |= m_pDasherModel->RenderToView(m_pDasherView,policy);
  }

  // Draw the decorations
  m_pDasherView->Screen()->SendMarker(1);

  if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
    pTeacher->DrawGameDecorations(m_pDasherView);

  bool bDecorationsChanged(false);

  if(m_pInputFilter) {
    bDecorationsChanged = m_pInputFilter->DecorateView(m_pDasherView);
  }

  bool bActionButtonsChanged(false);
#ifdef EXPERIMENTAL_FEATURES
  bActionButtonsChanged = DrawActionButtons();
#endif

  // Only blit the image to the display if something has actually changed
  if(bRedrawNodes || bDecorationsChanged || bActionButtonsChanged)
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

  CreateNCManager();

#ifndef _WIN32_WCE
  // Let our user log object know about the new alphabet since
  // it needs to convert symbols into text for the log file.
  if (m_pUserLog != NULL)
    m_pUserLog->SetAlphabetPtr(m_Alphabet);
#endif

  // Apply options from alphabet

  SetBoolParameter( BP_TRAINING, false );

  //}
}

void CDasherInterfaceBase::ChangeColours() {
  if(!m_ColourIO || !m_DasherScreen)
    return;

  // TODO: Make fuction return a pointer directly
  m_DasherScreen->SetColourScheme(&(m_ColourIO->GetInfo(GetStringParameter(SP_COLOUR_ID))));
}

void CDasherInterfaceBase::ChangeScreen(CDasherScreen *NewScreen) {
  // What does ChangeScreen do?
  m_DasherScreen = NewScreen;
  ChangeColours();

  if(m_pDasherView != 0) {
    m_pDasherView->ChangeScreen(m_DasherScreen);
  } else if(GetLongParameter(LP_VIEW_ID) != -1) {
    ChangeView();
  }

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

    if (m_pInput)
      m_pDasherView->SetInput(m_pInput);

    // Tell the Teacher which view we are using
    if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
	pTeacher->SetDasherView(m_pDasherView);
  }
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


// TODO: Check that none of this needs to be reimplemented

// void CDasherInterfaceBase::InvalidateContext(bool bForceStart) {
//   m_pDasherModel->m_strContextBuffer = "";

//   Dasher::CEditContextEvent oEvent(10);
//   m_pEventHandler->InsertEvent(&oEvent);

//    std::string strNewContext(m_pDasherModel->m_strContextBuffer);

//   // We keep track of an internal context and compare that to what
//   // we are given - don't restart Dasher if nothing has changed.
//   // This should really be integrated with DasherModel, which
//   // probably will be the case when we start to deal with being able
//   // to back off indefinitely. For now though we'll keep it in a
//   // separate string.

//    int iContextLength( 6 ); // The 'important' context length - should really get from language model

//    // FIXME - use unicode lengths

//    if(bForceStart || (strNewContext.substr( std::max(static_cast<int>(strNewContext.size()) - iContextLength, 0)) != strCurrentContext.substr( std::max(static_cast<int>(strCurrentContext.size()) - iContextLength, 0)))) {

//      if(m_pDasherModel != NULL) {
//        // TODO: Reimplement this
//        //       if(m_pDasherModel->m_bContextSensitive || bForceStart) {
//        {
//  	m_pDasherModel->SetContext(strNewContext);
//  	PauseAt(0,0);
//        }
//      }

//      strCurrentContext = strNewContext;
//      WriteTrainFileFull();
//    }

//    if(bForceStart) {
//      int iMinWidth;

//      if(m_pInputFilter && m_pInputFilter->GetMinWidth(iMinWidth)) {
//        m_pDasherModel->LimitRoot(iMinWidth);
//      }
//    }

//    if(m_pDasherView)
//      while( m_pDasherModel->CheckForNewRoot(m_pDasherView) ) {
//        // Do nothing
//      }

//    ScheduleRedraw();
// }

// TODO: Fix this

std::string CDasherInterfaceBase::GetContext(int iStart, int iLength) {
  m_strContext = "";

  CEditContextEvent oEvent(iStart, iLength);
  m_pEventHandler->InsertEvent(&oEvent);

  return m_strContext;
}

void CDasherInterfaceBase::SetContext(std::string strNewContext) {
  m_strContext = strNewContext;
}

// Control mode stuff

void CDasherInterfaceBase::RegisterNode( int iID, const std::string &strLabel, int iColour ) {
  m_pNCManager->RegisterNode(iID, strLabel, iColour);
}

void CDasherInterfaceBase::ConnectNode(int iChild, int iParent, int iAfter) {
  m_pNCManager->ConnectNode(iChild, iParent, iAfter);
}

void CDasherInterfaceBase::DisconnectNode(int iChild, int iParent) {
  m_pNCManager->DisconnectNode(iChild, iParent);
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
CUserLogBase* CDasherInterfaceBase::GetUserLogPtr() {
  return m_pUserLog;
}

void CDasherInterfaceBase::KeyDown(int iTime, int iId, bool bPos, int iX, int iY) {
  if(m_iCurrentState != ST_NORMAL)
    return;

  if(m_pInputFilter && !GetBoolParameter(BP_TRAINING)) {
    m_pInputFilter->KeyDown(iTime, iId, m_pDasherView, m_pDasherModel, m_pUserLog, bPos, iX, iY);
  }

  if(m_pInput && !GetBoolParameter(BP_TRAINING)) {
    m_pInput->KeyDown(iTime, iId);
  }
}

void CDasherInterfaceBase::KeyUp(int iTime, int iId, bool bPos, int iX, int iY) {
  if(m_iCurrentState != ST_NORMAL)
    return;

  if(m_pInputFilter && !GetBoolParameter(BP_TRAINING)) {
    m_pInputFilter->KeyUp(iTime, iId, m_pDasherView, m_pDasherModel, bPos, iX, iY);
  }

  if(m_pInput && !GetBoolParameter(BP_TRAINING)) {
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
    m_pInputFilter = (CInputFilter *)GetDefaultInputMethod();

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

CDasherModule *CDasherInterfaceBase::GetDefaultInputDevice() {
    return m_oModuleManager.GetDefaultInputDevice();
}

CDasherModule *CDasherInterfaceBase::GetDefaultInputMethod() {
    return m_oModuleManager.GetDefaultInputMethod();
}

void CDasherInterfaceBase::SetDefaultInputDevice(CDasherModule *pModule) {
    m_oModuleManager.SetDefaultInputDevice(pModule);
}

void CDasherInterfaceBase::SetDefaultInputMethod(CDasherModule *pModule) {
    m_oModuleManager.SetDefaultInputMethod(pModule);
}

void CDasherInterfaceBase::CreateModules() {
  SetDefaultInputMethod(
    RegisterModule(new CDefaultFilter(m_pEventHandler, m_pSettingsStore, this, 3, _("Normal Control")))
  );
  RegisterModule(new COneDimensionalFilter(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CEyetrackerFilter(m_pEventHandler, m_pSettingsStore, this));
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


void CDasherInterfaceBase::ExecuteCommand(const std::string &strName) {
  // TODO: Pointless - just insert event directly

  CCommandEvent *pEvent = new CCommandEvent(strName);
  m_pEventHandler->InsertEvent(pEvent);
  delete pEvent;
}

double CDasherInterfaceBase::GetFramerate() {
  if(m_pDasherModel)
    return(m_pDasherModel->Framerate());
  else
    return 0.0;
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

void CDasherInterfaceBase::SetBuffer(int iOffset) {
   CreateModel(iOffset);
}

void CDasherInterfaceBase::UnsetBuffer() {
  // TODO: Write training file?
  if(m_pDasherModel)
    delete m_pDasherModel;

  m_pDasherModel = 0;
}

void CDasherInterfaceBase::SetOffset(int iOffset) {
  if(m_pDasherModel)
    m_pDasherModel->SetOffset(iOffset, m_pDasherView);
}

void CDasherInterfaceBase::SetControlOffset(int iOffset) {
  if(m_pDasherModel)
    m_pDasherModel->SetControlOffset(iOffset);
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

