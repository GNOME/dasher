#ifndef _SPEECH_H_
#define _SPEECH_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string>

#ifdef GNOME_SPEECH
#include <gnome-speech/gnome-speech.h>
#include <libbonobo.h>
#endif
#ifdef HAVE_SPEECHD
#include <speech-dispatcher/libspeechd.h>
#endif

class CSpeech {
public:
  CSpeech();
  ~CSpeech();
  bool Init();
  void Speak(const std::string &strText, bool bInterrupt);
private:
#ifdef GNOME_SPEECH
  GNOME_Speech_SynthesisDriver m_driver;
  GNOME_Speech_Speaker m_speaker;
  CORBA_Environment m_ev;
#endif
#ifdef HAVE_SPEECHD
  SPDConnection *m_speaker;
#endif
};

#endif /* !_SPEECH_H_ */
