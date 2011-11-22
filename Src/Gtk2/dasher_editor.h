#ifndef __dasher_editor_h__
#define __dasher_editor_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "../DasherCore/ControlManager.h"

/* Forward declaration */
typedef struct _DasherAppSettings DasherAppSettings;
struct _DasherAppSettings;
typedef struct _GtkDasherControl GtkDasherControl;
struct _GtkDasherControl;

G_BEGIN_DECLS
#define DASHER_TYPE_EDITOR            (dasher_editor_get_type())
#define DASHER_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_EDITOR, DasherEditor ))
#define DASHER_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_EDITOR, DasherEditorClass ))
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

  /* VTable */
  void (*initialise)(DasherEditor *, DasherAppSettings *, GtkDasherControl *, GtkBuilder *, const gchar *);
  gboolean (*command)(DasherEditor *, const gchar *);
  void (*clear)(DasherEditor *);
  const gchar *(*get_all_text)(DasherEditor *);
  const gchar *(*get_new_text)(DasherEditor *);
  void (*output)(DasherEditor *, const gchar *, int);
  void (*delete_text)(DasherEditor *, int, int);
  const gchar *(*get_context)(DasherEditor *, int, int);
  gint (*get_offset)(DasherEditor *);
  void (*handle_parameter_change)(DasherEditor *, gint);
  void (*handle_start)(DasherEditor *);
  void (*handle_stop)(DasherEditor *);
  void (*grab_focus)(DasherEditor *);
  gboolean (*file_changed)(DasherEditor *);
  const gchar *(*get_filename)(DasherEditor *);
  ///Get a buffer to display formatted text for game mode, if we have one
  GtkTextBuffer *(*game_text_buffer)(DasherEditor *);

  /* Signal handlers */
  void (*filename_changed)(DasherEditor *);
  void (*buffer_changed)(DasherEditor *);
  void (*context_changed)(DasherEditor *);

  /* These moved from dasher_buffer_set */
  //void (*insert_text)(DasherEditor *pSelf, const gchar *szText, int iOffset);// = output
  //void (*delete_text)(DasherEditor *pSelf, gint iLength, int iOffset);//duplicate
  //gchar *(*get_context)(DasherEditor *pSelf, gint iOffset, gint iLength);//duplicate
  gint (*ctrl_move)(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance iDist);
  gint (*ctrl_delete)(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance iDist);
  void (*edit_convert)(DasherEditor *pSelf);
  void (*edit_protect)(DasherEditor *pSelf);
  //gint (*get_offset)(DasherEditor *pSelf);//duplicate
};

/* Boilerplate code */
GType dasher_editor_get_type();

/* Functions for initialisation and takedown */
void dasher_editor_initialise(DasherEditor *pSelf,
                              DasherAppSettings *pAppSettings,
                              GtkDasherControl *pDasherCtrl, GtkBuilder *pXML,
                              const gchar *szFullPath);

/* Abstract command handler */
gboolean dasher_editor_command(DasherEditor *pSelf, const gchar *szCommand);

GtkTextBuffer *dasher_editor_game_text_buffer(DasherEditor *);

void dasher_editor_clear(DasherEditor *pSelf);
const gchar *dasher_editor_get_all_text(DasherEditor *pSelf);
const gchar *dasher_editor_get_new_text(DasherEditor *pSelf);

/* Functions for editing the active buffer */
void dasher_editor_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
void dasher_editor_delete(DasherEditor *pSelf, int iLength, int iOffset);
void dasher_editor_start_compose(DasherEditor *pSelf);
void dasher_editor_end_compose(DasherEditor *pSelf, bool bKeep);

/* Function for reading the active buffer */
const gchar *dasher_editor_get_context(DasherEditor *pSelf, int iOffset, int iLength);
gint dasher_editor_get_offset(DasherEditor *pSelf);

gint dasher_editor_ctrl_move(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance dist);
gint dasher_editor_ctrl_delete(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance dist);
void dasher_editor_edit_convert(DasherEditor *pSelf);
void dasher_editor_edit_protect(DasherEditor *pSelf);

/* Events proagated from main */
void dasher_editor_handle_parameter_change(DasherEditor *pSelf, gint iParameter);
void dasher_editor_handle_stop(DasherEditor *pSelf);
void dasher_editor_handle_start(DasherEditor *pSelf);

/* Functions needed to maintain application UI */
void dasher_editor_grab_focus(DasherEditor *pSelf);
gboolean dasher_editor_file_changed(DasherEditor *pSelf);
const gchar *dasher_editor_get_filename(DasherEditor *pSelf);

G_END_DECLS

#endif
