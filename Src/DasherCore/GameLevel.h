#ifndef __GameLevel_h__
#define __GameLevel_h__

#include "DasherGameMode.h"
#include <sstream>
// Base class to represent different levels in the Dasher Game.
namespace Dasher {
  namespace GameMode {
    class LevelStart;
    class Level1;
    class Level2;
    class Level3;
  }
}

class Dasher::GameMode::Level {
 public:
  Level(CDasherGameMode* p):m_pGameParent(p),
    m_bDrawHelperArrow(p->m_bDrawHelperArrow),
    m_bDrawTargetArrow(p->m_bDrawTargetArrow),
    m_bOscillator(p->m_bOscillator),
    m_bSentenceFinished(p->m_bSentenceFinished),
    m_iOscillatorOn(p->m_iOscillatorOn),
    m_iOscillatorOff(p->m_iOscillatorOff),
    m_ulTime(p->m_ulTime),
    m_iUserX(p->m_iUserX), m_iUserY(p->m_iUserY),
    m_iMaxY(p->m_iMaxY), m_iCrossX(p->m_iCrossX),
    m_Target(p->m_Target), m_pScorer(p->m_pScorer),
    m_pModel(p->m_pModel)
    {m_bIsCompleted=true;
    m_iLevelScore=0; m_dSentenceScore=0.0;}
  virtual ~Level() {}
  virtual void DoGameLogic() {}
  virtual int GetCurrentScore() {return 0;}
  virtual void SentenceFinished() {}
  virtual void Reset() {}
  inline std::string GetLevel(){return m_strLevel;}
  virtual Level* GetNextLevel()=0;
  virtual std::string GetRules(){return std::string("Generic Rules");};
  std::ostringstream m_strPerformance;
  bool m_bIsCompleted;
 protected:
  CDasherGameMode* m_pGameParent;
  bool& m_bDrawHelperArrow;
  bool& m_bDrawTargetArrow;
  bool& m_bOscillator;
  bool& m_bSentenceFinished;
  unsigned int& m_iOscillatorOn;
  unsigned int& m_iOscillatorOff;
  unsigned long& m_ulTime;
  myint& m_iUserX, m_iUserY; // User mouse position in Dasher Coordinates
  const myint& m_iMaxY, m_iCrossX;
  CDasherGameMode::TargetInfo& m_Target;
  Scorer*& m_pScorer;
  CDasherModel*& m_pModel;
  std::string m_strLevel;
  int m_iLevelScore;
  double m_dSentenceScore;
};

class Dasher::GameMode::Level1 : public Level {
 public:
  Level1(CDasherGameMode* pGameParent):Level(pGameParent),
    oldTime(0), oldNats(0.0), m_dCurrentScore(0), m_iErrorSize(2048),
    bFixing(false), bMadeError(false), iErrors(0)
    {m_iOscillatorOn=500;
     m_iOscillatorOff=2000;
     m_strLevel = "1";}

  Level* GetNextLevel();
  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();
 private:
  void ComputeNewPoints();
  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
};


class Dasher::GameMode::Level2 : public Level {
public:
 Level2(CDasherGameMode* pGameParent):Level(pGameParent),
    oldTime(0), oldNats(0.0), m_dCurrentScore(0), m_iErrorSize(2048),
    bFixing(false), bMadeError(false), iErrors(0)
    {m_iOscillatorOn=2000;
     m_iOscillatorOff=500;
     m_strLevel = "2";}

  Level* GetNextLevel();
  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();
 private:
  void ComputeNewPoints();
  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
};

class Dasher::GameMode::Level3 : public Level {
 public:
  Level3(CDasherGameMode* pGameParent):Level(pGameParent) {m_iOscillatorOn=0; m_iOscillatorOff=2000;}
  void DoGameLogic()
    {
      if(m_Target.iTargetY < -500 || m_iMaxY+500 < m_Target.iTargetY)
	m_bDrawHelperArrow = true;
      else
	m_bDrawHelperArrow = false;
      
      if((m_bOscillator || m_iUserX > m_iCrossX))
	m_bDrawTargetArrow=true;
      else
	m_bDrawTargetArrow=false;
    }
  int GetCurrentScore()
  {
    //    double dNats = m_pModel->GetNats();
    //    double efficieny = 1.4427*(100.0*1000*dNats)/(double(m_pScorer->GetTime())*m_pGameParent->GetLongParameter(LP_MAX_BITRATE));
    //    return efficieny*100;
    return 0;
  }
};

class Dasher::GameMode::LevelStart : public Level {
public:
  LevelStart(CDasherGameMode* pGameParent):Level(pGameParent){}
  Level* GetNextLevel() {
    Level* p = new Level1(m_pGameParent);
    p->m_bIsCompleted = false;
    delete this;
    return p;}
};
#endif
