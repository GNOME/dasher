#include "GameLevel.h"
#include "GameScorer.h"

void Level1::DoGameLogic()
{
  if(m_Target.iTargetY < 0-m_iErrorSize || m_iMaxY +m_iErrorSize< m_Target.iTargetY)
    {
      m_bDrawHelperArrow = true;
      if(bMadeError==false) iErrors++;
      bMadeError=true;
    }
  else if(bMadeError && !bFixing)
    {
      new DelaySet(m_pGameParent,1000,&bFixing,false);
      new DelaySet(m_pGameParent,1000,&bMadeError,false);
      bFixing = true;
      m_bDrawHelperArrow=true;
    }
  else if(bMadeError)
    m_bDrawHelperArrow=false;//true;
  else
    m_bDrawHelperArrow = false;
  
  if((m_bOscillator || m_iUserX > m_iCrossX))
    m_bDrawTargetArrow=true;
  else
    m_bDrawTargetArrow=false;
}

int Level1::GetCurrentScore()
{
  ComputeNewPoints();

  return int(m_dLevelScore+m_dSentenceScore);
}

void Level1::Reset()
{
  oldNats=m_pScorer->GetNats();
  iErrors=0;
  m_dSentenceScore=0.0;
  m_strPerformance.str("");
}

void Level1::ComputeNewPoints()
{
  double dNats = m_pScorer->GetNats();
  double extraPoints = (dNats-oldNats);
  oldNats = dNats;
  m_dSentenceScore+=extraPoints;
}

void Level1::SentenceFinished()
{
  ComputeNewPoints();
  int bonusFactor = std::max(2-iErrors,0);
  int bonusPoints = int(m_dSentenceScore)*bonusFactor;

  
  m_strPerformance << "Well done!\n" << "This sentence was worth " << int(m_dSentenceScore) << " points.\n";
  if(bonusPoints!=0)
    {
      m_strPerformance << "You also get a bonus of " << bonusPoints << " for only making only "
		       << iErrors << " errors.\n";
    }
  int speed = 14426*m_pScorer->GetNats()/double(m_pScorer->GetTime());
  int speedBonus = std::max((speed-15)*5,0);
  
  m_strPerformance << "You also get a writing speed bonus of " << speedBonus << "\n";
  m_dSentenceScore+=bonusPoints+speedBonus;
  m_dLevelScore+=m_dSentenceScore;
  m_dSentenceScore=0.0;
  
  if(bonusPoints==2)
    true;
  else
    Reset();
}
