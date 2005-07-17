#ifndef __gtkdashercontrol_h__
#define __gtkdashercontrol_h__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <glib.h>

G_BEGIN_DECLS

#define TYPE_GTK_DASHER_CONTROL         (gtk_dasher_control_get_type())
#define GTK_DASHER_CONTROL(obj)         (GTK_CHECK_CAST((obj), TYPE_GTK_DASHER_CONTROL, GtkDasherControl ))
#define GTK_DASHER_CONTROL_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), TYPE_GTK_DASHER_CONTROL, GtkDasherControlClass ))
#define IS_GTK_DASHER_CONTROL(obj)		(GTK_CHECK_TYPE ((obj), TYPE_GTK_DASHER_CONTROL))
#define IS_GTK_DASHER_CONTROL_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), TYPE_GTK_DASHER_CONTROL))

  typedef struct _GtkDasherControl GtkDasherControl;
  typedef struct _GtkDasherControlClass GtkDasherControlClass;

  struct _GtkDasherControl {
    GtkVBox   box;
    gpointer  private_data;
  };
  
  struct _GtkDasherControlClass {
    GtkVBoxClass  parent_class;

    // Signal handlers

    void (*dasher_changed) (GtkDasherControl *pDasherControl, gint iParameter, gpointer data );
    void (*dasher_start) (GtkDasherControl *pDasherControl, gpointer data);
    void (*dasher_stop) (GtkDasherControl *pDasherControl, gpointer data);
    void (*dasher_edit_insert) (GtkDasherControl *pDasherControl, const gchar *szText, gpointer data);
    void (*dasher_edit_delete) (GtkDasherControl *pDasherControl, const gchar *szText, gpointer data);
  };

  GtkWidget *gtk_dasher_control_new();
  GType gtk_dasher_control_get_type();

  void gtk_dasher_control_set_parameter_bool( GtkDasherControl *pControl, int iParameter, bool bValue );
  void gtk_dasher_control_set_parameter_long(  GtkDasherControl *pControl, int iParameter, long lValue );
  void gtk_dasher_control_set_parameter_string(  GtkDasherControl *pControl, int iParameter, const char *szValue );
  
  bool gtk_dasher_control_get_parameter_bool(  GtkDasherControl *pControl, int iParameter );
  long gtk_dasher_control_get_parameter_long(  GtkDasherControl *pControl, int iParameter );
const char *gtk_dasher_control_get_parameter_string( GtkDasherControl *pControl,  int iParameter );

GArray *gtk_dasher_control_get_allowed_values( GtkDasherControl *pControl, int iParameter );

void gtk_dasher_control_train( GtkDasherControl *pControl, const gchar *szFilename );

G_END_DECLS

#endif
