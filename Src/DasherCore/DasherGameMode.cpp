
#include "../Common/Common.h"

#include "DasherGameMode.h"
#include "DasherInterfaceBase.h"
#include "DasherModel.h"
#include "DasherNode.h"
#include "DasherView.h"
#include "Event.h"

#include "GameScorer.h"
#include "GameLevel.h"
#include "GameMessages.h"

#include <limits>
#include <iostream>
#include <fstream>

using Dasher::GameMode::CDasherGameMode;
using Dasher::GameMode::UTF8Char;

class Dasher::CEventHandler;
class CSettingsStore;

std::pair<double,double> GaussianRand();

CDasherGameMode* CDasherGameMode::pTeacher = NULL;

CDasherGameMode* CDasherGameMode::CreateTeacher(Dasher::CEventHandler *pEventHandler,
						CSettingsStore *pSettingsStore,
						Dasher::CDasherInterfaceBase *pDashIface)
{
  delete pTeacher; //Initialised to NULL, so this line is safe.
  pTeacher = new CDasherGameMode(pEventHandler, pSettingsStore, pDashIface);
  return pTeacher;
}

void CDasherGameMode::DestroyTeacher()
{
  delete pTeacher;
  pTeacher = NULL;
}

CDasherGameMode::CDasherGameMode(Dasher::CEventHandler *pEventHandler,
				 CSettingsStore *pSettingsStore,
				 Dasher::CDasherInterfaceBase *pDashIface)
  :CDasherComponent(pEventHandler, pSettingsStore),
   m_pDasherInterface(pDashIface),
   m_pView(NULL), m_pModel(NULL),
   m_pScorer(NULL), m_pDemo(NULL),
   m_iCrossX((myint)GetLongParameter(LP_OX)),
   m_iCrossY((myint)GetLongParameter(LP_OY)),
   m_iMaxY((myint)GetLongParameter(LP_MAX_Y))

{
  // Create the GameMode object in the background
  // m_bGameModeOn tells us whether we should be alive or not.
  // we listen for parameter change events, and wake up if necessary.

  m_bGameModeOn = GetBoolParameter(BP_GAME_MODE);
  m_strGameTextFile = GetStringParameter(SP_GAME_TEXT_FILE);

  m_bDrawHelperArrow = false;
  m_bDrawTargetArrow = false;

  m_bDrawPoints = false;

  // We have not initialised our target strings yet
  m_iCurrentString = -1;
  m_iNumStrings = 0;
  m_bSentenceFinished = true;

  // Start the oscillator, intially flipping every second.
  m_ulTime = 0;
  m_iOscillatorOn = 1000;
  m_iOscillatorOff = 1000;
  m_bOscillator = false;
  Oscillator();
}

CDasherGameMode::~CDasherGameMode()
{
  delete m_pScorer;
}

void CDasherGameMode::Message(int message, void* messagedata)
{
  switch(message){
  case GAME_MESSAGE_NEXT:
    GameNext();
    break;
  case GAME_MESSAGE_DEMO_OFF:
    DemoModeStop();
    break;
  case GAME_MESSAGE_DEMO_ON:
    DemoModeStart(false);
    // Start in 2 seconds
    Callback(&CDasherGameMode::DemoGo,2000);
    break;
  case GAME_MESSAGE_FULL_DEMO:
    // Start internally...
    InitializeTargets();
    DemoModeStart(true);
    // ... then externally
    NotifyGameCooperators(true);
    
    FullDemoNext();
    break;
  case GAME_MESSAGE_GAME_ON:
    m_bGameModeOn = true;
    // Get going internally first...
    GameModeStart();
    // ...then make the links to cooperators
    NotifyGameCooperators(m_bGameModeOn);
    break;
    
  case GAME_MESSAGE_GAME_OFF:
    m_bGameModeOn = false;
    // Shutdown layer by layer
    DemoModeStop();
    // Break links to cooperators first...
    NotifyGameCooperators(m_bGameModeOn);
    // ... then stop ourselves.
    GameModeStop();
    break;
  }

      
}

