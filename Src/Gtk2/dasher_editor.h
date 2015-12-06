#ifndef __dasher_editor_h__
#define __dasher_editor_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "../DasherCore/ControlManager.h"

G_BEGIN_DECLS

/* Forward declaration */
class DasherAppSettings;
typedef struct _GtkDasherControl GtkDasherControl;
struct _GtkDasherControl;

typedef enum {
  CLIPBOARD_CUT,
  CLIPBOARD_COPY,
  CLIPBOARD_PASTE,
  CLIPBOARD_COPYALL,
  CLIPBOARD_SELECTALL,
  CLIPBOARD_CLEAR
} clipboard_action;

#define DASHER_TYPE_EDITOR            (dasher_editor_get_type())
#define DASHER_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_EDITOR, DasherEditor))
#define DASHER_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_EDITOR, DasherEditor))
#define DASHER_IS_EDITOR(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), DASHER_TYPE_EDITOR))
#define DASHER_IS_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_TYPE_EDITOR))
#define DASHER_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_TYPE_EDITOR, DasherEditorClass))

typedef struct _DasherEditor DasherEditor;
typedef struct _DasherEditorClass DasherEditorClass;

struct _DasherEditor {
  GtkVBox box;
};

struct _DasherEditorClass {
  GtkVBoxClass parent_class;

  void (*filename_changed)(DasherEditor *pDasherEditor);
  void (*buffer_changed)(DasherEditor *pDasherEditor);
  void (*context_changed)(DasherEditor *pDasherEditor);
};

DasherEditor *dasher_editor_new();

GType dasher_editor_get_type();
G_END_DECLS

/* Functions for initialisation and takedown */
void dasher_editor_initialise(DasherEditor *pSelf,
                              DasherAppSettings *pAppSettings,
                              GtkDasherControl *pDasherCtrl, GtkBuilder *pXML,
                              const gchar *szFullPath);

/* Abstract command handler */
gboolean dasher_editor_command(DasherEditor *pSelf, const gchar *szCommand);

GtkTextBuffer *dasher_editor_game_text_buffer(DasherEditor *);

void dasher_editor_clear(DasherEditor *pSelf);
std::string dasher_editor_get_all_text(DasherEditor *pSelf);

/* Functions for editing the active buffer */
void dasher_editor_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
void dasher_editor_delete(DasherEditor *pSelf, int iLength, int iOffset);

/* Function for reading the active buffer */
std::string dasher_editor_get_context(DasherEditor *pSelf, int iOffset, int iLength);
std::string dasher_editor_get_text_around_cursor(DasherEditor *pSelf, Dasher::CControlManager::EditDistance dist);
gint dasher_editor_get_offset(DasherEditor *pSelf);

gint dasher_editor_ctrl_move(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance dist);
gint dasher_editor_ctrl_delete(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance dist);
void dasher_editor_edit_convert(DasherEditor *pSelf);
void dasher_editor_edit_protect(DasherEditor *pSelf);

/* Events proagated from main */
void dasher_editor_handle_parameter_change(DasherEditor *pSelf, gint iParameter);
void dasher_editor_handle_stop(DasherEditor *pSelf);
void dasher_editor_handle_start(DasherEditor *pSelf);
void dasher_editor_toggle_direct_mode(DasherEditor *);

/* Functions needed to maintain application UI */
void dasher_editor_grab_focus(DasherEditor *pSelf);
gboolean dasher_editor_file_changed(DasherEditor *pSelf);
const gchar *dasher_editor_get_filename(DasherEditor *pSelf);

#endif
