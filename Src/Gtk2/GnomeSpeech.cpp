// Inclusion in build system conditional on USE_GNOMESPEECH
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Speech.h"
#include <glib/gi18n.h>

CSpeech::CSpeech() : m_speaker(NULL) {
  CORBA_exception_init(&m_ev);
}

CSpeech::~CSpeech() {
  if (m_speaker) {
    bonobo_object_release_unref(m_speaker, &m_ev);
    GNOME_Speech_SynthesisDriver_unref(m_driver, &m_ev);
  }
  CORBA_exception_free(&m_ev);
}


bool CSpeech::Init() {
  if (m_speaker) return true;

  Bonobo_ServerInfoList *servers = bonobo_activation_query("repo_ids.has_one(['IDL:GNOME/Speech/SynthesisDriver:0.2','IDL:GNOME/Speech/SynthesisDriver:0.3'])", NULL, &m_ev);

  if(servers->_length == 0) {
    g_warning(_("Unable to initialize speech support\n"));
    return false;
  }

  for(unsigned int i = 0; i < servers->_length; i++) {
    
    Bonobo_ServerInfo *info = &servers->_buffer[i];

    m_driver = bonobo_activation_activate_from_id((const Bonobo_ActivationID)info->iid, 0, NULL, &m_ev);
    if(BONOBO_EX(&m_ev)) continue;

    GNOME_Speech_SynthesisDriver_driverInit(m_driver, &m_ev);

    GNOME_Speech_VoiceInfoList *voices = GNOME_Speech_SynthesisDriver_getAllVoices(m_driver, &m_ev);

    if(voices == NULL || BONOBO_EX(&m_ev) || voices->_length == 0) {
      CORBA_free(voices);
      GNOME_Speech_SynthesisDriver_unref(m_driver, &m_ev);
      continue;
    }

    m_speaker = GNOME_Speech_SynthesisDriver_createSpeaker(m_driver, &voices->_buffer[0], &m_ev);
    CORBA_free(voices);
    break;
  }

  CORBA_free(servers);

  if (m_speaker) {
    GNOME_Speech_ParameterList *list = GNOME_Speech_Speaker_getSupportedParameters(m_speaker, &m_ev);
    if(BONOBO_EX(&m_ev) || list->_length == 0) {
      g_warning(_("Warning: unable to set speech parameters\n"));
    } else {
      for(unsigned i = 0; i < list->_length; i++) {
        GNOME_Speech_Parameter *p = &(list->_buffer[i]);
        if(!strcmp(p->name, "rate")) {
          GNOME_Speech_Speaker_setParameterValue(m_speaker, p->name, 200.0, &m_ev);
        }
      }
    }
    CORBA_free(list);
  } else
    g_warning(_("Unable to initialize voices\n"));

  return m_speaker != NULL;
}

void CSpeech::Speak(const std::string &strText, bool bInterrupt) {
    if (Init())
      GNOME_Speech_Speaker_say(m_speaker, strText.c_str(), &m_ev);
}