void CDasherGameMode::NotifyGameCooperators(bool bGameOn)
{
  SetBoolParameter(BP_GAME_MODE, bGameOn);
  m_pView->SetGameMode(bGameOn);
  m_pDasherInterface->SetContext(std::string(""));
  m_pDasherInterface->SetBuffer(0);
  m_pDasherInterface->PauseAt(0,0);

}

// This routine is used when the interactive game is turned on
void CDasherGameMode::GameModeStart()
{
  string output = "Click \"Next\" to begin";
  m_strGameTextFile = GetStringParameter(SP_GAME_TEXT_FILE);
  InitializeTargets();
  m_pDasherInterface->GameMessageOut(GAME_MESSAGE_DISPLAY_TEXT,
				     reinterpret_cast<const void*>(&output));
  m_bSentenceFinished = true;

  // Create a new scorer to measure player stats and the like
  if(!m_pScorer)
    m_pScorer = new Scorer;
  m_pLevel = new LevelStart(this);

  output = m_pLevel->GetLevel();
  m_pDasherInterface->GameMessageOut(GAME_MESSAGE_SET_LEVEL,
    reinterpret_cast<const void*>(&output));
}

// This routine is used when the interactive game is turned off
void CDasherGameMode::GameModeStop()
{
  delete m_pScorer;
  m_pScorer = NULL;
  delete m_pLevel;
  m_pLevel = NULL;
}

void CDasherGameMode::DemoModeStart(bool bFullDemo)
{
  // Start up internal first...
  if(!m_pDemo)
    m_pDemo = new Demo(m_pSettingsStore, bFullDemo);

  m_iDemoX = m_iUserX;
  m_iDemoY = m_iUserY;
  LoadDemoConfigFile();

  // ...then do external  
  m_pView->SetDemoMode(true);
}

void CDasherGameMode::DemoModeStop()
{
  // Close down external first...
  m_pDasherInterface->PauseAt(0,0);
  m_pView->SetDemoMode(false);

  // ... then internal.
  delete m_pDemo;
  m_pDemo=NULL;
}


void CDasherGameMode::HandleEvent(Dasher::CEvent * pEvent) 
{
  // If we are not active, return
  if(!m_bGameModeOn && !m_pDemo)
    {
      //      if(pEvent->m_iEventType == EV_PARAM_NOTIFY)
      //	{
      //	  if(static_cast<CParameterNotificationEvent*>(pEvent)->m_iParameter == BP_GAME_MODE)
      //	    {
      //	      m_bGameModeOn = GetBoolParameter(BP_GAME_MODE);
      //	      NotifyGameCooperators(m_bGameModeOn);
      //	      GameModeStart();
      //	    }
      //	}
      return;
    }
  else
    {
      string output;

      // Otherwise listen for all events and deal with them appropriately
      switch(pEvent->m_iEventType)
	{
	case EV_PARAM_NOTIFY:

	  switch(static_cast<CParameterNotificationEvent*>(pEvent)->m_iParameter)
	    {
	    case BP_GAME_MODE:
	      //	      m_bGameModeOn = GetBoolParameter(BP_GAME_MODE);
	      //	      if(!m_bGameModeOn) // i.e, if we have just been turned off
	      //		GameModeStop();
	      //	      NotifyGameCooperators(m_bGameModeOn);
	      break;	  
	    case SP_GAME_TEXT_FILE:
              output = "Welcome to Dasher Game Mode!";
	      m_strGameTextFile = GetStringParameter(SP_GAME_TEXT_FILE);
	      std::cout << "Change of game file to " << m_strGameTextFile << std::endl;
     	      InitializeTargets();
	      m_pDasherInterface->GameMessageOut(GAME_MESSAGE_DISPLAY_TEXT,
						 reinterpret_cast<const void*>(&output));
	      m_bSentenceFinished = true;
	      break;
	    case LP_MAX_BITRATE: 
	      CalculateDemoParameters();
	      break;
	    }
	  break;
	case EV_STOP:
	  if(m_pScorer)
	    m_pScorer->Stop();
	  break;
	case EV_START:
	  if(m_pDemo)
	    CalculateDemoParameters();
	  if(m_pScorer)
	    m_pScorer->Start();
	  break;
	case EV_EDIT:
	  if(m_bSentenceFinished) break;
	  CEditEvent *pEditEvent(static_cast < CEditEvent * >(pEvent));
	  
	  if(pEditEvent->m_iEditType == 1) {
	    m_pDasherInterface->GameMessageOut(GAME_MESSAGE_EDIT,
					       reinterpret_cast<const void*>(&(pEditEvent->m_sText)));
	  }
	  else if(pEditEvent->m_iEditType == 2) {
	    int numDeleted = pEditEvent->m_sText.size();
	    m_pDasherInterface->GameMessageOut(GAME_MESSAGE_EDIT_DELETE,
					       reinterpret_cast<const void*>(&numDeleted));
	  }
	  break;
	}
      return;
    }
}

