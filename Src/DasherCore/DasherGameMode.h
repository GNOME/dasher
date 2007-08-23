#ifndef __DasherGameMode_h__
#define __DasherGameMode_h__

#include "../Common/NoClones.h"
#include "LanguageModelling/LanguageModel.h"
#include "DasherComponent.h"
#include "Alphabet/Alphabet.h"
#include "AlphabetManagerFactory.h"
#include "ControlManagerFactory.h"
#include <math.h>
#include "DasherTypes.h"
#include "FrameRate.h"

#include <vector>
#include <list>
#include <deque>
#include <fstream>
#include <iostream>

namespace Dasher {
  class CDasherModel;
  class CDasherInterfaceBase;
  class CEventHandler;
  class CDasherGameMode;
  class CDasherComponent;
  class CDasherView;
}

enum {GM_ERR_NO_ERROR = 0, GM_ERR_NO_GAME_FILE, GM_ERR_BAD_GAME_FILE, GM_ERR_NO_STRING,
      GM_ERR_LOOP};

typedef void (CDasherGameMode::*GameFncPtr)();
typedef std::string UTF8Char;
typedef std::list< std::pair<GameFncPtr, unsigned long> > CallbackList;

/// \defgroup GameMode Game mode support
/// @{
class Dasher::CDasherGameMode:public CDasherComponent, private NoClones {

public:
  
  static CDasherGameMode* CreateTeacher(CEventHandler* pEventHandler, CSettingsStore* pSettingsStore,
					CDasherInterfaceBase* pDashIface);
  static void DestroyTeacher();

  inline static CDasherGameMode* GetTeacher() {return pTeacher;}

  inline void SetDasherView(CDasherView* pView) {m_pView=pView;}
  inline void SetDasherModel(CDasherModel* pModel) {m_pModel=pModel;}
  void SetTargetY(const std::vector<std::pair<myint,bool> >& vTargetY);
  void NewFrame(unsigned long Time);
  
  void DemoModeGetCoordinates(myint& iDasherX, myint& iDasherY);
  void SetUserMouseCoordinates(myint iDasherX, myint iDasherY);

  UTF8Char GetSymbolAtOffset(int iOffset);
  void SentenceFinished();

  void HandleEvent(Dasher::CEvent *);
  void DrawGameDecorations(CDasherView* pView);

  void Message(int message, void* messagedata);
  
private:
  CDasherGameMode(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase * pDashIface);
  ~CDasherGameMode();

  class Demo : public CDasherComponent {
  public:
    Demo(CSettingsStore * pSettingsStore, bool full=false):CDasherComponent(NULL, pSettingsStore),
      sp_alphabet_id(GetStringParameter(SP_ALPHABET_ID)),
      bp_draw_mouse(GetBoolParameter(BP_DRAW_MOUSE)),
      bFullDemo(full){
      SetBoolParameter(BP_DRAW_MOUSE, true);}
    ~Demo(){SetBoolParameter(BP_DRAW_MOUSE, bp_draw_mouse);
    SetStringParameter(SP_ALPHABET_ID, sp_alphabet_id);}
    
    const std::string sp_alphabet_id;
    const bool bp_draw_mouse;
    bool bFullDemo;
  };

  class Scorer;

  struct TargetInfo {
    myint iTargetY, iVisibleTargetY;
    myint iCenterY, iVisibleCenterY;    
  };

  struct DemoConfig {
    double dSpring;
    int iNoiseMag;
    double dNoiseNew;
    double dNoiseOld;
  };

  // Performs the necessary notifications to the rest of DasherCore
  void NotifyGameCooperators(bool bGameOn);
  
  // Starting an stopping the interactive game
  void GameModeStart();
  void GameModeStop();

  // Starting and stopping the demo behaviour
  void DemoModeStart(bool bFullDemo);
  void DemoModeStop();

  // Routines for getting the next sentence
  void FullDemoNext();
  void GameNext();
  int NextString(bool bRandomString = false);

  void DemoGo();
  
  // Loading the target strings, and demo settings
  int InitializeTargets();
  int LoadTargetStrings(std::istream& in);
  void LoadDemoConfigFile();
  void CalculateDemoParameters();

  // Private graphics routines
  void DrawHelperArrow(CDasherView* pView);
  void DrawTargetArrow(CDasherView* pView);
  void DrawPoints(CDasherView* pView);

  myint ComputeBrachCenter(const myint& iTargetY, const myint& iCrossX, const myint& iCrossY);

  // The one instance of ourself.
  static CDasherGameMode* pTeacher;

  // Cross hair position
  const myint m_iCrossX, m_iCrossY;

  // The classes from whom we require some cooperation
  CDasherInterfaceBase *m_pDasherInterface;
  CDasherView* m_pView;
  CDasherModel *m_pModel;

  // Storing the target strings
  std::string m_strCurrentTarget;
  std::vector < std::vector<UTF8Char> > m_vTargetStrings;
  int m_iNumStrings;
  int m_iCurrentString;

  std::string m_strGameTextFile;

  CallbackList m_lCallbacks;

  // Internal status
  bool m_bGameModeOn;
  bool m_bSentenceFinished;
  bool m_bDemoModeOn;
  unsigned long m_ulTime;

  myint m_iUserX, m_iUserY; // User mouse position in Dasher Coordinates
  myint m_iDemoX, m_iDemoY; // Demo mouse position
  TargetInfo m_Target;
  DemoConfig m_DemoCfg;

  // owned objects
  Scorer* m_pScorer;
  Demo* m_pDemo;

  // Graphics instruction flags
  bool m_bDrawHelperArrow;
  bool m_bDrawTargetArrow;
  bool m_bDrawPoints;

};
/// @}



inline myint CDasherGameMode::ComputeBrachCenter(const myint& iTargetY, const myint& iCrossX, const myint& iCrossY)
{

  // This formula computes the Dasher Y Coordinate of the center of the circle on which
  // the dasher brachistochrone lies : iCenterY

  // It comes from the pythagorean relation: iCrossX^2 + (iCenterY - iCrossY)^2 = r^2
  // where r is the radius of the circle, r = abs(iTargetY-iCenterY)
  myint iCenterY = myint(0.5*(double(m_iCrossX*m_iCrossX)/double(m_iCrossY-iTargetY)
			+double(m_iCrossY+iTargetY)));

  return iCenterY;
}

#endif

