#ifndef __dasher_action_keyboard_h__
#define __dasher_action_keyboard_h__

#include "dasher_action.h"
#include "dasher_buffer_set.h"

G_BEGIN_DECLS
#define TYPE_DASHER_ACTION_KEYBOARD           (dasher_action_keyboard_get_type())
#define DASHER_ACTION_KEYBOARD(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_ACTION_KEYBOARD, DasherActionKeyboard ))
#define DASHER_ACTION_KEYBOARD_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_ACTION_KEYBOARD, DasherActionKeyboardClass ))
#define IS_DASHER_ACTION_KEYBOARD(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_ACTION_KEYBOARD))
#define IS_DASHER_ACTION_KEYBOARD_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_ACTION_KEYBOARD))
#define DASHER_ACTION_KEYBOARD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_ACTION_KEYBOARD, DasherActionKeyboardClass))

typedef struct _DasherActionKeyboard DasherActionKeyboard;
typedef struct _DasherActionKeyboardClass DasherActionKeyboardClass;

struct _DasherActionKeyboard {
  DasherAction parent;
  gpointer private_data;
};

struct _DasherActionKeyboardClass {
  DasherActionClass parent_class;
};

DasherActionKeyboard *dasher_action_keyboard_new(IDasherBufferSet *pBufferSet);
GType dasher_action_keyboard_get_type();

G_END_DECLS

#endif