void CDasherGameMode::GameNext()
{
  if(m_pLevel->IsCompleted())
  {
    string output = m_pLevel->GetLevel();
    m_pLevel = m_pLevel->GetNextLevel();
    m_pDasherInterface->GameMessageOut(GAME_MESSAGE_SET_LEVEL,
				     reinterpret_cast<const void *>(&output));
    output = m_pLevel->GetRules();
    m_pDasherInterface->GameMessageOut(GAME_MESSAGE_HELP_MESSAGE, &output);
  }
  
  // Choose next string (NOT) at random...
  NextString(false);//true);
  
  // ...display it to the user...
  const std::string * pStr = &m_strCurrentTarget;
  m_pDasherInterface->GameMessageOut(GAME_MESSAGE_SET_TARGET_STRING, reinterpret_cast<const void *>(pStr));
  
  // ...reset our internal state...
  m_bDrawPoints=false;
  m_pScorer->Reset();
  m_pLevel->Reset();
  m_bSentenceFinished = false;

  // ...then reset the external state, and leave them at the start ready to go.
  m_pDasherInterface->GameMessageOut(GAME_MESSAGE_CLEAR_BUFFER, NULL);
  m_pDasherInterface->SetContext(std::string(""));
  m_pDasherInterface->SetBuffer(0);
  m_pDasherInterface->PauseAt(0,0);
 
  RunningScoreUpdates();
}

void CDasherGameMode::FullDemoNext()
{
  if(!m_pDemo) return;
  // Choose the alphabet, by alternating between home alphabet and foreign.
  if(GetStringParameter(SP_ALPHABET_ID) == m_pDemo->sp_alphabet_id)
    {
      // Find a foreign alphabet with some sentences to write
      std::vector<std::string> vAlphabets;
      m_pDasherInterface->GetPermittedValues(SP_ALPHABET_ID, vAlphabets);
      
      do{ 
	while(GetBoolParameter(BP_TRAINING)) {}
	int randomAlph = rand() % vAlphabets.size();
	std::cout << "Setting: " << vAlphabets[randomAlph] << std::endl;
	SetStringParameter(SP_ALPHABET_ID, vAlphabets[randomAlph]);
	std::cout << "    Set: " << GetStringParameter(SP_ALPHABET_ID) << std::endl;
	std::cout << "    Game:" << m_strGameTextFile << std::endl;
      } while(NextString(true) != GM_ERR_NO_ERROR);
    }
  else
    {
      SetStringParameter(SP_ALPHABET_ID, m_pDemo->sp_alphabet_id);
      NextString(true);
    }
  
  while(GetBoolParameter(BP_TRAINING)) {}

  m_bSentenceFinished = false;

  // ...then reset the external state, and leave the demo at the start ready to go.
  m_pDasherInterface->GameMessageOut(GAME_MESSAGE_CLEAR_BUFFER, NULL);
  m_pDasherInterface->SetContext(std::string(""));
  m_pDasherInterface->SetBuffer(0);
  m_pDasherInterface->PauseAt(0,0);

  // We start in 3 seconds.
  Callback(&CDasherGameMode::DemoGo,3000);
}

