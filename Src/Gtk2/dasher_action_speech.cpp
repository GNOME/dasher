// TODO: Make inclusion in build system conditional
#include "config.h"
#ifdef GNOME_SPEECH

#include "dasher_action_speech.h"

#include <gnome-speech/gnome-speech.h>
#include <bonobo/bonobo-exception.h>
#include <libintl.h>
#include <string.h>

#define _(_x) gettext(_x)

struct _DasherActionSpeechPrivate {
  CORBA_Object rv;
  GNOME_Speech_Speaker speaker;
  GNOME_Speech_VoiceInfoList *voices;
  CORBA_Environment ev;
};

typedef struct _DasherActionSpeechPrivate DasherActionSpeechPrivate;


static void dasher_action_speech_class_init(DasherActionSpeechClass *pClass);
static void dasher_action_speech_init(DasherActionSpeech *pActionSpeech);
static void dasher_action_speech_destroy(GObject *pObject);
static gboolean dasher_action_speech_execute(DasherAction *pSelf, const gchar *szData);
static const gchar *dasher_action_speech_get_name(DasherAction *pSelf);
gboolean dasher_action_speech_activate(DasherAction *pSelf);
gboolean dasher_action_speech_deactivate(DasherAction *pSelf);

GType dasher_action_speech_get_type() {

  static GType dasher_action_speech_type = 0;

  if(!dasher_action_speech_type) {
    static const GTypeInfo dasher_action_speech_info = {
      sizeof(DasherActionSpeechClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_action_speech_class_init,
      NULL,
      NULL,
      sizeof(DasherActionSpeech),
      0,
      (GInstanceInitFunc) dasher_action_speech_init,
      NULL
    };

    dasher_action_speech_type = g_type_register_static(TYPE_DASHER_ACTION, "DasherActionSpeech", &dasher_action_speech_info, static_cast < GTypeFlags > (0));
  }

  return dasher_action_speech_type;
}

static void dasher_action_speech_class_init(DasherActionSpeechClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_action_speech_destroy;

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_speech_execute;
  pDasherActionClass->get_name = dasher_action_speech_get_name;
  pDasherActionClass->activate = dasher_action_speech_activate;
  pDasherActionClass->deactivate = dasher_action_speech_deactivate;
}

static void dasher_action_speech_init(DasherActionSpeech *pDasherControl) {
  pDasherControl->private_data = new DasherActionSpeechPrivate;
}

static void dasher_action_speech_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherActionSpeech *dasher_action_speech_new() {
  DasherActionSpeech *pDasherControl;
  pDasherControl = (DasherActionSpeech *)(g_object_new(dasher_action_speech_get_type(), NULL));

  return pDasherControl;
}

static gboolean dasher_action_speech_execute(DasherAction *pSelf, const gchar *szData) {
  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = (DasherActionSpeechPrivate *)(((DasherActionSpeech *)pSelf)->private_data);

  if(szData && (strlen(szData) > 0)) {
    if(pDasherActionSpeechPrivate->speaker != NULL) {
      GNOME_Speech_Speaker_say(pDasherActionSpeechPrivate->speaker, szData, &(pDasherActionSpeechPrivate->ev));
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

static const gchar *dasher_action_speech_get_name(DasherAction *pSelf) {
  return "Speak";
}

gboolean dasher_action_speech_activate(DasherAction *pSelf) {
  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = (DasherActionSpeechPrivate *)(((DasherActionSpeech *)pSelf)->private_data);
  
  pDasherActionSpeechPrivate->rv = 0;
  Bonobo_ServerInfoList *servers;
  Bonobo_ServerInfo *info;

  servers = bonobo_activation_query("repo_ids.has_one(['IDL:GNOME/Speech/SynthesisDriver:0.2','IDL:GNOME/Speech/SynthesisDriver:0.3'])", NULL, &(pDasherActionSpeechPrivate->ev));

  if(servers->_length == 0) {
    pDasherActionSpeechPrivate->speaker = NULL;
    printf(_("Unable to initialize speech support\n"));
    fflush(stdout);
    return false;
  }

  for(unsigned int i = 0; i < servers->_length; i++) {
    CORBA_exception_init(&(pDasherActionSpeechPrivate->ev));

    info = &servers->_buffer[i];

    pDasherActionSpeechPrivate->rv = bonobo_activation_activate_from_id((const Bonobo_ActivationID)info->iid, 0, NULL, &(pDasherActionSpeechPrivate->ev));
    if(!BONOBO_EX(&(pDasherActionSpeechPrivate->ev))) {
      break;
    }
  }

  CORBA_free(servers);

  GNOME_Speech_SynthesisDriver_driverInit(pDasherActionSpeechPrivate->rv, &(pDasherActionSpeechPrivate->ev));

  pDasherActionSpeechPrivate->voices = GNOME_Speech_SynthesisDriver_getAllVoices(pDasherActionSpeechPrivate->rv, &(pDasherActionSpeechPrivate->ev));

  if(pDasherActionSpeechPrivate->voices == NULL || BONOBO_EX(&(pDasherActionSpeechPrivate->ev)) || pDasherActionSpeechPrivate->voices->_length == 0) {
    g_warning(_("Unable to initialize voices\n"));
    pDasherActionSpeechPrivate->speaker = NULL;
    return false;
  }

  pDasherActionSpeechPrivate->speaker = GNOME_Speech_SynthesisDriver_createSpeaker(pDasherActionSpeechPrivate->rv, &pDasherActionSpeechPrivate->voices->_buffer[0], &(pDasherActionSpeechPrivate->ev));

  GNOME_Speech_ParameterList *list;
  list = GNOME_Speech_Speaker_getSupportedParameters(pDasherActionSpeechPrivate->speaker, &(pDasherActionSpeechPrivate->ev));
  if(BONOBO_EX(&(pDasherActionSpeechPrivate->ev)) || list->_length == 0) {
    if(!BONOBO_EX(&(pDasherActionSpeechPrivate->ev)))
      CORBA_free(list);
    g_warning(_("Warning: unable to set speech parameters\n"));
    return false;
  }
  for(unsigned i = 0; i < list->_length; i++) {
    GNOME_Speech_Parameter *p = &(list->_buffer[i]);
    if(!strcmp(p->name, "rate")) {
      GNOME_Speech_Speaker_setParameterValue(pDasherActionSpeechPrivate->speaker, p->name, 200.0, &(pDasherActionSpeechPrivate->ev));
    }
  }
  CORBA_free(list);

  return true;
}

gboolean dasher_action_speech_deactivate(DasherAction *pSelf) {
  g_message("Shutting down speech synthesis");

  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = (DasherActionSpeechPrivate *)(((DasherActionSpeech *)pSelf)->private_data);

  bonobo_object_release_unref(pDasherActionSpeechPrivate->speaker, NULL);
  CORBA_free(pDasherActionSpeechPrivate->voices);
  GNOME_Speech_SynthesisDriver_unref(pDasherActionSpeechPrivate->rv, &(pDasherActionSpeechPrivate->ev));
  CORBA_exception_free(&(pDasherActionSpeechPrivate->ev));
  
  return true;
}

#endif
