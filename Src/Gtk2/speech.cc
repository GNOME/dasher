#include <iostream>
#include "speech.h"

#ifdef GNOME_SPEECH

#include <gnome-speech/gnome-speech.h>

GNOME_Speech_Speaker speaker;
GNOME_Speech_VoiceInfoList *voices;
CORBA_Environment ev;

void setup_speech() {

  Bonobo_ServerInfoList *servers;
  Bonobo_ServerInfo *info;
  CORBA_Object rv;

  CORBA_exception_init (&ev);

  servers = bonobo_activation_query (
	     "repo_ids.has ('IDL:GNOME/Speech/SynthesisDriver:0.2')",
	     NULL, &ev);

  info = &servers->_buffer[0];
  printf ("Atempting to activate %s.\n", info->iid);

  rv = bonobo_activation_activate_from_id (
	   (const Bonobo_ActivationID) info->iid,
	   0, NULL, &ev);

  CORBA_free (servers);

  voices = GNOME_Speech_SynthesisDriver_getAllVoices (rv, &ev);

  speaker = GNOME_Speech_SynthesisDriver_createSpeaker (rv,
                                                        &voices->_buffer[0],
                                                        &ev);
}

void teardown_speech() {

  bonobo_object_release_unref (speaker, NULL);
  CORBA_free (voices);
  CORBA_exception_free (&ev);

}

void SPEAK_DAMN_YOU(std::string *speech) {
  GNOME_Speech_Speaker_say (speaker, speech->c_str(), &ev);
}

#else
void setup_speech() {};
void teardown_speech() {};
void SPEAK_DAMN_YOU(std::string *speech) {};
#endif