void CDasherGameMode::DemoGo()
{
  if(!m_pDemo) return;

  CalculateDemoParameters();
  m_pDasherInterface->Unpause(m_ulTime);
}

void CDasherGameMode::NewFrame(unsigned long Time)
{
  // NewFrame acts as a main loop for DasherGameMode.

  // There is a risk that we end up in here recursively, thus corrupting the Callback's list.
  // thus we require this check.
  static int depth = 0;
  if(depth==1) return;
  ++depth;

  // Set our internal clock
  m_ulTime = Time;

  // Pass on relevant game information to the scorer, and execute help rules according to level
  if(!m_bSentenceFinished)
    {
      if(m_pScorer)
	{
	  m_pScorer->NewFrame(Scorer::GameInfo(Time, m_iUserX, m_iUserY, m_Target.iTargetY, m_pModel->GetNats()));
	  m_pLevel->DoGameLogic();
	}
    }

  // Timed callbacks, for demo mode and oscillators, and the like
  std::vector<CallbackList::iterator> vDeleteList;
  for(CallbackList::iterator it = m_lCallbacks.begin();
      it != m_lCallbacks.end(); ++it)
    {
      if((*it).first <= Time)
	{
	  mem_fun((*it).second)(this);
	  vDeleteList.push_back(it);
	}
    }
  for(std::vector<CallbackList::iterator>::iterator it = vDeleteList.begin();
      it != vDeleteList.end(); ++it)
    {
      m_lCallbacks.erase(*it);
    }
  
  // Functor callbacks, for utility things like DelaySet.
  std::vector<FunctorCallbackList::iterator> vFunctorDeleteList;
  for(FunctorCallbackList::iterator it = m_lFunctorCallbacks.begin();
      it != m_lFunctorCallbacks.end(); ++it)
    {
      if((*it).first <= Time)
	{
	  ((*it).second)->Callback();
	  vFunctorDeleteList.push_back(it);
	}
    }
  for(std::vector<FunctorCallbackList::iterator>::iterator it = vFunctorDeleteList.begin();
      it != vFunctorDeleteList.end(); ++it)
    {
      m_lFunctorCallbacks.erase(*it);
    }
  
  --depth;  
}


void CDasherGameMode::SetTargetY(const std::vector<std::pair<Dasher::myint,bool> >& vTargetY)
{
  // Called by the Node Rendering Routine to let us know where the target sentence is
  // vTargetY is a vector of the centerpoints of all drawn targets.
  if(vTargetY.size()!=0)
    {
      // Save the real target...
      m_Target.iTargetY = vTargetY.back().first;
      m_Target.iCenterY = ComputeBrachCenter(m_Target.iTargetY, m_iCrossX, m_iCrossY);

      // ...and find the smallest visible square too
      for(std::vector<std::pair<myint,bool> >::const_reverse_iterator i=vTargetY.rbegin();
	  i!=vTargetY.rend(); ++i)
	{
	  if((*i).second == true)
	    {
	      m_Target.iVisibleTargetY = (*i).first;
	      m_Target.iVisibleCenterY = ComputeBrachCenter(m_Target.iVisibleTargetY,m_iCrossX, m_iCrossY);
	      break;
	    }
	}
    }
} 

void CDasherGameMode::CalculateDemoParameters()
{
  // Recalculates the parameters used in the demo following a change in framerate or speed.
  double spring = GetLongParameter(LP_DEMO_SPRING)/100.0;
  double noisemem = GetLongParameter(LP_DEMO_NOISE_MEM)/100.0;
  double lambda = 0.7*double(GetLongParameter(LP_MAX_BITRATE))/(100.0*m_pModel->Framerate());
  
  if(m_pModel->Framerate() <= 0) return;

  m_DemoCfg.dSpring = (1-exp(-spring*lambda));
  m_DemoCfg.dNoiseNew = noisemem*(1-exp(-lambda));
  m_DemoCfg.dNoiseOld = sqrt(1.0-m_DemoCfg.dNoiseNew*m_DemoCfg.dNoiseNew);
}

