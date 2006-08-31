#ifndef __UserLogBase_h__
#define __UserLogBase_h__

#include "DasherComponent.h"

#include <string>

class CUserLogBase : public Dasher::CDasherComponent {
 public:
  CUserLogBase(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore) : Dasher::CDasherComponent(pEventHandler, pSettingsStore) {};
  virtual ~CUserLogBase() {};

  virtual void AddParam(const string& strName, const string& strValue, int iOptionMask = 0) {};
  virtual void AddParam(const string& strName, double dValue, int iOptionMask = 0) = 0;
  virtual void AddParam(const string& strName, int iValue, int iOptionMask = 0) = 0;
  virtual void StartWriting() = 0;
  virtual void StopWriting(float dNats) = 0;
  virtual void StopWriting() = 0;
  virtual void AddSymbols(Dasher::VECTOR_SYMBOL_PROB* pVectorNewSymbolProbs, eUserLogEventType iEvent = userLogEventMouse) = 0;
  virtual void DeleteSymbols(int iNumToDelete, eUserLogEventType iEvent = userLogEventMouse) = 0;    
  virtual void NewTrial() = 0;
  virtual void AddWindowSize(int iTop, int iLeft, int iBottom, int iRight) = 0;
  virtual void AddCanvasSize(int iTop, int iLeft, int iBottom, int iRight) = 0;
  virtual void AddMouseLocation(int iX, int iY, float dNats) = 0;
  virtual void AddMouseLocationNormalized(int iX, int iY, bool bStoreIntegerRep, float dNats) = 0;
  virtual void OutputFile() = 0;
  virtual void SetAlphabetPtr(Dasher::CAlphabet* pAlphabet = 0) = 0;
  virtual void InitIsDone() = 0;
  virtual void SetOuputFilename(const string& strFilename = "") = 0;
  virtual int GetLogLevelMask() = 0;
  virtual void KeyDown(int iId, int iType, int iEffect) = 0;
  virtual void HandleEvent(Dasher::CEvent* pEvent) = 0;
};

#endif
