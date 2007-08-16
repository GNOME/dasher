
#include "../Common/Common.h"

#include "DasherGameMode.h"
#include "DasherInterfaceBase.h"
#include "DasherModel.h"
#include "DasherNode.h"
#include "DasherView.h"
#include "Event.h"

#include "GameScorer.h"
#include "GameMessages.h"

#include <limits>
#include <iostream>
#include <fstream>

std::pair<double,double> GaussianRand();

CDasherGameMode* CDasherGameMode::pTeacher = NULL;

CDasherGameMode* CDasherGameMode::CreateTeacher(CEventHandler *pEventHandler,
						CSettingsStore *pSettingsStore,
						CDasherInterfaceBase *pDashIface)
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

CDasherGameMode::CDasherGameMode(CEventHandler *pEventHandler,
				 CSettingsStore *pSettingsStore,
				 CDasherInterfaceBase *pDashIface)
  :CDasherComponent(pEventHandler, pSettingsStore),
   m_pDasherInterface(pDashIface),
   m_pView(NULL), m_pModel(NULL),
   m_iCrossX((myint)GetLongParameter(LP_OX)),
   m_iCrossY((myint)GetLongParameter(LP_OY))
{
  // Create the GameMode object in the background
  // m_bGameModeOn tells us whether we should be alive or not.
  // we listen for parameter change events, and wake up if necessary.

  m_bGameModeOn = GetBoolParameter(BP_GAME_MODE);
  m_strGameTextFile = GetStringParameter(SP_GAME_TEXT_FILE);

  m_bDrawHelperArrow = false;
  m_bDrawTargetArrow = true;
  m_bDemoModeOn = false;
  m_bDrawPoints = false;
  counter2 = 0;
  m_pScorer = new Scorer;
  if(m_bGameModeOn)
    GameModeStart();
}

CDasherGameMode::~CDasherGameMode()
{
}

void CDasherGameMode::Message(int message, void* messagedata)
{
  switch(message){
  case GAME_MESSAGE_REGHELPER:
    m_pGUI = messagedata;
    break;
  case GAME_MESSAGE_NEXT:
    DemoNext();
    break;
  case GAME_MESSAGE_DEMO:
    if(m_bDemoModeOn)
      DemoModeStop();
    else
      DemoModeStart();
    break;
  }

      
}

void CDasherGameMode::GameModeStart()
{
  m_strGameTextFile = GetStringParameter(SP_GAME_TEXT_FILE);

  std::cout << "Welcome to Dasher Game Mode! \n"
	    << "Current game file is:" << m_strGameTextFile << "\n"; 
  InitializeTargetFile();
  m_pDasherInterface->GameMessageOut(m_pGUI, GAME_MESSAGE_WELCOME, "Welcome to Dasher Game Mode!");

  m_bSentenceFinished = true;
  m_pDasherInterface->SetContext(std::string(""));
  m_pDasherInterface->SetBuffer(0);
  m_pView->SetGameMode(true);
  
}

void CDasherGameMode::GameModeStop()
{
  std::cout << "Thank you for playing Dasher Game Mode!" << std::endl; 

  DemoModeStop();
  m_pView->SetGameMode(false);
  
  m_pDasherInterface->SetContext(std::string(""));
  m_pDasherInterface->SetBuffer(0);

  
}

void CDasherGameMode::DemoModeStart()
{
  m_bDemoModeOn = true;
  m_bDrawHelperArrow=false;
  m_bDrawTargetArrow=false;
  LoadDemoConfigFile();
 
  SetBoolParameter(BP_DRAW_MOUSE, m_bDemoModeOn);
  m_pView->SetDemoMode(m_bDemoModeOn);
  m_iDemoX = m_iUserX;
  m_iDemoY = m_iUserY;
  
}

void CDasherGameMode::DemoModeStop()
{
  m_bDemoModeOn=false;
  m_bDrawTargetArrow=true;
  SetBoolParameter(BP_DRAW_MOUSE, m_bDemoModeOn);
  m_pView->SetDemoMode(m_bDemoModeOn);
}


