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
    class LevelEnd;
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
  Level* GetNextLevel(){
    Level* p = NextLevel();
    p->m_bIsCompleted = false;
    delete this;
    return p;
  }
  virtual std::string GetRules(){
    return std::string("Generic Rules");
  }
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
private:
  virtual Level* NextLevel() = 0;
};


class Dasher::GameMode::LevelEnd : public Level {
public:
  LevelEnd(CDasherGameMode* pGameParent):Level(pGameParent){
    m_strLevel = "Done!";
  }
  std::string GetRules(){
    return std::string("Final rules and congratulations go here");
  }
private:
  Level* NextLevel() {
    return new LevelEnd(m_pGameParent);
    }
};



class Dasher::GameMode::Level3 : public Level {
 public:
  Level3(CDasherGameMode* pGameParent):Level(pGameParent)
  { m_iOscillatorOn=0;
    m_iOscillatorOff=2000;
    m_strLevel="3";}
};

class Dasher::GameMode::Level2 : public Level {
public:
 Level2(CDasherGameMode* pGameParent):Level(pGameParent),
    oldTime(0), oldNats(0.0), m_dCurrentScore(0), m_iErrorSize(2048),
    bFixing(false), bMadeError(false), iErrors(0)
    {m_iOscillatorOn=2000;
     m_iOscillatorOff=500;
     m_strLevel = "2";}
 std::string GetRules() {
    return std::string("Level 2 rules go here");
  }

  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();
 private:
  void ComputeNewPoints();
  Level* NextLevel() {
    return new LevelEnd(m_pGameParent);
  };

  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
};

class Dasher::GameMode::Level1 : public Level {
 public:
  Level1(CDasherGameMode* pGameParent):Level(pGameParent),
    oldTime(0), oldNats(0.0), m_dCurrentScore(0), m_iErrorSize(2048),
    bFixing(false), bMadeError(false), iErrors(0)
    {m_iOscillatorOn=500;
     m_iOscillatorOff=2000;
     m_strLevel = "1";}

  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();

  std::string GetRules() {
    return std::string("Level 1 rules go here");
  }
 private:
  void ComputeNewPoints();
  Level* NextLevel() {
    return new Level2(m_pGameParent);
  }

  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
  
};

class Dasher::GameMode::LevelStart : public Level {
public:
  LevelStart(CDasherGameMode* pGameParent):Level(pGameParent){}
  std::string GetRules() {
    return std::string("These are the opening rules");
  }
private:
  Level* NextLevel() {
    return new Level1(m_pGameParent);
    }
};
#endif
