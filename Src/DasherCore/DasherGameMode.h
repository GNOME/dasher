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

typedef void (CDasherGameMode::*GameFncPtr)();
typedef std::list< std::pair<GameFncPtr, unsigned long> > CallbackList;

// DasherGameMode is a teacher object which should be able to do everything that a
// human sitting next to a beginner would be able to: eg, go this way, back out,
// slow down, let me show you. When the teacher is turned on, it should be able to
// watch all the movement of the input device, it should be able to access the visible
// aspects of the model too. For this, some cooperation will have to be written into
// other classes. It should be sensitive to which input device is being used.

// DasherView is suitably placed to provide the best cooperation with the TeacherObject.
// Ideally, the TeacherObject would not have to deal with DasherModel, since a real
// teacher does not see DasherModel either, only what is rendered by DasherView.

// The NF_GAME flag can be set on Nodes, but this should not be done by the TeacherObject,
// which should form a dialogue with DasherView only - (maybe). WORK IN PROGRESS - pconlon

// Alternatively, talk with DasherModel to ensure that appropriate NF_GAME flags are set
// where required, and then let DasherView deal with it?


/// \defgroup GameMode Game mode support
/// @{
class Dasher::CDasherGameMode:public CDasherComponent, private NoClones {

public:
  
  static CDasherGameMode* CreateTeacher(CEventHandler* pEventHandler, CSettingsStore* pSettingsStore,
					CDasherInterfaceBase* pDashIface);
  static void DestroyTeacher();

  inline static CDasherGameMode* GetTeacher() {return pTeacher;}

  class Scorer;
  // Returns Dasher Coordinate of the target string
  // myint.Max() reserved for "off the screen to the bottom"
  // myint.Min()+1 reserved for "off the screen to the top"
  // myint.Min() reserved for "nothing to draw" (string finished or error)
  //  myint GetDasherCoordOfTarget();
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
  struct GamePoints {
    myint width;
    myint height;
    std::string points;
  };

  inline void SetDasherView(CDasherView* pView) {m_pView=pView;}
  inline void SetDasherModel(CDasherModel* pModel) {m_pModel=pModel;}
  void SetTargetY(const std::vector<std::pair<myint,bool> >& vTargetY);
  void NewFrame(unsigned long Time);
  
  void DemoModeGetCoordinates(myint& iDasherX, myint& iDasherY);
  void SetUserMouseCoordinates(myint iDasherX, myint iDasherY);

  std::string GetSymbolAtOffset(int iOffset);
  void SentenceFinished();

  void HandleEvent(Dasher::CEvent *);
  void DrawGameDecorations(CDasherView* pView);

  void Message(int message, void* messagedata);
  
private:
  CDasherGameMode(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase * pDashIface);
  ~CDasherGameMode();

  // loads .txt file with strings for current alphabet
  void GameModeStart();
  void GameModeStop();
  void DemoModeStart();
  void DemoModeStop();
  void DemoNext();
  void DemoGo();
  void Countdown();

  void InitializeTargetFile();
  void LoadTargetStrings(std::istream& in);
  void LoadDemoConfigFile();
  void CalculateDemoParameters();
  int NextString();

  // Private graphics routines
  void DrawHelperArrow(CDasherView* pView);
  void DrawTargetArrow(CDasherView* pView);
  void DrawPoints(CDasherView* pView);
  void DrawPaused(CDasherView* pView);

  // The one instance of ourself.
  static CDasherGameMode* pTeacher;

  // The classes from whom we require some cooperation
  CDasherModel *m_pModel;
  CDasherView* m_pView;
  CDasherInterfaceBase *m_pDasherInterface;
  
  std::string m_strCurrentTarget;
  std::vector < std::vector<std::string> > m_vTargetStrings;

  std::string m_strGameTextFile;
  std::vector< GamePoints> m_vGamePoints;
  CallbackList m_lCallbacks;
  int m_iNumStrings;
  int m_iCurrentString;
  bool m_bGameModeOn;
  bool m_bSentenceFinished;
  bool m_bDemoModeOn;
  bool m_bDrawHelperArrow;
  bool m_bDrawTargetArrow;
  bool m_bUpdateMouse;
  bool m_bDrawPoints;
  bool m_bDrawPaused;
  int counter2;
  int m_countdown;
  myint m_iUserX, m_iUserY; // User mouse position in Dasher Coordinates
  myint m_iDemoX, m_iDemoY; // Demo mouse position
  const myint m_iCrossX, m_iCrossY; // Cross-hair position
  TargetInfo m_Target;
  DemoConfig m_DemoCfg;
  unsigned long m_ulTime;
  Scorer* m_pScorer;
  void* m_pGUI;
};
/// @}

#endif