void CDasherGameMode::HandleEvent(Dasher::CEvent * pEvent) 
{
  // If GameMode is not turned on, only listen for BP_GAME_MODE events
  if(!m_bGameModeOn)
    {
      if(pEvent->m_iEventType == EV_PARAM_NOTIFY)
	{
	  if(static_cast<CParameterNotificationEvent*>(pEvent)->m_iParameter == BP_GAME_MODE)
	    {
	      m_bGameModeOn = GetBoolParameter(BP_GAME_MODE);
	      GameModeStart();
	    }
	}
      return;
    }
  else
    {
      // Otherwise listen for all events and deal with them appropriately
      switch(pEvent->m_iEventType)
	{
	case EV_PARAM_NOTIFY:

	  switch(static_cast<CParameterNotificationEvent*>(pEvent)->m_iParameter)
	    {
	    case BP_GAME_MODE:
	      m_bGameModeOn = GetBoolParameter(BP_GAME_MODE);
	      GameModeStop();
	      break;	  
	    case SP_GAME_TEXT_FILE:
	      m_strGameTextFile = GetStringParameter(SP_GAME_TEXT_FILE);
	      std::cout << "Change of game file to " << m_strGameTextFile << std::endl;
	      InitializeTargetFile();
	      m_pDasherInterface->GameMessageOut(m_pGUI, GAME_MESSAGE_WELCOME, "Welcome to Dasher Game Mode!");
	      m_bSentenceFinished = true;
	      break;
	    case LP_MAX_BITRATE: 
	      CalculateDemoParameters();
	      break;
	    }
	  break;
	case EV_STOP:
	  //	  if(!m_bDemoModeOn)
	  //	    DemoModeStart();
	  //	  else
	  //	    DemoModeStop();
	  m_bDrawPaused=true;
	  m_pScorer->Stop(m_ulTime);
	  break;
	case EV_START:
	  m_bDrawPaused=false;
	  if(m_bDemoModeOn) CalculateDemoParameters();
	  m_pScorer->Start(m_ulTime);
	  break;
	case EV_EDIT:
	  /*	  GamePoints gp = {2048, 2048, "100"};
		  m_vGamePoints.push_back(gp);*/
	  break;
	}
      return;
    }
}

void CDasherGameMode::DemoNext()
{
  NextString();
  m_pDasherInterface->GameMessageOut(m_pGUI, GAME_MESSAGE_SET_STRING, (void *)(m_strCurrentTarget.c_str()));

  m_bDrawPoints=false;
  m_pDasherInterface->SetContext(std::string(""));
  m_pDasherInterface->SetBuffer(0);
  m_pDasherInterface->PauseAt(0,0);
  m_pScorer->Reset();
  if(m_bDemoModeOn)
    m_lCallbacks.push_back(std::pair<GameFncPtr, unsigned long>(&CDasherGameMode::DemoGo,m_ulTime+2000));

  
}

void CDasherGameMode::DemoGo()
{
  CalculateDemoParameters();
  m_pDasherInterface->Unpause(m_ulTime);
}

void CDasherGameMode::NewFrame(unsigned long Time)
{
  // Set our internal clock
  m_ulTime = Time;

  m_pScorer->NewFrame(Time,0,0, m_Target.iTargetY);
  // DasherInterfaceBase doesn't provide callback functionality, so we create our own
  // We only callback member functions of this class
  std::vector<CallbackList::iterator> vDeleteList;

  for(CallbackList::iterator it = m_lCallbacks.begin();
      it != m_lCallbacks.end(); ++it)
    {
      if((*it).second < Time)
	{
	  (this->*(*it).first)();
	  vDeleteList.push_back(it);
	}
    }
  
  for(std::vector<CallbackList::iterator>::iterator it = vDeleteList.begin();
      it != vDeleteList.end(); ++it)
    {
      m_lCallbacks.erase(*it);
    }
  
}



void CDasherGameMode::SetTargetY(const std::vector<std::pair<myint,bool> >& vTargetY)
{
  if(vTargetY.size()!=0)
    {
      m_Target.iTargetY = vTargetY.back().first;
      m_Target.iCenterY = 0.5*((double)(m_iCrossX*m_iCrossX)/(double)(m_iCrossY-m_Target.iTargetY)
			+(m_iCrossY+m_Target.iTargetY));

      for(std::vector<std::pair<myint,bool> >::const_reverse_iterator i=vTargetY.rbegin();
	  i!=vTargetY.rend(); ++i)
	{
	  if((*i).second == true)
	    {
	      m_Target.iVisibleTargetY = (*i).first;
	      m_Target.iVisibleCenterY = 0.5*((double)(m_iCrossX*m_iCrossX)/(double)(m_iCrossY-m_Target.iVisibleTargetY)
			+(m_iCrossY+m_Target.iVisibleTargetY));
	      break;
	    }
	}
    }
  /*
  if(m_Target.iTargetY < 0 || 4096 < m_Target.iTargetY)
    m_bDrawHelperArrow = true;
  else
    m_bDrawHelperArrow = false;
  */
}


