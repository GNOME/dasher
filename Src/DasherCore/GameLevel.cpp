#include "GameLevel.h"
#include "GameScorer.h"

using namespace Dasher::GameMode;
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
    m_bDrawHelperArrow = true;
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
  return int(m_iLevelScore+m_dSentenceScore);
}

void Level1::Reset()
{
  oldNats=m_pScorer->GetNats();
  iErrors=0;
  m_dSentenceScore=0.0;
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
  int iSentenceScore = int(m_dSentenceScore);

  int bonusFactor = std::max(2-iErrors,0);
  int bonusPoints = iSentenceScore*bonusFactor;

  m_strPerformance.str("");  
  m_strPerformance << "Well done!\n" << "This sentence was worth "
                   << iSentenceScore << " points.\n";
  if(bonusPoints!=0)
    {
      m_strPerformance << "You also get a bonus of " << bonusPoints << " for only making only "
		       << iErrors << " errors.\n";
    }
  int speed = (int)(14426*m_pScorer->GetNats()/double(m_pScorer->GetTime()));
  int speedBonus = std::max((speed-15)*5,0);
  if(speedBonus != 0)
    m_strPerformance << "You also get a writing speed bonus of " << speedBonus << "\n";
  iSentenceScore+=bonusPoints+speedBonus;
  m_iLevelScore+=iSentenceScore;
  m_dSentenceScore=0.0;
  m_bIsCompleted = (m_iLevelScore>450);
  Reset();
}

// ---------Level 2--------------

void Level2::DoGameLogic()
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
    m_bDrawHelperArrow=true;
  else
    m_bDrawHelperArrow = false;
  
  if((m_bOscillator || m_iUserX > m_iCrossX))
    m_bDrawTargetArrow=true;
  else
    m_bDrawTargetArrow=false;
}

int Level2::GetCurrentScore()
{
  ComputeNewPoints();
  return int(m_iLevelScore+m_dSentenceScore);
}

void Level2::Reset()
{
  oldNats=m_pScorer->GetNats();
  iErrors=0;
  m_dSentenceScore=0.0;
}


void Level2::ComputeNewPoints()
{
  double dNats = m_pScorer->GetNats();
  double extraPoints = (dNats-oldNats);
  oldNats = dNats;
  m_dSentenceScore+=extraPoints;
}

void Level2::SentenceFinished()
{
  ComputeNewPoints();
  int iSentenceScore = int(m_dSentenceScore);

  int bonusFactor = std::max(2-iErrors,0);
  int bonusPoints = iSentenceScore*bonusFactor;

  m_strPerformance.str("");  
  m_strPerformance << "Well done!\n" << "This sentence was worth "
                   << iSentenceScore << " points.\n";
  if(bonusPoints!=0)
    {
      m_strPerformance << "You also get a bonus of " << bonusPoints << " for only making only "
		       << iErrors << " errors.\n";
    }
  int speed = (int)(14426*m_pScorer->GetNats()/double(m_pScorer->GetTime()));
  int speedBonus = std::max((speed-15)*5,0);
  if(speedBonus != 0)
    m_strPerformance << "You also get a writing speed bonus of " << speedBonus << "\n";
  iSentenceScore+=bonusPoints+speedBonus;
  m_iLevelScore+=iSentenceScore;
  m_dSentenceScore=0.0;
  m_bIsCompleted = (m_iLevelScore>450);
  Reset();
}
