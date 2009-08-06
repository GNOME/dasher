#ifndef __dasher_preferences_dialogue_h__
#define __dasher_preferences_dialogue_h__

#include <gtk/gtk.h>

#include "DasherAppSettings.h"
#include "dasher_editor.h"

G_BEGIN_DECLS
#define TYPE_DASHER_PREFERENCES_DIALOGUE            (dasher_preferences_dialogue_get_type())
#define DASHER_PREFERENCES_DIALOGUE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_PREFERENCES_DIALOGUE, DasherPreferencesDialogue ))
#define DASHER_PREFERENCES_DIALOGUE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_PREFERENCES_DIALOGUE, DasherPreferencesDialogueClass ))
#define IS_DASHER_PREFERENCES_DIALOGUE(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_PREFERENCES_DIALOGUE))
#define IS_DASHER_PREFERENCES_DIALOGUE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_PREFERENCES_DIALOGUE))
#define DASHER_PREFERENCES_DIALOGUE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_PREFERENCES_DIALOGUE, DasherPreferencesDialogueClass))

typedef struct _DasherPreferencesDialogue DasherPreferencesDialogue;
typedef struct _DasherPreferencesDialogueClass DasherPreferencesDialogueClass;

struct _DasherPreferencesDialogue {
  GObject parent;
  gpointer private_data;
};


struct _DasherPreferencesDialogueClass {
  GObjectClass parent_class;
};

DasherPreferencesDialogue *dasher_preferences_dialogue_new(GtkBuilder *, DasherEditor *, DasherAppSettings *, GtkWindow *);
GType dasher_preferences_dialogue_get_type();
void dasher_preferences_dialogue_show(DasherPreferencesDialogue *pSelf, gint iPage);
void dasher_preferences_dialogue_handle_parameter_change(DasherPreferencesDialogue *pSelf, int iParameter);
G_END_DECLS

#endif