void CDasherGameMode::CalculateDemoParameters()
{
  double spring = GetLongParameter(LP_DEMO_SPRING)/100.0;
  double noisemem = GetLongParameter(LP_DEMO_NOISE_MEM)/100.0;
  double lambda = 0.7*double(GetLongParameter(LP_MAX_BITRATE))/(100.0*m_pModel->Framerate());

  m_DemoCfg.dSpring = (1-exp(-spring*lambda));
  m_DemoCfg.dNoiseNew = noisemem*(1-exp(-lambda));
  m_DemoCfg.dNoiseOld = sqrt(1.0-m_DemoCfg.dNoiseNew*m_DemoCfg.dNoiseNew);
}

void CDasherGameMode::LoadDemoConfigFile()
{
  m_DemoCfg.iNoiseMag = GetLongParameter(LP_DEMO_NOISE_MAG);
  CalculateDemoParameters();
}

void CDasherGameMode::InitializeTargetFile()
{
  m_vTargetStrings.clear();
  m_iCurrentString = -1; // -1 to indicate no string available
  m_iNumStrings = 0;
  std::string strGameFileName;
  // Check that there is a game text file...
  if(m_strGameTextFile == "")
    {
      std::cout << "This alphabet does not have a game text file! Sorry" << std::endl;
      return;
    }
  // ...then put together path to the game text file from settings (read in from AlphIO)...
  strGameFileName = GetStringParameter(SP_SYSTEM_LOC) + m_strGameTextFile;
  // ...and open it.
  ifstream GameFile(strGameFileName.c_str());
  if(!GameFile)
    {
      std::cout << "Failed to open the Game file for this alphabet!" << std::endl;
      return;
    }
  LoadTargetStrings(GameFile);
  
  m_iNumStrings = m_vTargetStrings.size();
  std::cout << m_iNumStrings << " strings loaded!" << std::endl;

  return;
}

void CDasherGameMode::LoadTargetStrings(istream& in)
{
  std::vector<std::string > TargetStrings;// All the different strings from the game file
  const char delimiter = '#'; // Could come from some sort of config file?

  while(!in.eof())
    {
      std::string strTmp;
      getline(in, strTmp, delimiter);
      if(strTmp.length()!=0)
	TargetStrings.push_back(strTmp);
    }

  // Now process the strings for unicode
  // Another way would be to use the current alphabet to do the processing for us. It is a philisophical point -
  // should the GameMode class directly interface with the current alphabet?

  // This converts the string of chars into a vector of strings, each string being a unicode character.
  for(std::vector<std::string>::const_iterator it = TargetStrings.begin();
      it != TargetStrings.end(); it++)
    {
      std::vector<std::string > vTmp; // Each "string" in this vector is one unicode character

      for(int i = 0; i != (*it).length(); ++i)
	{
	  std::string strTmp = std::string(1,(*it)[i]);
	  if((*it)[i] & 0x80)
	    {    // Character is more than 1 byte long
	      int extras = 1;
	      for(unsigned int bit = 0x20; ((*it)[i] & bit) != 0; bit >>= 1)
		extras++;
	      
	      if(extras > 5) {}  // Malformed character
	      
	      while(extras-- > 0) 
		{
		  strTmp += (*it)[++i];
		}
	    }
	  vTmp.push_back(strTmp);
	}
      m_vTargetStrings.push_back(vTmp);
    }
}

// In Demo mode, DasherView will use this function to serve up coordinates to the input filter.

