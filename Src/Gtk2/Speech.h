// TODO: Make inclusion in build system conditional
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef GNOME_SPEECH

#include <string>
#include <glib/gi18n.h>
#include <gnome-speech/gnome-speech.h>
#include <libbonobo.h>

class CSpeech {
public:
  CSpeech();
  ~CSpeech();
  bool Init();
  void Speak(const std::string &strText, bool bInterrupt);
private:
  GNOME_Speech_SynthesisDriver m_driver;
  GNOME_Speech_Speaker m_speaker;
  CORBA_Environment m_ev;
};

#endif
