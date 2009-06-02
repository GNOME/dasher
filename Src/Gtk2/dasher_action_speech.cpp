// TODO: Make inclusion in build system conditional
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef GNOME_SPEECH

#include "dasher_action_speech.h"
#include "dasher_editor_internal.h"

#include <glib/gi18n.h>
#include <gnome-speech/gnome-speech.h>
#include <libbonobo.h>
#include <cstring>
//#include <libintl.h>
//#include <string.h>

/* Replace with glib i11n header */
//#define _(_x) gettext(_x)

struct _DasherActionSpeechPrivate {
  CORBA_Object rv;
  GNOME_Speech_Speaker speaker;
  GNOME_Speech_VoiceInfoList *voices;
  CORBA_Environment ev;
  char *szLast;
};

typedef struct _DasherActionSpeechPrivate DasherActionSpeechPrivate;

#define DASHER_ACTION_SPEECH_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_DASHER_ACTION_SPEECH, DasherActionSpeechPrivate))

G_DEFINE_TYPE(DasherActionSpeech, dasher_action_speech, TYPE_DASHER_ACTION);

static gboolean dasher_action_speech_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
static gboolean dasher_action_speech_preview(DasherAction *pSelf, DasherEditor *pEditor);
static const gchar *dasher_action_speech_get_name(DasherAction *pSelf);
static int dasher_action_speech_get_sub_count(DasherAction *pSelf);
static const gchar *dasher_action_speech_get_sub_name(DasherAction *pSelf, int iIdx);
static gboolean dasher_action_speech_activate(DasherAction *pSelf);
static gboolean dasher_action_speech_deactivate(DasherAction *pSelf);

static void 
dasher_action_speech_class_init(DasherActionSpeechClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherActionSpeechPrivate));

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_speech_execute;
  pDasherActionClass->preview = dasher_action_speech_preview;
  pDasherActionClass->get_name = dasher_action_speech_get_name;
  pDasherActionClass->get_sub_count = dasher_action_speech_get_sub_count;
  pDasherActionClass->get_sub_name = dasher_action_speech_get_sub_name;
  pDasherActionClass->activate = dasher_action_speech_activate;
  pDasherActionClass->deactivate = dasher_action_speech_deactivate;
}

static void 
dasher_action_speech_init(DasherActionSpeech *pDasherActionSpeech) {
  DasherActionSpeechPrivate *pPrivate = DASHER_ACTION_SPEECH_GET_PRIVATE(pDasherActionSpeech);

  pPrivate->szLast = NULL;
}

DasherActionSpeech *
dasher_action_speech_new() {
  DasherActionSpeech *pDasherControl;
  pDasherControl = (DasherActionSpeech *)(g_object_new(dasher_action_speech_get_type(), NULL));

  return pDasherControl;
}

static gboolean 
dasher_action_speech_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) {
  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = DASHER_ACTION_SPEECH_GET_PRIVATE(pSelf);

  const char *szData;

  switch(iIdx) {
  case 0:
    szData = dasher_editor_get_all_text(pEditor);
    break;
  case 1:
    szData = dasher_editor_get_new_text(pEditor);
    break;
  case 2:
    szData = pDasherActionSpeechPrivate->szLast;
    break;
  default:
    szData = NULL;
    break;
  }

  if(szData && (strlen(szData) > 0)) {
    // XXX PRLW - bolt-on
    if(pDasherActionSpeechPrivate->speaker == NULL)
      dasher_action_speech_activate(pSelf);
    if(pDasherActionSpeechPrivate->speaker == NULL)
      cerr << "Didn't activate speech\n";
    if(pDasherActionSpeechPrivate->speaker != NULL) {
      GNOME_Speech_Speaker_say(pDasherActionSpeechPrivate->speaker, szData, &(pDasherActionSpeechPrivate->ev));

      if(pDasherActionSpeechPrivate->szLast)
        g_free(pDasherActionSpeechPrivate->szLast);

      pDasherActionSpeechPrivate->szLast = g_strdup(szData);

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

static gboolean 
dasher_action_speech_preview(DasherAction *pSelf, DasherEditor *pEditor) {
  const gchar *szData = dasher_editor_get_all_text(pEditor);

  if(!szData)
    return false;

  const gchar *szWord = strrchr(szData, ' ') + 1;

  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = DASHER_ACTION_SPEECH_GET_PRIVATE(pSelf);
  
  if(pDasherActionSpeechPrivate->speaker != NULL) {
    GNOME_Speech_Speaker_say(pDasherActionSpeechPrivate->speaker, szWord, &(pDasherActionSpeechPrivate->ev));
  }

  return false;
}

static const gchar *
dasher_action_speech_get_name(DasherAction *pSelf) {
  return _("Speak");
}

static int 
dasher_action_speech_get_sub_count(DasherAction *pSelf) {
  return 3;
}

static const gchar *
dasher_action_speech_get_sub_name(DasherAction *pSelf, int iIdx) {
  switch(iIdx) {
  case 0:
    /* TRANSLATORS: Send all text to the text-to-speech engine. */
    return _("All");
  case 1:
    /* TRANSLATORS: Send most recently entered text to the text-to-speech engine. */
    return _("Last");
  case 2:
    /* TRANSLATORS: Repeat the last phrase sent to the text-to-speech engine. */
    return _("Repeat");
  default:
    return NULL;
  }
}

static gboolean 
dasher_action_speech_activate(DasherAction *pSelf) {
  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = DASHER_ACTION_SPEECH_GET_PRIVATE(pSelf);

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

static gboolean 
dasher_action_speech_deactivate(DasherAction *pSelf) {
  DasherActionSpeechPrivate *pDasherActionSpeechPrivate = DASHER_ACTION_SPEECH_GET_PRIVATE(pSelf);

  bonobo_object_release_unref(pDasherActionSpeechPrivate->speaker, NULL);
  CORBA_free(pDasherActionSpeechPrivate->voices);
  GNOME_Speech_SynthesisDriver_unref(pDasherActionSpeechPrivate->rv, &(pDasherActionSpeechPrivate->ev));
  CORBA_exception_free(&(pDasherActionSpeechPrivate->ev));
  
  return true;
}

#endif
