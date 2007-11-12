#include "ActionSpeech.h"

CActionSpeech::CActionSpeech() {
  m_bActive = false;
}

// TODO: The next few methods are pretty much boiler plate - move into parent?
std::string CActionSpeech::GetName() {
  return "Speak";
}

bool CActionSpeech::Activate() {
  HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);

  if(hr!=S_OK)
    pVoice=0;

  if(pVoice == 0) {
    m_bActive = false;
    return false;
  }

  // TODO: Why is this needed?
  pVoice->Speak(L"",SPF_ASYNC,NULL);

  m_bActive = true;
  return true;
}

bool CActionSpeech::Deactivate() {
  if (pVoice!=NULL) {     
    pVoice->Release();
    pVoice=NULL;
  }

  m_bActive = false;
  return true;
}

bool CActionSpeech::GetActive() {
  return m_bActive;
}

bool CActionSpeech::Execute(const std::wstring &strText) {
  if(!m_bActive)
    return false;

  if(pVoice == 0)
    return false;

  pVoice->Speak(strText.c_str(), SPF_ASYNC, NULL);

  return true;
}

void CActionSpeech::Preview(const std::wstring &strText) {
  Execute(strText);
}