void CDasherGameMode::LoadDemoConfigFile()
{
  m_DemoCfg.iNoiseMag = GetLongParameter(LP_DEMO_NOISE_MAG);
  CalculateDemoParameters();
}

int CDasherGameMode::InitializeTargets()
{
  // Reset all members associated with the target...
  m_vTargetStrings.clear();
  m_iCurrentString = -1; // -1 to indicate no string available (yet)
  m_iNumStrings = 0;
  std::string strGameFileName;
  
  // ...then check that there is a game text file...
  if(m_strGameTextFile == "")
    return GM_ERR_NO_GAME_FILE;
      
  // ...and open it.
  strGameFileName = GetStringParameter(SP_SYSTEM_LOC) + m_strGameTextFile;
  ifstream GameFile(strGameFileName.c_str());
  if(!GameFile)
    return GM_ERR_BAD_GAME_FILE;
  
  // Once the file has been successfully opened, read in the
  // UTF-8 strings.
  LoadTargetStrings(GameFile);
  m_iNumStrings = m_vTargetStrings.size();
  
  if(m_iNumStrings == 0)
    return GM_ERR_NO_STRING;
  else
    return GM_ERR_NO_ERROR;
}

int CDasherGameMode::LoadTargetStrings(istream& in)
{
  std::vector<std::string > vTargetStrings;// All the different strings from the game file

  // A target string is (for now) any string > 4 chars delimited by '#' on either side. 
  const char delimiter = '#'; // Could come from some sort of config file?

  // Load each string from the file...
  while(!in.eof())
    {
      std::string strOneTarget;
      getline(in, strOneTarget, delimiter);
      if(strOneTarget.length()>=4)
	    vTargetStrings.push_back(strOneTarget);
    }
  
  // ...then split them into separate UTF-8 characters

  for(std::vector<UTF8Char>::const_iterator it = vTargetStrings.begin();
      it != vTargetStrings.end(); it++)
    {
      std::vector<UTF8Char> vUtf8TargetString; 

      for(unsigned int i = 0; i != (*it).length(); ++i)
	{
	  UTF8Char strUtf8Char = std::string(1,(*it)[i]);
	  if((*it)[i] & 0x80)
	    {    // Character is more than 1 byte long
	      int extras = 1;
	      for(unsigned int bit = 0x20; ((*it)[i] & bit) != 0; bit >>= 1)
		extras++;
	      
	      if(extras > 5) {}  // Malformed character
	      while(extras-- > 0) 
		{
		  strUtf8Char += (*it)[++i];
		}
	    }
	  vUtf8TargetString.push_back(strUtf8Char);
	}
      m_vTargetStrings.push_back(vUtf8TargetString);
    }
  return GM_ERR_NO_ERROR;
}

// In Demo mode, DasherView will use this function to serve up coordinates to the input filter.
// Make demo mode like as an input device?

