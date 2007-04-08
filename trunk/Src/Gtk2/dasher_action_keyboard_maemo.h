#ifndef __dasher_action_keyboard_maemo_h__
#define __dasher_action_keyboard_maemo_h__

#include "dasher_action.h"

G_BEGIN_DECLS
#define TYPE_DASHER_ACTION_KEYBOARD_MAEMO           (dasher_action_keyboard_maemo_get_type())
#define DASHER_ACTION_KEYBOARD_MAEMO(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_ACTION_KEYBOARD_MAEMO, DasherActionKeyboardMaemo ))
#define DASHER_ACTION_KEYBOARD_MAEMO_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_ACTION_KEYBOARD_MAEMO, DasherActionKeyboardMaemoClass ))
#define IS_DASHER_ACTION_KEYBOARD_MAEMO(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_ACTION_KEYBOARD_MAEMO))
#define IS_DASHER_ACTION_KEYBOARD_MAEMO_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_ACTION_KEYBOARD_MAEMO))
#define DASHER_ACTION_KEYBOARD_MAEMO_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_ACTION_KEYBOARD_MAEMO, DasherActionKeyboardMaemoClass))

typedef struct _DasherActionKeyboardMaemo DasherActionKeyboardMaemo;
typedef struct _DasherActionKeyboardMaemoClass DasherActionKeyboardMaemoClass;

struct _DasherActionKeyboardMaemo {
  DasherAction parent;
  gpointer private_data;
};

struct _DasherActionKeyboardMaemoClass {
  DasherActionClass parent_class;
};

DasherActionKeyboardMaemo *dasher_action_keyboard_maemo_new();
GType dasher_action_keyboard_maemo_get_type();

G_END_DECLS

#endif
