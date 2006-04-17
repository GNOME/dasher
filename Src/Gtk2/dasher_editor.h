#ifndef __dasher_editor_h__
#define __dasher_editor_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "dasher_buffer_set.h"
#include "dasher_action.h"

typedef enum {
  CLIPBOARD_CUT,
  CLIPBOARD_COPY,
  CLIPBOARD_PASTE,
  CLIPBOARD_COPYALL,
  CLIPBOARD_SELECTALL,
  CLIPBOARD_CLEAR,
} clipboard_action;

G_BEGIN_DECLS
#define TYPE_DASHER_EDITOR            (dasher_editor_get_type())
#define DASHER_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_EDITOR, DasherEditor ))
#define DASHER_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_EDITOR, DasherEditorClass ))
#define IS_DASHER_EDITOR(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_EDITOR))
#define IS_DASHER_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_EDITOR))
#define DASHER_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_EDITOR, DasherEditorClass))

typedef struct _DasherEditor DasherEditor;
typedef struct _DasherEditorClass DasherEditorClass;

struct _DasherEditor {
  GObject parent;
  gpointer private_data;
};

struct _DasherEditorClass {
  GObjectClass parent_class;
};

DasherEditor *dasher_editor_new(GtkTextView *pTextView, GtkVBox *pActionPane);
GType dasher_editor_get_type();

IDasherBufferSet *dasher_editor_get_buffer_set(DasherEditor *pSelf);

// TODO: Just have one 'handle event' method?
void dasher_editor_clipboard(DasherEditor *pSelf, clipboard_action act);
void dasher_editor_handle_stop(DasherEditor *pSelf);
void dasher_editor_handle_start(DasherEditor *pSelf);
void dasher_editor_handle_control(DasherEditor *pSelf, int iNodeID);
void dasher_editor_action_button(DasherEditor *pSelf, DasherAction *pAction);
void dasher_editor_clear(DasherEditor *pSelf, gboolean bStore);
void dasher_editor_actions_start(DasherEditor *pSelf);
bool dasher_editor_actions_more(DasherEditor *pSelf);
void dasher_editor_actions_get_next(DasherEditor *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto);
void dasher_editor_action_set_show(DasherEditor *pSelf, int iActionID, bool bValue);
void dasher_editor_action_set_control(DasherEditor *pSelf, int iActionID, bool bValue);
void dasher_editor_action_set_auto(DasherEditor *pSelf, int iActionID, bool bValue);
void dasher_editor_create_buffer(DasherEditor *pSelf);
void dasher_editor_handle_parameter_change(DasherEditor *pSelf, int iParameter);
void dasher_editor_output(DasherEditor *pSelf, const gchar *szText);
void dasher_editor_delete(DasherEditor *pSelf, int iLength);
void dasher_editor_refresh_context(DasherEditor *pSelf, int iMaxLength);

// Temporarily here - move back to private eventually
const gchar *dasher_editor_get_all_text(DasherEditor *pSelf);

G_END_DECLS

#endif

