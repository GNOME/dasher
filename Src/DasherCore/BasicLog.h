#ifndef __BasicLog_h__
#define __BasicLog_h__

#include "UserLogBase.h"
#include "SettingsStore.h"

/// \ingroup Logging
/// @{
class CBasicLog : public CUserLogBase, public Dasher::CSettingsUser {
 public:
  CBasicLog(Dasher::CSettingsUser *pCreateFrom, Dasher::CDasherInterfaceBase *pIntf);
  ~CBasicLog();

  virtual void AddParam(const string& strName, const string& strValue, int iOptionMask = 0) {};
  virtual void AddParam(const string& strName, double dValue, int iOptionMask = 0) {};
  virtual void AddParam(const string& strName, int iValue, int iOptionMask = 0) {};
  virtual void StartWriting();
  virtual void StopWriting(float dNats);
  virtual void StopWriting() {};
  virtual void AddSymbols(Dasher::VECTOR_SYMBOL_PROB* pVectorNewSymbolProbs, eUserLogEventType iEvent = userLogEventMouse);
  virtual void DeleteSymbols(int iNumToDelete, eUserLogEventType iEvent = userLogEventMouse);    
  virtual void NewTrial();
  virtual void AddWindowSize(int iTop, int iLeft, int iBottom, int iRight) {};
  virtual void AddCanvasSize(int iTop, int iLeft, int iBottom, int iRight) {};
  virtual void AddMouseLocation(int iX, int iY, float dNats) {};
  virtual void AddMouseLocationNormalized(int iX, int iY, bool bStoreIntegerRep, float dNats) {};
  virtual void OutputFile() {};
  virtual void InitIsDone() {};
  virtual void SetOuputFilename(const string& strFilename = "") {};
  virtual int GetLogLevelMask() {return 0;};
  virtual void KeyDown(int iId, int iType, int iEffect);

 private:
  void StartTrial();
  void EndTrial();
  std::string GetDateStamp();

  bool m_bStarted;
  int m_iSymbolCount;
  int m_iKeyCount;
  int m_iInitialRate;
  double m_dBits;
  std::string m_strStartDate;
};
/// @}

#endif
