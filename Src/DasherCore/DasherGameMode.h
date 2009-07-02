#ifndef __DasherGameMode_h__
#define __DasherGameMode_h__

#include <vector>
#include <list>
#include <cmath>

#include "../Common/NoClones.h"

#include "DasherComponent.h"
#include "DasherTypes.h"


namespace Dasher {
  class CDasherModel;
  class CDasherInterfaceBase;
  class CEventHandler;
  class CDasherComponent;
  class CDasherView;
  
  namespace GameMode {
    class CDasherGameMode;
    class Level;
    class Scorer;
    class Zero_aryCallback;
    class DelaySet;

    enum {GM_ERR_NO_ERROR = 0, GM_ERR_NO_GAME_FILE, GM_ERR_BAD_GAME_FILE, GM_ERR_NO_STRING,
      GM_ERR_LOOP};

    typedef void (*CallbackFunction)(Dasher::GameMode::CDasherGameMode * const);
    typedef std::unary_function<Dasher::GameMode::CDasherGameMode* const, void> CallbackFunctor;
    typedef void (Dasher::GameMode::CDasherGameMode::*GameFncPtr)();
    typedef std::string UTF8Char;
    typedef std::list< std::pair<unsigned long, GameFncPtr > > CallbackList;
    typedef std::list< std::pair<unsigned long, Zero_aryCallback* > > FunctorCallbackList;
  }
}

/// \defgroup GameMode Game mode support
/// @{
class Dasher::GameMode::CDasherGameMode:public Dasher::CDasherComponent, private NoClones {

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

  UTF8Char GetSymbolAtOffset(unsigned int iOffset);
  void SentenceFinished();

  void HandleEvent(Dasher::CEvent *);
  void DrawGameDecorations(CDasherView* pView);

  void Message(int message, void* messagedata);
  friend class Level;
  
private:
  CDasherGameMode(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase * pDashIface);
  ~CDasherGameMode();

  class Demo : public CDasherComponent {
  public:
    Demo(CSettingsStore * pSettingsStore, bool fullDemo=false):CDasherComponent(NULL, pSettingsStore),
      sp_alphabet_id(GetStringParameter(SP_ALPHABET_ID)),
      bp_draw_mouse(GetBoolParameter(BP_DRAW_MOUSE)),
      bp_auto_speedcontrol(GetBoolParameter(BP_AUTO_SPEEDCONTROL)),
      sp_input_filter(GetStringParameter(SP_INPUT_FILTER)),
      bFullDemo(fullDemo){
      SetBoolParameter(BP_DRAW_MOUSE, true);
      SetStringParameter(SP_INPUT_FILTER, "Normal Control");}
    ~Demo()
    {
      SetBoolParameter(BP_DRAW_MOUSE, bp_draw_mouse);
      SetStringParameter(SP_ALPHABET_ID, sp_alphabet_id);
      SetBoolParameter(BP_AUTO_SPEEDCONTROL, bp_auto_speedcontrol);
      SetStringParameter(SP_INPUT_FILTER, sp_input_filter);
    }
    
    const std::string sp_alphabet_id;
    const bool bp_draw_mouse;
    const bool bp_auto_speedcontrol;
    const std::string sp_input_filter;
    bool bFullDemo;
  };

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

  struct Oscillator {
    bool* pVariable;
    unsigned int timeOn;
    unsigned int timeOff;
  };
  
  struct Score {
    unsigned int m_iLevel;
    unsigned int m_iScore;
  };

  // Performs the necessary notifications to the rest of DasherCore
  void NotifyGameCooperators(bool bGameOn);
  
  // Starting an stopping the interactive game
  void GameModeStart();
  void GameModeStop();
  void Oscillator();
  void RunningScoreUpdates();
  void ScoreUpdate();

  // Starting and stopping the demo behaviour
  void DemoModeStart(bool bFullDemo);
  void DemoModeStop();

  // Routines for getting the next sentence
  void PrivateSentenceFinished();
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
  //void DrawPoints(CDasherView* pView);

  inline void Callback(GameFncPtr f, unsigned long wait){
    m_lCallbacks.push_back(std::pair<unsigned long,GameFncPtr>(m_ulTime+wait,f) );
  }

  inline void FunctorCallback(Zero_aryCallback* f, unsigned long wait){
    m_lFunctorCallbacks.push_back(std::pair<unsigned long,Zero_aryCallback*>(m_ulTime+wait,f) );
  }

  friend class Zero_aryCallback;

  myint ComputeBrachCenter(const myint& iTargetY, const myint& iCrossX, const myint& iCrossY);

  // The one instance of ourself.
  static CDasherGameMode* pTeacher;

  // The classes from whom we require some cooperation
  CDasherInterfaceBase *m_pDasherInterface;
  CDasherView* m_pView;
  CDasherModel *m_pModel;

  // Storing the target strings
  std::string m_strCurrentTarget;
  std::vector < std::vector<UTF8Char> > m_vTargetStrings;
  unsigned int m_iNumStrings;
  int m_iCurrentString;

  std::string m_strGameTextFile;

  CallbackList m_lCallbacks;
  FunctorCallbackList m_lFunctorCallbacks;

  // Internal status
  bool m_bGameModeOn;
  bool m_bSentenceFinished;
  bool m_bOscillator;
  unsigned int m_iOscillatorOn;
  unsigned int m_iOscillatorOff;
  unsigned long m_ulTime;

  myint m_iUserX, m_iUserY; // User mouse position in Dasher Coordinates
  myint m_iDemoX, m_iDemoY; // Demo mouse position
  TargetInfo m_Target;
  DemoConfig m_DemoCfg;

  // owned objects
  Scorer* m_pScorer;
  Level* m_pLevel;
  Demo* m_pDemo;

  // Graphics instruction flags
  bool m_bDrawHelperArrow;
  bool m_bDrawTargetArrow;
  bool m_bDrawPoints;

  // Cross hair position
  const myint m_iCrossX, m_iCrossY, m_iMaxY;
  Score m_Score;  
};
/// @}



inline Dasher::myint Dasher::GameMode::CDasherGameMode::ComputeBrachCenter(const myint& iTargetY, const myint& iCrossX, const myint& iCrossY)
{
  // This formula computes the Dasher Y Coordinate of the center of the circle on which
  // the dasher brachistochrone lies : iCenterY

  // It comes from the pythagorean relation: iCrossX^2 + (iCenterY - iCrossY)^2 = r^2
  // where r is the radius of the circle, r = abs(iTargetY-iCenterY)
  myint iCenterY = myint(0.5*(double(m_iCrossX*m_iCrossX)/double(m_iCrossY-iTargetY)
			+double(m_iCrossY+iTargetY)));

  return iCenterY;
}


class Dasher::GameMode::Zero_aryCallback
{
 public:
  Zero_aryCallback(CDasherGameMode* pGame, unsigned int wait)
    {
      pGame->FunctorCallback(this, wait);
    }
  virtual ~Zero_aryCallback(){}
  void Callback(){Action(); delete this;}
  virtual void Action() = 0;
};

class Dasher::GameMode::DelaySet : public Dasher::GameMode::Zero_aryCallback
{
 public:
  DelaySet(CDasherGameMode* pGame, unsigned int wait, bool* pVariable, bool bValue):
    Zero_aryCallback(pGame, wait),
    m_bValue(bValue), m_pVariable(pVariable){}
  void Action(void){*m_pVariable=m_bValue;}
 private:
  bool m_bValue;
  bool* m_pVariable;
};

#endif // __DasherGameMode_h__

