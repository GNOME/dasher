#ifndef _SPEECH_H_
#define _SPEECH_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string>

#ifdef HAVE_SPEECHD
#include <speech-dispatcher/libspeechd.h>
#endif

class CSpeech {
public:
  CSpeech();
  ~CSpeech();
  bool Init();
  void Speak(const std::string &strText, bool bInterrupt, const std::string &lang);
private:
#ifdef HAVE_SPEECHD
  SPDConnection *m_speaker;
#endif
};

#endif /* !_SPEECH_H_ */
