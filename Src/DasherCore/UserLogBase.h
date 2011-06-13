#ifndef __UserLogBase_h__
#define __UserLogBase_h__

#include "DasherTypes.h"
#include "DasherComponent.h"
#include "UserLogTrial.h" // Don't want to include this, but needed for event type enum
#include "Event.h"

#include <string>
#include <vector>
/// \defgroup Logging Logging routines
/// @{
class CUserLogBase : public Dasher::CDasherComponent {
 public:
  CUserLogBase(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore)
  : Dasher::CDasherComponent(pEventHandler, pSettingsStore), m_iNumDeleted(0) {};

  virtual void AddParam(const std::string& strName, const std::string& strValue, int iOptionMask = 0) = 0;
  virtual void AddParam(const std::string& strName, double dValue, int iOptionMask = 0) = 0;
  virtual void AddParam(const std::string& strName, int iValue, int iOptionMask = 0) = 0;
  virtual void StartWriting() = 0;
  virtual void StopWriting(float dNats) = 0;
  virtual void StopWriting() = 0;
  virtual void NewTrial() = 0;
  virtual void AddWindowSize(int iTop, int iLeft, int iBottom, int iRight) = 0;
  virtual void AddCanvasSize(int iTop, int iLeft, int iBottom, int iRight) = 0;
  virtual void AddMouseLocation(int iX, int iY, float dNats) = 0;
  virtual void AddMouseLocationNormalized(int iX, int iY, bool bStoreIntegerRep, float dNats) = 0;
  virtual void OutputFile() = 0;
  virtual void InitIsDone() = 0;
  virtual void SetOuputFilename(const std::string& strFilename = "") = 0;
  virtual int GetLogLevelMask() = 0;
  virtual void KeyDown(int iId, int iType, int iEffect) = 0;
  ///Watches output events to record symbols added/deleted
  virtual void HandleEvent(Dasher::CEvent *pEvent);
  ///Passes record of symbols added/deleted to AddSymbols/DeleteSymbols
  void FrameEnded();
protected:
  virtual void AddSymbols(Dasher::VECTOR_SYMBOL_PROB* pVectorNewSymbolProbs, eUserLogEventType iEvent = userLogEventMouse) = 0;
  virtual void DeleteSymbols(int iNumToDelete, eUserLogEventType iEvent = userLogEventMouse) = 0;  
private:
  std::vector<Dasher::SymbolProb> m_vAdded;
  int m_iNumDeleted;
};
/// @}

#endif
