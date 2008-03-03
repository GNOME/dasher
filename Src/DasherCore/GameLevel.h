#ifndef __GameLevel_h__
#define __GameLevel_h__

#include "DasherGameMode.h"
#include <sstream>

namespace Dasher {
  namespace GameMode {
    class LevelEnd;
    class Level4;
    class Level3;
    class Level2;
    class Level1;
    class LevelStart;
  }
}

// Base class to represent different levels in the Dasher Game, provides
// the appropriate access to the GameParent
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
    m_Target(p->m_Target), m_pScorer(p->m_pScorer)
  {
    m_bIsCompleted=true;
    m_iLevelScore=0;
    m_dSentenceScore=0.0;
  }
  virtual ~Level() {}
 
  // Interface
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
    return std::string("The rules of the level.");
  }
  virtual bool IsCompleted() {
    return m_bIsCompleted;
  }
  // TODO: make this a method interface?
  std::ostringstream m_strPerformance;
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
  std::string m_strLevel;
  int m_iLevelScore;
  double m_dSentenceScore;
  bool m_bIsCompleted;
private:
  virtual Level* NextLevel() = 0;
};

// The end level - not a real level
class Dasher::GameMode::LevelEnd : public Dasher::GameMode::Level {
public:
  LevelEnd(CDasherGameMode* pGameParent):Level(pGameParent){
    m_strLevel = "Done!";
  }
  std::string GetRules(){
    return std::string("Well done, well done, etc etc");
  }
private:
  Level* NextLevel() {
    return new LevelEnd(m_pGameParent);
    }
};
// Level 4
class Dasher::GameMode::Level4 : public Dasher::GameMode::Level {
 public:
  Level4(CDasherGameMode* pGameParent):Level(pGameParent)
  {
    // In Level 4, there are no arrows at all.
    m_iOscillatorOn=0;
    m_iOscillatorOff=2000;
    m_strLevel="4";
  }
private:
  // We must provide this function to be instantiated
  Level* NextLevel() {
    return new Level4(m_pGameParent);
  };
};

// Level 3
class Dasher::GameMode::Level3 : public Dasher::GameMode::Level {
 public:
  Level3(CDasherGameMode* pGameParent):Level(pGameParent)
  {
    // In level 3, the flashing arrow is always off
    m_iOscillatorOn=0;
    m_iOscillatorOff=2000;
    m_strLevel="3";
  }
private:
  // We must provide this function to be instantiated
  Level* NextLevel() {
    return new Level4(m_pGameParent);
  };
};

class Dasher::GameMode::Level2 : public Dasher::GameMode::Level {
public:
  Level2(CDasherGameMode* pGameParent):Level(pGameParent),
    oldTime(0), m_dCurrentScore(0), oldNats(0.0), iErrors(0),
    m_iErrorSize(2048), bFixing(false), bMadeError(false)
  {
    // In level 2, the flashing arrow is mainly off
    m_iOscillatorOn=500;
    m_iOscillatorOff=3000;
    m_strLevel = "2";
  }
  std::string GetRules() {
    return std::string("Level 2 rules go here");
  }

  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();
private:
  // We must provide this function to be instantiated
  Level* NextLevel() {
    return new Level3(m_pGameParent);
  };

  // Internal functions
  void ComputeNewPoints();
  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
};

// Level 1
class Dasher::GameMode::Level1 : public Dasher::GameMode::Level {
public:
  Level1(CDasherGameMode* pGameParent):Level(pGameParent),
   oldTime(0), m_dCurrentScore(0), oldNats(0.0), iErrors(0),
   m_iErrorSize(2048), bFixing(false), bMadeError(false)
  {
    // In Level 1, the flashing arrow is often on
    m_iOscillatorOn=1000;
    m_iOscillatorOff=2000;
    m_strLevel = "1";
  }

  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();

  std::string GetRules() {
    return std::string("Level 1 rules go here");
  }
 private:
  Level* NextLevel() {
    return new Level2(m_pGameParent);
  }

  void ComputeNewPoints();

  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
};

// The start level - this is not a real level.
class Dasher::GameMode::LevelStart : public Dasher::GameMode::Level {
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
