#include "speech.h"

#ifdef GNOME_SPEECH

#include <gnome-speech/gnome-speech.h>
#include <bonobo/bonobo-exception.h>
#include <libintl.h>
#define _(_x) gettext(_x)

GNOME_Speech_Speaker speaker;
GNOME_Speech_VoiceInfoList *voices;
CORBA_Environment ev;

void setup_speech() {

  Bonobo_ServerInfoList *servers;
  Bonobo_ServerInfo *info;
  CORBA_Object rv=NULL;

  servers = bonobo_activation_query ("repo_ids.has_one(['IDL:GNOME/Speech/SynthesisDriver:0.2','IDL:GNOME/Speech/SynthesisDriver:0.3'])", NULL, &ev);

  if (servers->_length==0) {
    speaker=NULL;
    printf(_("Unable to initialize speech support\n"));
    fflush(stdout);
    return;
  }

  for (unsigned int i=0; i<servers->_length; i++) 
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

  GNOME_Speech_SynthesisDriver_driverInit (rv, &ev);

  voices = GNOME_Speech_SynthesisDriver_getAllVoices (rv, &ev);

  if (voices==NULL || BONOBO_EX (&ev) || voices->_length==0) {
    printf(_("Unable to initialize voices"));
    speaker=NULL;
    return;
  }

  speaker = GNOME_Speech_SynthesisDriver_createSpeaker (rv,
                                                        &voices->_buffer[0],
                                                        &ev);

  GNOME_Speech_ParameterList *list;
  list = GNOME_Speech_Speaker_getSupportedParameters (speaker, &ev);
  if (BONOBO_EX (&ev) || list->_length == 0)
    {
      if (!BONOBO_EX (&ev))
	CORBA_free (list);
      printf(_("Warning: unable to set speech parameters\n"));
      return;
    }
  for (unsigned i = 0; i < list->_length; i++)
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
  if (speaker!=NULL) {
    GNOME_Speech_Speaker_say (speaker, speech->c_str(), &ev);
  }
}

#else
void setup_speech() {};
void teardown_speech() {};
void SPEAK_DAMN_YOU(std::string *speech) {};
#endif