void CDasherGameMode::DemoModeGetCoordinates(myint& iDasherX, myint& iDasherY)
{
  static bool madeError=false;
  static myint lastTargetY=m_Target.iVisibleTargetY;

  // First choose an appropriate target...
  if(m_Target.iTargetY>10000 || m_Target.iTargetY <-6000)
    {
      madeError=true;
    }

  myint iTargetY=(m_Target.iVisibleTargetY);
  if(madeError)
    {
      iTargetY=m_Target.iTargetY;
      if(abs(iTargetY-m_iCrossY) < 300 && abs(iTargetY - m_Target.iVisibleTargetY)<10)
	{
	  madeError=false;
	}
    }

  // ...and now calculate the ideal direction...
  double iIdealUnitVec[2];  
  
  myint iCenterY = 0.5*((double)(m_iCrossX*m_iCrossX)/(double)(m_iCrossY-iTargetY)
			+(m_iCrossY+iTargetY));

  iIdealUnitVec[0] = (m_iCrossY<iTargetY)?(iCenterY-m_iCrossY):(m_iCrossY-iCenterY);
  iIdealUnitVec[1] = (m_iCrossY<iTargetY)?m_iCrossX:-m_iCrossX;
  myint mag = sqrt((double)(iIdealUnitVec[0]*iIdealUnitVec[0]+iIdealUnitVec[1]*iIdealUnitVec[1]));
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
      m_iDemoX = (m_iCrossX+(1500*iIdealUnitVec[0])+m_DemoCfg.iNoiseMag*noiseX)*m_DemoCfg.dSpring
	+(1.0-m_DemoCfg.dSpring)*m_iDemoX;
      m_iDemoY = (m_iCrossY+(1500*iIdealUnitVec[1])+m_DemoCfg.iNoiseMag*noiseY)*m_DemoCfg.dSpring
	+(1.0-m_DemoCfg.dSpring)*m_iDemoY;
    }
  else
    {
      m_iDemoX = m_iCrossX*m_DemoCfg.dSpring + (1.0-m_DemoCfg.dSpring)*m_iDemoX;
      m_iDemoY = m_iCrossY*m_DemoCfg.dSpring + (1.0-m_DemoCfg.dSpring)*m_iDemoY;
    }

  // ...and finally set the mouse coordinates.
  iDasherX=m_iDemoX;
  iDasherY=m_iDemoY;
}

// In Gamemode, DasherView will helpfully call this function whenever it provides
// coordinates to the inputfilters - ie, we can track the mouse from this
void CDasherGameMode::SetUserMouseCoordinates(myint iDasherX, myint iDasherY)
{
  m_iUserX = iDasherX;
  m_iUserY = iDasherY;
}

void CDasherGameMode::SentenceFinished()
{
  m_bSentenceFinished=true;
  m_pScorer->SentenceFinished(m_ulTime);
  m_bDrawPoints=true;
  if(m_bDemoModeOn)
    m_lCallbacks.push_back(std::pair<GameFncPtr, unsigned long>(&CDasherGameMode::DemoNext,m_ulTime+7000));
  
}

std::string CDasherGameMode::GetSymbolAtOffset(int iOffset)
{ 
  // The string "GameEnd" indicates the NF_END_GAME flag should be set on the node.
  if(m_iCurrentString == -1)
    return std::string("GameEnd");
  
  if(iOffset < m_vTargetStrings[m_iCurrentString].size())
    return m_vTargetStrings[m_iCurrentString][iOffset];
  
  return std::string("GameEnd");
}

int CDasherGameMode::NextString()
{
  int ret=0;
  if(m_iNumStrings <= 0)
    {
      m_iCurrentString = -1;
      std::cout << "No string available" << std::endl;
      return -1;
    }
  
  if(m_iCurrentString<m_iNumStrings-1)
    m_iCurrentString++;
  else
    {
      m_iCurrentString=0; // Loop back to beginning
      ret=1;
    }

  m_strCurrentTarget = "";
  for(int i = 0; i != m_vTargetStrings[m_iCurrentString].size(); ++i)
    m_strCurrentTarget += m_vTargetStrings[m_iCurrentString][i];
  m_bSentenceFinished=false;
  
  return ret;
}

// Drawing Methods for Game/Teacher Mode

void CDasherGameMode::DrawGameDecorations(CDasherView* pView)
{
  static myint height=2048;
  if((height+=64) >= 4096) height = 2048;
  if(!m_bGameModeOn) return;

  if(m_bDrawHelperArrow)
    DrawHelperArrow(pView);
  if(m_bDrawTargetArrow)
    DrawTargetArrow(pView);
  if(m_bDrawPoints)
    DrawPoints(pView);
  //  if(m_bDrawPaused)
  //    DrawPaused(pView);
  //  DrawScoreBoard
  //  etc
}

void CDasherGameMode::Countdown()
{
  if(m_countdown != 0)
    {
      m_countdown--;
      m_lCallbacks.push_back(std::pair<GameFncPtr, unsigned long>(&CDasherGameMode::Countdown, m_ulTime+2000));
    }
}

void CDasherGameMode::DrawPaused(CDasherView* pView)
{
  static int count = 0;
  if(count < 10)
    pView->DrawText("Paused!",16000,2048, 20);
  else if (count == 20) count=0;
  ++count;
}
void CDasherGameMode::DrawPoints(CDasherView* pView)
{
  pView->DrawText(m_pScorer->GetBreakdown(),16000,1000, 20);
}

