#ifndef __dasher_editor_internal_h__
#define __dasher_editor_internal_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "DasherAppSettings.h"
#include "dasher_action.h"
#include "dasher_buffer_set.h"
#include "dasher_editor.h"

/* Forward declaration */
typedef struct _DasherMain DasherMain;
struct _DasherMain;

typedef enum {
  CLIPBOARD_CUT,
  CLIPBOARD_COPY,
  CLIPBOARD_PASTE,
  CLIPBOARD_COPYALL,
  CLIPBOARD_SELECTALL,
  CLIPBOARD_CLEAR
} clipboard_action;

G_BEGIN_DECLS
#define DASHER_TYPE_EDITOR_INTERNAL            (dasher_editor_internal_get_type())
#define DASHER_EDITOR_INTERNAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternal ))
#define DASHER_EDITOR_INTERNAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternalClass ))
#define DASHER_IS_EDITOR_INTERNAL(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), DASHER_TYPE_EDITOR_INTERNAL))
#define DASHER_IS_EDITOR_INTERNAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_TYPE_EDITOR_INTERNAL))
#define DASHER_EDITOR_INTERNAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternalClass))

typedef struct _DasherEditorInternal DasherEditorInternal;
typedef struct _DasherEditorInternalClass DasherEditorInternalClass;

struct _DasherEditorInternal {
  DasherEditor parent;
};

struct _DasherEditorInternalClass {
  DasherEditorClass parent_class;

  void (*filename_changed)(DasherEditorInternal *pDasherEditorInternal);
  void (*buffer_changed)(DasherEditorInternal *pDasherEditorInternal);
  void (*context_changed)(DasherEditorInternal *pDasherEditorInternal);
};

DasherEditorInternal *dasher_editor_internal_new();
void dasher_editor_internal_initialise(DasherEditorInternal *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath);

void dasher_editor_internal_cleartext(DasherEditorInternal *pSelf);
GType dasher_editor_internal_get_type();

/* Replace this with GTK text buffer */
//IDasherBufferSet *dasher_editor_internal_get_buffer_set(DasherEditorInternal *pSelf);

/* // TODO: Just have one 'handle event' method? */
/* gboolean dasher_editor_internal_command(DasherEditorInternal *pSelf, const gchar *szCommand); */

/* void dasher_editor_internal_handle_font(DasherEditorInternal *pSelf, const gchar *szFont); */

/* /\* To be obsoleted by movement to GTK buffers *\/ */
/* void dasher_editor_internal_output(DasherEditorInternal *pSelf, const gchar *szText, int iOffset); */
/* void dasher_editor_internal_delete(DasherEditorInternal *pSelf, int iLength, int iOffset); */
/* const gchar *dasher_editor_internal_get_context(DasherEditorInternal *pSelf, int iOffset, int iLength); */
/* gint dasher_editor_internal_get_offset(DasherEditorInternal *pSelf); */

/* /\* Events proagated from main *\/ */
/* void dasher_editor_internal_handle_stop(DasherEditorInternal *pSelf); */
/* void dasher_editor_internal_handle_start(DasherEditorInternal *pSelf); */
/* void dasher_editor_internal_handle_control(DasherEditorInternal *pSelf, int iNodeID); */

/* /\* Action related methods - TODO: a lot of this should be moved to dasher_main (eg action on stop etc) - that way we get a better level of abstraction, and can incorporate commands from other modules too. Actions should only be externally visible as a list of string commands*\/ */
/* void dasher_editor_internal_action_button(DasherEditorInternal *pSelf, DasherAction *pAction); */
/* void dasher_editor_internal_actions_start(DasherEditorInternal *pSelf); */
/* bool dasher_editor_internal_actions_more(DasherEditorInternal *pSelf); */
/* void dasher_editor_internal_actions_get_next(DasherEditorInternal *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto); */
/* void dasher_editor_internal_action_set_show(DasherEditorInternal *pSelf, int iActionID, bool bValue); */
/* void dasher_editor_internal_action_set_control(DasherEditorInternal *pSelf, int iActionID, bool bValue); */
/* void dasher_editor_internal_action_set_auto(DasherEditorInternal *pSelf, int iActionID, bool bValue); */

/* void dasher_editor_internal_grab_focus(DasherEditorInternal *pSelf); */

/* /\* TODO: Tutorial editor should be a separate class *\/ */
/* //void dasher_editor_internal_start_tutorial(DasherEditorInternal *pSelf); */

/* /\* Todo: possibly tidy up the need to have this public (quit in dasher_main possibly too connected) *\/ */
/* gboolean dasher_editor_internal_file_changed(DasherEditorInternal *pSelf); */
/* const gchar *dasher_editor_internal_get_filename(DasherEditorInternal *pSelf); */

/* const gchar *dasher_editor_internal_get_all_text(DasherEditorInternal *pSelf); */
/* const gchar *dasher_editor_internal_get_new_text(DasherEditorInternal *pSelf); */
G_END_DECLS

#endif
