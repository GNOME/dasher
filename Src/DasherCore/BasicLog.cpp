#include "BasicLog.h"

#include "DasherInterfaceBase.h"

#include <cmath>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

using namespace Dasher;

CBasicLog::CBasicLog(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pIntf)
: CUserLogBase(pIntf), CSettingsUser(pCreateFrom) {
  m_iSymbolCount = 0;
  m_bStarted = false;
}

CBasicLog::~CBasicLog() {
  EndTrial();
}

void CBasicLog::StartWriting() {
  if(!m_bStarted) {
    StartTrial();
    m_bStarted = true;
  }
}

void CBasicLog::StopWriting(float dNats) {
  m_dBits += dNats / log(2.0);
}

void CBasicLog::AddSymbols(Dasher::VECTOR_SYMBOL_PROB* pVectorNewSymbolProbs, eUserLogEventType iEvent) {
  m_iSymbolCount += pVectorNewSymbolProbs->size();
}

void CBasicLog::DeleteSymbols(int iNumToDelete, eUserLogEventType iEvent) {
  m_iSymbolCount -= iNumToDelete;
}

void CBasicLog::NewTrial() {
  EndTrial();
}

void CBasicLog::KeyDown(int iId, int iType, int iEffect) {
  ++m_iKeyCount;
}

void CBasicLog::StartTrial() {
  m_iSymbolCount = 0;
  m_iKeyCount = 0;
  m_dBits = 0.0;
  m_strStartDate = GetDateStamp();
  m_iInitialRate = GetLongParameter(LP_MAX_BITRATE);
}

void CBasicLog::EndTrial() {
  if(!m_bStarted)
    return;

  std::string strFileName(CFileLogger::GetFullFilenamePath("dasher_basic.log"));

  std::ofstream oFile;
  oFile.open(strFileName.c_str(), ios::out | ios::app);

  oFile << "\"" << m_strStartDate << "\":\"" << GetDateStamp() << "\":" << m_iSymbolCount << ":" << m_dBits << ":" << m_iKeyCount << ":" << m_iInitialRate / 100.0 << ":" << GetLongParameter(LP_MAX_BITRATE) / 100.0 << ":\"" << GetStringParameter(SP_INPUT_FILTER) << "\":\"" << GetStringParameter(SP_ALPHABET_ID) << "\"" << std::endl;

  oFile.close();

  m_bStarted = false;
}

std::string CBasicLog::GetDateStamp() {
  char* szTimeLine = NULL;
  time_t t;

  t = time(NULL);
  szTimeLine = ctime(&t);

  return std::string(szTimeLine).substr(0, 24);
}