void CDasherGameMode::DemoModeGetCoordinates(Dasher::myint& iDasherX, Dasher::myint& iDasherY)
{
  static bool bNavError, bFixingNavError=false;
  // First choose an appropriate target...
  myint iTargetY=(m_Target.iVisibleTargetY);

  if(m_Target.iTargetY>10000 || m_Target.iTargetY <-6000)
      bNavError=true;

  if(bNavError)
    {
      iTargetY=m_Target.iTargetY;
      if(!bFixingNavError &&
	 abs((int)(iTargetY-m_iCrossY)) < 300 &&
		abs(int(iTargetY - m_Target.iVisibleTargetY))<10 )
	{
	  // Now we seem on the right track, give ourselves one second to get sorted.
	  new DelaySet(this, 1000, &bNavError, false);
	  new DelaySet(this, 1000, &bFixingNavError, false);
	  bFixingNavError = true;
	}
    }

  // ...and now calculate the ideal direction...
  double iIdealUnitVec[2];  
  
  myint iCenterY = ComputeBrachCenter(iTargetY, m_iCrossX, m_iCrossY);

  iIdealUnitVec[0] = double(m_iCrossY<iTargetY?(iCenterY-m_iCrossY):(m_iCrossY-iCenterY));
  iIdealUnitVec[1] = double(m_iCrossY<iTargetY?m_iCrossX:-m_iCrossX);
  double mag = sqrt((double)(iIdealUnitVec[0]*iIdealUnitVec[0]+iIdealUnitVec[1]*iIdealUnitVec[1]));
  iIdealUnitVec[0] = iIdealUnitVec[0]/mag;
  iIdealUnitVec[1] = iIdealUnitVec[1]/mag;

  // ...and then modify for realism... 
  // ...by adding noise...
  std::pair<double,double> noise = GaussianRand();
  static double noiseX=0.0;
  static double noiseY=0.0;
  noiseX = m_DemoCfg.dNoiseOld*noiseX + m_DemoCfg.dNoiseNew*noise.first;
  noiseY = m_DemoCfg.dNoiseOld*noiseY + m_DemoCfg.dNoiseNew*noise.second;

  // ...and springy behaviour...
  if(!m_bSentenceFinished)
    {
      //      CalculateNewDemoCoordinates()
      m_iDemoX = myint((m_iCrossX+(1500*iIdealUnitVec[0])+m_DemoCfg.iNoiseMag*noiseX)*m_DemoCfg.dSpring
	+(1.0-m_DemoCfg.dSpring)*m_iDemoX);
      m_iDemoY = myint((m_iCrossY+(1500*iIdealUnitVec[1])+m_DemoCfg.iNoiseMag*noiseY)*m_DemoCfg.dSpring
	+(1.0-m_DemoCfg.dSpring)*m_iDemoY);
    }
  else
    {
      m_iDemoX = myint(m_iCrossX*m_DemoCfg.dSpring + (1.0-m_DemoCfg.dSpring)*m_iDemoX);
      m_iDemoY = myint(m_iCrossY*m_DemoCfg.dSpring + (1.0-m_DemoCfg.dSpring)*m_iDemoY);
    }

  // ...and finally set the mouse coordinates.
  iDasherX=m_iDemoX;
  iDasherY=m_iDemoY;
}

// In Gamemode, DasherView will helpfully call this function whenever it provides
// coordinates to the inputfilters - ie, we can track the mouse from this
// Mouse coordinates are passed onto the scorer every NewFrame, in case it wants to reward steady
// hand position, or something similar.
void CDasherGameMode::SetUserMouseCoordinates(Dasher::myint iDasherX, Dasher::myint iDasherY)
{
  m_iUserX = iDasherX;
  m_iUserY = iDasherY;
}

// Used to turn on running updates of the score, every second.
void CDasherGameMode::RunningScoreUpdates()
{
  ScoreUpdate();

  if(!m_bSentenceFinished)
    Callback(&CDasherGameMode::RunningScoreUpdates, 1000);
}

void CDasherGameMode::ScoreUpdate()
{
  stringstream score;
  score << m_pLevel->GetCurrentScore();
  std::string strScore = score.str();
  m_pDasherInterface->GameMessageOut(GAME_MESSAGE_SET_SCORE,
				     reinterpret_cast<const void *>(&strScore));
}

void CDasherGameMode::SentenceFinished()
{
  if(m_bSentenceFinished) return;
  // We delay processing until the NewFrame function is called
  Callback(&CDasherGameMode::PrivateSentenceFinished,0);
}