void CDasherGameMode::DrawHelperArrow(CDasherView* pView)
{
 // This plots a helpful pointer to the best direction to take to get to the target.
  // Probably too much floating point maths here, sort later.
 // Start of line is the crosshair location
  const int gameColour = 136; //Neon green. (!)
  const int noOfPoints = 10; // The curve will be made up of 9 straigt segments...

  myint iX[noOfPoints];
  myint iY[noOfPoints];
  myint iLength;

  iX[0] = m_iCrossX;
  iY[0] = m_iCrossY;

  myint a = iX[0]/5;
  myint defaultlength = iX[0] - a ; 

  // ... then decide the length of the arrow...
  myint r = m_Target.iTargetY-m_Target.iCenterY; // radius of our circle (+ or -)

  if(m_Target.iTargetY < a && m_Target.iCenterY < m_iCrossY-defaultlength/2)
    {
      myint x = sqrt((double)(r*r-pow((long double)(m_Target.iCenterY-a),2)));
      iLength = sqrt((double)(pow((long double)(x-m_iCrossX),2)+pow((long double)(a-m_iCrossY),2)));
    }
  else if(m_Target.iTargetY > 2*m_iCrossY-a && m_Target.iCenterY > m_iCrossY+defaultlength/2)
    {
      myint x = sqrt((double)(r*r-pow((long double)(m_Target.iCenterY+a-2*m_iCrossY),2)));
      iLength = sqrt((double)(pow((long double)(x-m_iCrossX),2)+pow((long double)(a-m_iCrossY),2)));
    }
  else
    iLength = defaultlength;

  //...then calculate the points required...
  double angle = ((double)iLength/(double)r)/(double)noOfPoints;

  for(int n = 1; n < noOfPoints; ++n)
    {
      iX[n] = cos(angle)*(iX[n-1]) - sin(angle)*(iY[n-1]-m_Target.iCenterY);
      iY[n] = m_Target.iCenterY + sin(angle)*(iX[n-1]) + cos(angle)*(iY[n-1]-m_Target.iCenterY);
    }
  //...then plot it.
  pView->DasherPolyarrow(iX, iY, noOfPoints, GetLongParameter(LP_LINE_WIDTH)*4, gameColour);

}


void CDasherGameMode::DrawTargetArrow(CDasherView* pView) {
 // This plots an arrow to show where the target is.
  // Probably too much floating point maths here, sort later.

  const int gameColour = 136; //Neon green. (!)
  const int noOfPoints = 2; // The curve will be made up of 9 straigt segments...
  const int arrowPoints = 3; // ...and two straight segments for the arrow.
  myint iX[noOfPoints+arrowPoints];
  myint iY[noOfPoints+arrowPoints];
  myint iLength = m_iCrossX/5; 
  myint iYmax = GetLongParameter(LP_MAX_Y);
  if(m_Target.iTargetY > 0 && m_Target.iTargetY < iYmax)
    {
      iX[0] = -2*iLength;
      iY[0] = m_Target.iTargetY;
      iX[1] = -1*iLength;
      iY[1] = m_Target.iTargetY;
      
    }
  else if(m_Target.iTargetY <= 0)
    {
      iX[0] = -1.5*iLength;
      iY[0] = iLength;
      iX[1] = -1.5*iLength;
      iY[1] = 0;
    }
  else
    {
      iX[0] = -1.5*iLength;
      iY[0] = iYmax - iLength;
      iX[1] = -1.5*iLength;
      iY[1] = iYmax;
    }
  // Put an arrow on the end of the arc. i.e, two straight segments 45 deg either side.
  myint iXvec = (iX[noOfPoints-2] - iX[noOfPoints-1])*0.6;
  myint iYvec = (iY[noOfPoints-2] - iY[noOfPoints-1])*0.6;
  
  iX[noOfPoints  ] = iX[noOfPoints-1] + iXvec + iYvec;
  iY[noOfPoints  ] = iY[noOfPoints-1] - iXvec + iYvec;
  iX[noOfPoints+1] = iX[noOfPoints-1] ;
  iY[noOfPoints+1] = iY[noOfPoints-1] ;
  iX[noOfPoints+2] = iX[noOfPoints-1] + iXvec - iYvec;
  iY[noOfPoints+2] = iY[noOfPoints-1] + iXvec + iYvec;
  
  // Now draw it
  pView->DasherPolyline(iX, iY, noOfPoints+arrowPoints, GetLongParameter(LP_LINE_WIDTH)*4, gameColour);
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
