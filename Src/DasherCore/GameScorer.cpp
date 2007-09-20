#include "DasherGameMode.h"
#include "GameScorer.h"
#include "GameStatistics.h"
#include <sstream>

using namespace Dasher::GameMode;

void Scorer::NewFrame(const GameInfo& gameInfo)
{
  if(m_bInPlay && !m_bPaused)
    m_vGameInfoData.push_back(gameInfo);
}

void Scorer::Reset()
{
  m_vGameInfoData.clear();
  m_Statsbreakdown.str("");
  m_bSentenceFinished=false;
  m_bInPlay = false;
}

void Scorer::Start()
{
  if(!m_bSentenceFinished)
    m_bInPlay=true;
  m_bPaused=false; 
}

void Scorer::Stop()
{
  m_bPaused=true;
}

void Scorer::SentenceFinished()
{
  m_bSentenceFinished = true;
  m_bInPlay = false;
  ComputeStats();
}

void Scorer::ComputeStats()
{
  const std::vector<GameInfo>& v = m_vGameInfoData;
  m_Statsbreakdown.str("");
  
  size_t cnt = v.size();
  if(cnt==0)
    return;

  myint GameInfo::* p = &GameInfo::m_iTargetY;
  m_stats.m1 = MemberNthMoment(1,v.begin(), v.end(), p, 0.0);
  m_stats.m2 = MemberNthMoment(2,v.begin(), v.end(), p, m_stats.m1);
  m_stats.m3 = MemberNthMoment(3,v.begin(), v.end(), p, m_stats.m1);
  m_stats.m4 = MemberNthMoment(4,v.begin(), v.end(), p, m_stats.m1);

  m_stats.dev = sqrt(m_stats.m2); // Standard Deviation
  m_stats.skew = m_stats.m3/(m_stats.m2*m_stats.dev); // Skewness
  m_stats.kurt = m_stats.m4 / (m_stats.m2*m_stats.m2) - 3.0; // Excess Kurtosis

  m_Statsbreakdown << "Time: " << GetTime() << "\n"
		   << "Samples: " << cnt << "\n"
		   << "Mean: " << m_stats.m1 << "\n"
		   << "StdDev: " << m_stats.dev << "\n"
		   << "Skew: " << m_stats.skew << "\n"
		   << "Kurt: " << m_stats.kurt << "\n";
}

std::string Scorer::GetBreakdown()
{
  ComputeStats();
  return m_Statsbreakdown.str();
}

double Scorer::GetNats()
{
  if(m_vGameInfoData.size()==0)
    return 0.0;
  else
    return m_vGameInfoData.back().m_dTotalNats-m_vGameInfoData.front().m_dTotalNats;

}

unsigned int Scorer::GetTime()
{
  if(m_vGameInfoData.size()==0)
    return 0;
  else
    return m_vGameInfoData.back().m_ulTime - m_vGameInfoData.front().m_ulTime;
}