void CDasherGameMode::PrivateSentenceFinished()
{
  if(m_pScorer)
    {
      string output = _("Well done!");
      ScoreUpdate();
      m_pScorer->SentenceFinished();
      m_pLevel->SentenceFinished();
      m_pDasherInterface->GameMessageOut(GAME_MESSAGE_DISPLAY_TEXT,
					 reinterpret_cast<const void*>(&output));
    }
  
  if(m_pDemo)
    {
      if(m_pDemo->bFullDemo)
        Callback(&CDasherGameMode::FullDemoNext, 6000);
      else
      {
	      Callback(&CDasherGameMode::GameNext, 6000);
	      Callback(&CDasherGameMode::DemoGo, 9000);
	    }
    }
  m_bDrawHelperArrow=false;
  m_bDrawTargetArrow=false;
  m_bSentenceFinished=true;
  m_pDasherInterface->PauseAt(0,0);
  if(!m_pDemo) {
    string msg = m_pLevel->m_strPerformance.str();
    m_pDasherInterface->GameMessageOut(GAME_MESSAGE_HELP_MESSAGE, &msg);
    ScoreUpdate();
    if(m_pLevel->IsCompleted())
    {
      msg = "You now progress to the next level!";
      m_pDasherInterface->GameMessageOut(GAME_MESSAGE_HELP_MESSAGE, &msg);
    }
  }
}


UTF8Char CDasherGameMode::GetSymbolAtOffset(unsigned int iOffset)
{ 
  // The string "GameEnd" indicates the NF_END_GAME flag should be set on the node.
  if(m_iCurrentString == -1)
    return std::string("GameEnd");
  
  if(m_iNumStrings != m_vTargetStrings.size())
    std::cout << "Error here!" << std::endl;

  if(iOffset < m_vTargetStrings[m_iCurrentString].size())
    return m_vTargetStrings[m_iCurrentString][iOffset];
  else
    return std::string("GameEnd");
}

int CDasherGameMode::NextString(bool bRandomString)
{
  m_strCurrentTarget = "";

  int ret=GM_ERR_NO_ERROR;
  if(m_iNumStrings == 0)
    {
      m_iCurrentString = -1;
      return GM_ERR_NO_STRING;
    }
  else if(bRandomString)
    {
      m_iCurrentString = rand() % m_iNumStrings;
    }
  else if(m_iCurrentString<int(m_iNumStrings)-1)
    m_iCurrentString++;
  else
    {
      m_iCurrentString=0; // Loop back to beginning
      ret=GM_ERR_LOOP;
    }

  for(size_t i = 0; i != m_vTargetStrings[m_iCurrentString].size(); ++i)
    m_strCurrentTarget += m_vTargetStrings[m_iCurrentString][i];

  return ret;
}

void CDasherGameMode::Oscillator()
{
  // Provides an oscillator for general use, eg flashing arrows.
  // The oscillator is true for m_iOscillatorOn ms, and false
  // for m_iOscillatorOff ms.
  if(m_bOscillator)
    {
      m_bOscillator = false;
      Callback(&CDasherGameMode::Oscillator,m_iOscillatorOff);
    }
  else
    {
      m_bOscillator = true;
      Callback(&CDasherGameMode::Oscillator,m_iOscillatorOn);
    }
}

// Drawing Methods for Game/Teacher Mode

// Public function, called by DasherInterfaceBase::Redraw()
void CDasherGameMode::DrawGameDecorations(Dasher::CDasherView* pView)
{
  if(!m_bGameModeOn) return;

  if(m_bDrawHelperArrow)
    DrawHelperArrow(pView);
  if(m_bDrawTargetArrow)
    DrawTargetArrow(pView);
  /*
  if(m_bDrawPoints)
    DrawPoints(pView);*/
}
/*
void CDasherGameMode::DrawPoints(CDasherView* pView)
{
  if(m_pScorer)
    pView->DrawText(m_pScorer->GetBreakdown(),16000,1000, 20);
}
*/

