#ifndef __GameScorer_h__
#define __GameScorer_h__

#include "DasherGameMode.h"
#include <sstream>

class Dasher::GameMode::Scorer {
 public:
  Scorer(): m_bInPlay(false), m_bSentenceFinished(true){}
  ~Scorer(){}
  struct Stats {
    double sum, m1, m2, m3, m4, dev, skew, kurt;
  };
  struct GameInfo {
    GameInfo(unsigned long time, myint iMouseX, myint iMouseY, myint iTargetY, double dTotalNats):
      m_ulTime(time), m_iMouseX(iMouseX), m_iMouseY(iMouseY), m_iTargetY(iTargetY), m_dTotalNats(dTotalNats){}
    unsigned long m_ulTime;
    myint m_iMouseX;
    myint m_iMouseY;
    myint m_iTargetY;
    double m_dTotalNats;
  };
  void SentenceFinished();
  unsigned int GetTime();
  double GetNats();

  std::string GetBreakdown();
  void NewFrame(const GameInfo&);
  void Start();
  void Stop();
  void Reset();

 private:
  void ComputeStats();
  bool m_bInPlay;
  bool m_bPaused;
  bool m_bSentenceFinished;

  std::vector<myint> m_vTargetData;
  std::vector<GameInfo> m_vGameInfoData;
  std::ostringstream m_Statsbreakdown;

  Stats m_stats;
};
#endif
