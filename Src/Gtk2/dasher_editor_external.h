#ifndef __dasher_editor_external_h__
#define __dasher_editor_external_h__

#include <glade/glade.h>
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
  CLIPBOARD_CLEAR,
} clipboard_action;

G_BEGIN_DECLS
#define TYPE_DASHER_EDITOR_EXTERNAL            (dasher_editor_external_get_type())
#define DASHER_EDITOR_EXTERNAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_EDITOR_EXTERNAL, DasherEditorExternal ))
#define DASHER_EDITOR_EXTERNAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_EDITOR_EXTERNAL, DasherEditorExternalClass ))
#define IS_DASHER_EDITOR_EXTERNAL(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_EDITOR_EXTERNAL))
#define IS_DASHER_EDITOR_EXTERNAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_EDITOR_EXTERNAL))
#define DASHER_EDITOR_EXTERNAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_EDITOR_EXTERNAL, DasherEditorExternalClass))

typedef struct _DasherEditorExternal DasherEditorExternal;
typedef struct _DasherEditorExternalClass DasherEditorExternalClass;

struct _DasherEditorExternal {
  DasherEditor parent;
};

struct _DasherEditorExternalClass {
  DasherEditorClass parent_class;

  void (*filename_changed)(DasherEditorExternal *pDasherEditorExternal);
  void (*buffer_changed)(DasherEditorExternal *pDasherEditorExternal);
  void (*context_changed)(DasherEditorExternal *pDasherEditorExternal);
};

DasherEditorExternal *dasher_editor_external_new();
void dasher_editor_external_initialise(DasherEditorExternal *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GladeXML *pGladeXML, const gchar *szFullPath);

GType dasher_editor_external_get_type();

/* Replace this with GTK text buffer */
//IDasherBufferSet *dasher_editor_external_get_buffer_set(DasherEditorExternal *pSelf);

/* // TODO: Just have one 'handle event' method? */
/* gboolean dasher_editor_external_command(DasherEditorExternal *pSelf, const gchar *szCommand); */

/* void dasher_editor_external_handle_font(DasherEditorExternal *pSelf, const gchar *szFont); */

/* /\* To be obsoleted by movement to GTK buffers *\/ */
/* void dasher_editor_external_output(DasherEditorExternal *pSelf, const gchar *szText, int iOffset); */
/* void dasher_editor_external_delete(DasherEditorExternal *pSelf, int iLength, int iOffset); */
/* const gchar *dasher_editor_external_get_context(DasherEditorExternal *pSelf, int iOffset, int iLength); */
/* gint dasher_editor_external_get_offset(DasherEditorExternal *pSelf); */

/* /\* Events proagated from main *\/ */
/* void dasher_editor_external_handle_stop(DasherEditorExternal *pSelf); */
/* void dasher_editor_external_handle_start(DasherEditorExternal *pSelf); */
/* void dasher_editor_external_handle_control(DasherEditorExternal *pSelf, int iNodeID); */

/* /\* Action related methods - TODO: a lot of this should be moved to dasher_main (eg action on stop etc) - that way we get a better level of abstraction, and can incorporate commands from otehr modules too. Actions should only be externally visible as a list of string commands*\/ */
/* void dasher_editor_external_action_button(DasherEditorExternal *pSelf, DasherAction *pAction); */
/* void dasher_editor_external_actions_start(DasherEditorExternal *pSelf); */
/* bool dasher_editor_external_actions_more(DasherEditorExternal *pSelf); */
/* void dasher_editor_external_actions_get_next(DasherEditorExternal *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto); */
/* void dasher_editor_external_action_set_show(DasherEditorExternal *pSelf, int iActionID, bool bValue); */
/* void dasher_editor_external_action_set_control(DasherEditorExternal *pSelf, int iActionID, bool bValue); */
/* void dasher_editor_external_action_set_auto(DasherEditorExternal *pSelf, int iActionID, bool bValue); */

/* void dasher_editor_external_grab_focus(DasherEditorExternal *pSelf); */

/* /\* TODO: Tutorial editor should be a separate class *\/ */
/* //void dasher_editor_external_start_tutorial(DasherEditorExternal *pSelf); */

/* /\* Todo: possibly tidy up the need to have this public (quit in dasher_main possibly too connected) *\/ */
/* gboolean dasher_editor_external_file_changed(DasherEditorExternal *pSelf); */
/* const gchar *dasher_editor_external_get_filename(DasherEditorExternal *pSelf); */

/* const gchar *dasher_editor_external_get_all_text(DasherEditorExternal *pSelf); */
/* const gchar *dasher_editor_external_get_new_text(DasherEditorExternal *pSelf); */
G_END_DECLS

#endif

