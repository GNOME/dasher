#include "speech.h"

#ifdef GNOME_SPEECH

#include <gnome-speech/gnome-speech.h>
#include <bonobo/bonobo-exception.h>

GNOME_Speech_Speaker speaker;
GNOME_Speech_VoiceInfoList *voices;
CORBA_Environment ev;

void setup_speech() {

  Bonobo_ServerInfoList *servers;
  Bonobo_ServerInfo *info;
  CORBA_Object rv;

  servers = bonobo_activation_query (
	     "repo_ids.has ('IDL:GNOME/Speech/SynthesisDriver:0.2')",
	     NULL, &ev);

  for (int i=0; i<servers->_length; i++) 
    {
      CORBA_exception_init (&ev);

      info = &servers->_buffer[i];

      rv = bonobo_activation_activate_from_id (
					       (const Bonobo_ActivationID) info->iid,
					       0, NULL, &ev);
      if (!BONOBO_EX (&ev)) {
	break;
      }
    }

  CORBA_free (servers);

  voices = GNOME_Speech_SynthesisDriver_getAllVoices (rv, &ev);

  speaker = GNOME_Speech_SynthesisDriver_createSpeaker (rv,
                                                        &voices->_buffer[0],
                                                        &ev);

  GNOME_Speech_ParameterList *list;
  int i;
  list = GNOME_Speech_Speaker_getSupportedParameters (speaker, &ev);
  if (BONOBO_EX (&ev) || list->_length == 0)
    {
      if (!BONOBO_EX (&ev))
	CORBA_free (list);
      printf("Warning: unable to set speech parameters\n");
      return;
    }
  for (i = 0; i < list->_length; i++)
    {
      GNOME_Speech_Parameter *p = &(list->_buffer[i]);
      if (!strcmp (p->name, "rate")) {
	GNOME_Speech_Speaker_setParameterValue (speaker, p->name, 200.0, &ev);
      }
    }
  CORBA_free (list);
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
