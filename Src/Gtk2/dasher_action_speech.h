#ifndef __dasher_action_speech_h__
#define __dasher_action_speech_h__

#include "dasher_action.h"

G_BEGIN_DECLS
#define TYPE_DASHER_ACTION_SPEECH           (dasher_action_speech_get_type())
#define DASHER_ACTION_SPEECH(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_ACTION_SPEECH, DasherActionSpeech ))
#define DASHER_ACTION_SPEECH_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_ACTION_SPEECH, DasherActionSpeechClass ))
#define IS_DASHER_ACTION_SPEECH(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_ACTION_SPEECH))
#define IS_DASHER_ACTION_SPEECH_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_ACTION_SPEECH))
#define DASHER_ACTION_SPEECH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_ACTION_SPEECH, DasherActionSpeechClass))

typedef struct _DasherActionSpeech DasherActionSpeech;
typedef struct _DasherActionSpeechClass DasherActionSpeechClass;

struct _DasherActionSpeech {
  DasherAction parent;
  gpointer private_data;
};

struct _DasherActionSpeechClass {
  DasherActionClass parent_class;
};

DasherActionSpeech *dasher_action_speech_new();
GType dasher_action_speech_get_type();

G_END_DECLS

#endif
