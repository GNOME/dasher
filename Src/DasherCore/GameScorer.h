#ifndef __GameScorer_h__
#define __GameScorer_h__

#include "DasherGameMode.h"
#include <sstream>

class Dasher::CDasherGameMode::Scorer {
 public:
  Scorer(): m_bInPlay(false){}
  ~Scorer(){}
  struct Stats {
    double sum, m1, m2, m3, m4, dev, skew, kurt;
  };
  void SentenceFinished(unsigned long time);
  int GetScore();
  std::string GetBreakdown();
  void NewFrame(unsigned long time, myint iMouseX, myint iMouseY, myint iTargetY);
  void Start(unsigned long time);
  void Stop(unsigned long time);
  void Reset();
 private:
  void CalculateStats();
  bool m_bInPlay;
  bool m_bPaused;
  int samples;
  double runningMean;
  std::vector<myint> TargetData;
  std::ostringstream m_Statsbreakdown;
  unsigned long m_iStartTime;
  unsigned long m_iStopTime;
  Stats m_stats;
};
#endif
