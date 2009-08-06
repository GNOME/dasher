#ifndef __gtkdashercontrol_h__
#define __gtkdashercontrol_h__

#include <gtk/gtk.h>

#include "../Common/ModuleSettings.h"
#include "Parameters.h"
#include "ControlManager.h"

typedef struct _DasherLockInfo DasherLockInfo;

struct _DasherLockInfo {
  const gchar *szMessage;
  gboolean bLock;
  gint iPercent;
};

typedef struct _DasherMessageInfo DasherMessageInfo;

struct _DasherMessageInfo {
  const gchar *szMessage;
  gint iID;
  gint iType;
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
  void (*dasher_start) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_stop) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_edit_insert) (GtkDasherControl * pDasherControl, const gchar * szText, int iOffset, gpointer data);
  void (*dasher_edit_delete) (GtkDasherControl * pDasherControl, const gchar * szText, int iOffset, gpointer data);
  void (*dasher_edit_convert) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_edit_protect) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_control) (GtkDasherControl * pDasherControl, gint iEvent, gpointer data);
  gboolean (*key_press_event) (GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data);
  gboolean (*key_release_event) (GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data);
  void (*dasher_context_request) (GtkDasherControl * pDasherControl, gint iOffset, gint iLength, gpointer data);
  void (*dasher_request_settings) (GtkDasherControl * pDasherControl, gpointer data);
  void (*dasher_lock) (GtkDasherControl * pDasherControl, gpointer pLockInfo, gpointer data);
  void (*dasher_message) (GtkDasherControl * pDasherControl, gpointer pLockInfo, gpointer data);
  void (*dasher_command) (GtkDasherControl * pDasherControl, const gchar * szText, gpointer data);
};

GtkWidget *gtk_dasher_control_new();
GType gtk_dasher_control_get_type();

gboolean gtk_dasher_control_default_key_press_handler(GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data);
gboolean gtk_dasher_control_default_key_release_handler(GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data);

void gtk_dasher_control_set_parameter_bool(GtkDasherControl * pControl, int iParameter, bool bValue);
void gtk_dasher_control_set_parameter_long(GtkDasherControl * pControl, int iParameter, long lValue);
void gtk_dasher_control_set_parameter_string(GtkDasherControl * pControl, int iParameter, const char *szValue);
bool gtk_dasher_control_get_parameter_bool(GtkDasherControl * pControl, int iParameter);
long gtk_dasher_control_get_parameter_long(GtkDasherControl * pControl, int iParameter);
void gtk_dasher_control_reset_parameter(GtkDasherControl * pControl, int iParameter);
const char *gtk_dasher_control_get_parameter_string(GtkDasherControl * pControl, int iParameter);
GArray *gtk_dasher_control_get_allowed_values(GtkDasherControl * pControl, int iParameter);
void gtk_dasher_control_train(GtkDasherControl * pControl, const gchar * szFilename);
void gtk_dasher_control_set_context(GtkDasherControl *pControl, const gchar *szContext);
//void gtk_dasher_control_invalidate_context(GtkDasherControl *pControl, bool bForceStart);
void gtk_dasher_control_set_buffer(GtkDasherControl *pControl, int iOffset);
void gtk_dasher_control_set_offset(GtkDasherControl *pControl, int iOffset);
void gtk_dasher_control_unset_buffer(GtkDasherControl *pControl);
void gtk_dasher_control_register_node(GtkDasherControl *pControl, int iID, const gchar *szLabel, int iColour);
void gtk_dasher_control_connect_node(GtkDasherControl *pControl, int iChild, int iParent, int iAfter);
void gtk_dasher_control_disconnect_node(GtkDasherControl *pControl, int iChild, int iParent);
void gtk_dasher_control_external_key_down(GtkDasherControl *pControl, int iKeyVal);
void gtk_dasher_control_external_key_up(GtkDasherControl *pControl, int iKeyVal);
gboolean gtk_dasher_control_get_module_settings(GtkDasherControl * pControl, const gchar *szModule, SModuleSettings **pSettings, gint *iCount);
void gtk_dasher_control_add_game_mode_string(GtkDasherControl *pControl, const gchar *szString);
void gtk_dasher_control_game_messagein(GtkDasherControl *pControl, int message, void* messagedata);
void gtk_dasher_control_game_helperreg(GtkDasherControl *pControl, void* gameHelper);
void gtk_dasher_control_game_messageout(GtkDasherControl *pControl, int message, const void* messagedata);

void gtk_dasher_control_force_pause(GtkDasherControl *pControl);
double gtk_dasher_control_get_framerate(GtkDasherControl *pControl);
void gtk_dasher_control_add_action_button(GtkDasherControl *pControl, const gchar *szCommand);
void gtk_dasher_control_set_control_offset(GtkDasherControl *pControl, gint iOffset);
void gtk_dasher_user_log_new_trial(GtkDasherControl * pControl);
void gtk_dasher_control_set_focus(GtkDasherControl * pControl);
const gchar* gtk_dasher_control_cl_set(GtkDasherControl *pControl, const gchar *szKey, const gchar *szValue);

G_END_DECLS
#endif