void CDasherGameMode::DrawHelperArrow(Dasher::CDasherView* pView)
{
 // This plots a helpful pointer to the best direction to take to get to the target.
  // Probably too much floating point maths here, sort later.
 // Start of line is the crosshair location
  const int gameColour = 135; //Neon green. (!)
  const int noOfPoints = 10; // The curve will be made up of 9 straight segments...

  myint iX[noOfPoints];
  myint iY[noOfPoints];
  myint iLength;

  // Arrow starts at the cross hairs
  iX[0] = m_iCrossX;
  iY[0] = m_iCrossY;

  myint a = m_iCrossX/5;
  myint defaultlength = m_iCrossX - a ; 

  // ... then decide the length of the arrow...
  myint r = m_Target.iTargetY-m_Target.iCenterY; // radius of our circle (+ or -)

  if(m_Target.iTargetY < a && m_Target.iCenterY < m_iCrossY-defaultlength/2)
    {
      myint x = (myint) sqrt((double)(r*r-pow((double)(m_Target.iCenterY-a),2)));
      iLength = (myint) sqrt((double)(pow((double)(x-m_iCrossX),2)+pow((double)(a-m_iCrossY),2)));
    }
  else if(m_Target.iTargetY > 2*m_iCrossY-a && m_Target.iCenterY > m_iCrossY+defaultlength/2)
    {
      myint x = (myint) sqrt((double)(r*r-pow((double)(m_Target.iCenterY+a-2*m_iCrossY),2)));
      iLength = (myint) sqrt((double)(pow((double)(x-m_iCrossX),2)+pow((double)(a-m_iCrossY),2)));
    }
  else
    iLength = defaultlength;

  //...then calculate the points required...
  double angle = ((double)iLength/(double)r)/(double)noOfPoints;

  for(int n = 1; n < noOfPoints; ++n)
    {
      iX[n] = (myint) (cos(angle)*(iX[n-1]) - sin(angle)*(iY[n-1]-m_Target.iCenterY));
      iY[n] = (myint) (m_Target.iCenterY + sin(angle)*(iX[n-1]) + cos(angle)*(iY[n-1]-m_Target.iCenterY));
    }
  //...then plot it.
  pView->DasherPolyarrow(iX, iY, noOfPoints, GetLongParameter(LP_LINE_WIDTH)*4, gameColour, 1.414);

}


void CDasherGameMode::DrawTargetArrow(Dasher::CDasherView* pView) {
 // This plots an arrow to show where the target is.
  // Probably too much floating point maths here, sort later.

  const int gameColour = 135; //Neon green. (!)
  const int noOfPoints = 2; // The curve will be made up of 2 straigt segments...

  myint iX[noOfPoints];
  myint iY[noOfPoints];
  myint iLength = m_iCrossX/5; 
  myint iYmax = GetLongParameter(LP_MAX_Y);
  if(m_Target.iTargetY > 0 && m_Target.iTargetY < iYmax)
    {
      iX[0] = myint(-1.5*iLength);
      iY[0] = m_Target.iTargetY;
      iX[1] = myint(-0.5*iLength);
      iY[1] = m_Target.iTargetY;
      
    }
  else if(m_Target.iTargetY <= 0)
    {
      iX[0] = myint(-0.5*iLength);
      iY[0] = iLength;
      iX[1] = myint(-0.5*iLength);
      iY[1] = 0;
    }
  else
    {
      iX[0] = myint(-0.5*iLength);
      iY[0] = iYmax - iLength;
      iX[1] = myint(-0.5*iLength);
      iY[1] = iYmax;
    }
  
  // Now draw it
  pView->DasherPolyarrow(iX, iY, noOfPoints, GetLongParameter(LP_LINE_WIDTH)*4, gameColour);
}

std::pair<double,double> GaussianRand() // Is there a random number class already?
{
  double u1 = (double(rand())+1)/(double(RAND_MAX)+1.0);
  double u2 = (double(rand())+1)/(double(RAND_MAX)+1.0);
  double g1 = sqrt(-2.0*log(u1))*cos(6.283185307*u2);
  double g2 = sqrt(-2.0*log(u1))*sin(6.283185307*u2);
  //  std::cout << u1 << " : " << u2 << " : " << g1 << std::endl;
  return std::pair<double,double>(g1,g2);
}
