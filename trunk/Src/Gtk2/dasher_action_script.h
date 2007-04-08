#ifndef __dasher_action_script_h__
#define __dasher_action_script_h__

#include "dasher_action.h"

G_BEGIN_DECLS
#define TYPE_DASHER_ACTION_SCRIPT           (dasher_action_script_get_type())
#define DASHER_ACTION_SCRIPT(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_ACTION_SCRIPT, DasherActionScript ))
#define DASHER_ACTION_SCRIPT_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_ACTION_SCRIPT, DasherActionScriptClass ))
#define IS_DASHER_ACTION_SCRIPT(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_ACTION_SCRIPT))
#define IS_DASHER_ACTION_SCRIPT_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_ACTION_SCRIPT))
#define DASHER_ACTION_SCRIPT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_ACTION_SCRIPT, DasherActionScriptClass))

typedef struct _DasherActionScript DasherActionScript;
typedef struct _DasherActionScriptClass DasherActionScriptClass;

struct _DasherActionScript {
  DasherAction parent;
  gpointer private_data;
};

struct _DasherActionScriptClass {
  DasherActionClass parent_class;
};

DasherActionScript *dasher_action_script_new(const gchar *szPath, const gchar *szFilename);
GType dasher_action_script_get_type();

G_END_DECLS

#endif
