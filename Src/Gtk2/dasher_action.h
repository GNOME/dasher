#ifndef __dasher_action_h__
#define __dasher_action_h__

// TODO: This should be an interface rather than an instantiable class - look for macros etc in GTK code which do this

typedef struct _DasherEditor DasherEditor;

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define TYPE_DASHER_ACTION            (dasher_action_get_type())
#define DASHER_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_ACTION, DasherAction ))
#define DASHER_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_ACTION, DasherActionClass ))
#define IS_DASHER_ACTION(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_ACTION))
#define IS_DASHER_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_ACTION))
#define DASHER_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_ACTION, DasherActionClass))

typedef struct _DasherAction DasherAction;
typedef struct _DasherActionClass DasherActionClass;

struct _DasherAction {
  GObject parent;
  gpointer private_data;
};

struct _DasherActionClass {
  GObjectClass parent_class;

  gboolean (*execute)(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
  gboolean (*preview)(DasherAction *pSelf, DasherEditor *pEditor);
  const gchar *(*get_name)(DasherAction *pSelf);
  int (*get_sub_count)(DasherAction *pSelf);
  const gchar *(*get_sub_name)(DasherAction *pSelf, int iIdx);  
  gboolean (*activate)(DasherAction *pSelf);
  gboolean (*deactivate)(DasherAction *pSelf);
};

DasherAction *dasher_action_new();
GType dasher_action_get_type();

gboolean dasher_action_execute(DasherAction *pSelf, DasherEditor *pEditor, gint iIdx);
gboolean dasher_action_preview(DasherAction *pSelf, DasherEditor *pEditor);
const gchar *dasher_action_get_name(DasherAction *pSelf);
int dasher_action_get_sub_count(DasherAction *pSelf);
const gchar *dasher_action_get_sub_name(DasherAction *pSelf, int iIdx);
gboolean dasher_action_activate(DasherAction *pSelf);
gboolean dasher_action_deactivate(DasherAction *pSelf);
gboolean dasher_action_get_active(DasherAction *pSelf);
G_END_DECLS

#endif
