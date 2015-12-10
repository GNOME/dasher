#ifndef __gtkdashercontrol_h__
#define __gtkdashercontrol_h__

#include <gtk/gtk.h>

#include "../Common/ModuleSettings.h"
#include "Parameters.h"
#include "../DasherCore/ControlManager.h"

/*Forward declaration*/
typedef struct _DasherEditor DasherEditor;
struct _DasherEditor;

typedef struct _DasherLockInfo DasherLockInfo;

struct _DasherLockInfo {
  const gchar *szMessage;
  gint iPercent; //-1 = unlock, anything else = locked
  unsigned long time;
};

G_BEGIN_DECLS
#define GTK_DASHER_TYPE_CONTROL         (gtk_dasher_control_get_type())
#define GTK_DASHER_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_DASHER_TYPE_CONTROL, GtkDasherControl ))
#define GTK_DASHER_CONTROL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_DASHER_TYPE_CONTROL, GtkDasherControlClass ))
#define GTK_DASHER_IS_CONTROL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_DASHER_TYPE_CONTROL))
#define GTK_DASHER_IS_CONTROL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_DASHER_TYPE_CONTROL))

typedef struct _GtkDasherControl GtkDasherControl;
typedef struct _GtkDasherControlClass GtkDasherControlClass;

struct _GtkDasherControl {
  GtkVBox box;
  //  gpointer private_data;
};

struct _GtkDasherControlClass {
  GtkVBoxClass parent_class;

  // Signal handlers

  void (*dasher_changed) (GtkDasherControl * pDasherControl, gint iParameter, gpointer data);
  void (*dasher_stop) (GtkDasherControl * pDasherControl, gpointer data);
  //These are for ordinary node entry & deletion upon exit
  void (*dasher_edit_insert) (GtkDasherControl * pDasherControl, const gchar * szText, int iOffset, gpointer data);
  void (*dasher_edit_delete) (GtkDasherControl * pDasherControl, const gchar * szText, int iOffset, gpointer data);
  void (*dasher_edit_convert) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_edit_protect) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_control) (GtkDasherControl * pDasherControl, gint iEvent, gpointer data);
  gboolean (*key_press_event) (GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data);
  gboolean (*key_release_event) (GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data);
  void (*dasher_request_settings) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_lock) (GtkDasherControl * pDasherControl, gpointer pLockInfo, gpointer data);
  void (*dasher_message) (GtkDasherControl * pDasherControl, gpointer pLockInfo, gpointer data);
};

GtkWidget *gtk_dasher_control_new();
GType gtk_dasher_control_get_type();

G_END_DECLS

GArray *gtk_dasher_control_get_allowed_values(GtkDasherControl * pControl, int iParameter);
void gtk_dasher_control_train(GtkDasherControl * pControl, const gchar * szFilename);
void gtk_dasher_control_set_editor(GtkDasherControl *pControl, DasherEditor *pEditor);
void gtk_dasher_control_clear_all_context(GtkDasherControl *pControl);
std::string gtk_dasher_control_get_all_text(GtkDasherControl *pControl);
std::string gtk_dasher_control_get_context(GtkDasherControl *pControl, unsigned int iOffset, unsigned int iLength);
std::string gtk_dasher_control_get_text_around_cursor(GtkDasherControl *pControl, Dasher::CControlManager::EditDistance dist);
//void gtk_dasher_control_invalidate_context(GtkDasherControl *pControl, bool bForceStart);
void gtk_dasher_control_set_buffer(GtkDasherControl *pControl, int iOffset);
void gtk_dasher_control_set_offset(GtkDasherControl *pControl, int iOffset);
gint gtk_dasher_control_ctrl_move(GtkDasherControl *pControl, bool bForwards, Dasher::CControlManager::EditDistance dist);
gint gtk_dasher_control_ctrl_delete(GtkDasherControl *pControl, bool bForwards, Dasher::CControlManager::EditDistance dist);
void gtk_dasher_control_external_key_down(GtkDasherControl *pControl, int iKeyVal);
void gtk_dasher_control_external_key_up(GtkDasherControl *pControl, int iKeyVal);
gboolean gtk_dasher_control_get_module_settings(GtkDasherControl * pControl, const gchar *szModule, SModuleSettings **pSettings, gint *iCount);
//For constructing a GtkGameModule around the text buffer:
GtkTextBuffer *gtk_dasher_control_game_text_buffer(GtkDasherControl *pPrivate);
//Calls EnterGameMode or LeaveGameMode, as appropriate, on the underlying CDasherControl
void gtk_dasher_control_set_game_mode(GtkDasherControl *pControl, bool bOn);
bool gtk_dasher_control_get_game_mode(GtkDasherControl *pControl);

#endif
