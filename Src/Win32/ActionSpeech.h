#ifndef __ActionSpeech_h__
#define __ActionSpeech_h__

#include <sapi.h>

#include "DasherAction.h"

class CActionSpeech : public CDasherAction {
public:
  CActionSpeech();

  // Inherited methods
  virtual std::string GetName();
  virtual bool Activate();
  virtual bool Deactivate();
  virtual bool GetActive();
  virtual bool Execute(const std::wstring &strText);
  virtual void Preview(const std::wstring &strText);

private:
  bool m_bActive;

  // Pointer to the voice object
  ISpVoice *pVoice;
};

#endif