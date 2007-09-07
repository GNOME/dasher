#ifndef __GameLevel_h__
#define __GameLevel_h__

#include "DasherGameMode.h"
#include <sstream>
// Base class to represent different levels in the Dasher Game.

class Dasher::CDasherGameMode::Level {
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
    {}
  virtual ~Level(){}
  virtual void DoGameLogic()=0;
  virtual int GetCurrentScore()=0;
  virtual void SentenceFinished()=0;
  virtual void Reset()=0;
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
  TargetInfo& m_Target;
  Scorer*& m_pScorer;
  CDasherModel*& m_pModel;
};

class Level1 : public Dasher::CDasherGameMode::Level {
 public:
  Level1(CDasherGameMode* pGameParent):Level(pGameParent),
    oldTime(0), oldNats(0.0), m_dCurrentScore(0), m_iErrorSize(2048),
    bFixing(false), bMadeError(false), iErrors(0), m_dSentenceScore(0.0),
    m_dLevelScore(0.0) {m_iOscillatorOn=500; m_iOscillatorOff=2000;}
  void DoGameLogic();
  int GetCurrentScore();
  void SentenceFinished();
  void Reset();
 private:
  void ComputeNewPoints();
  unsigned int oldTime;
  double m_dCurrentScore;
  double oldNats;
  double m_dLevelScore;
  double m_dSentenceScore;
  int iErrors;
  const myint m_iErrorSize;
  bool bFixing;
  bool bMadeError;
};


class Level2 : public Dasher::CDasherGameMode::Level {
 public:
  Level2(CDasherGameMode* pGameParent):Level(pGameParent) {m_iOscillatorOn=200; m_iOscillatorOff=2500;}
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

class Level3 : public Dasher::CDasherGameMode::Level {
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

#endif
