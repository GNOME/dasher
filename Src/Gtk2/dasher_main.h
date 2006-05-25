#ifndef __dasher_main_h__
#define __dasher_main_h__

#include <glade/glade.h>
#include <glib.h>
#include <glib-object.h>

#include "DasherAppSettings.h"

G_BEGIN_DECLS
#define TYPE_DASHER_MAIN            (dasher_main_get_type())
#define DASHER_MAIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_MAIN, DasherMain ))
#define DASHER_MAIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_MAIN, DasherMainClass ))
#define IS_DASHER_MAIN(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_MAIN))
#define IS_DASHER_MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_MAIN))
#define DASHER_MAIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_MAIN, DasherMainClass))

typedef struct _DasherMain DasherMain;
typedef struct _DasherMainClass DasherMainClass;

struct _DasherMain {
  GObject parent;
  gpointer private_data;
};

struct _DasherMainClass {
  GObjectClass parent_class;
};

DasherMain *dasher_main_new();
GType dasher_main_get_type();
void dasher_main_load_interface(DasherMain *pSelf);
GladeXML *dasher_main_get_glade(DasherMain *pSelf);
GtkWidget *dasher_main_get_window(DasherMain *pSelf);
void dasher_main_setup_window(DasherMain *pSelf);
void dasher_main_handle_parameter_change(DasherMain *pSelf, int iParameter);
void dasher_main_handle_pre_parameter_change(DasherMain *pSelf, int iParameter);
void dasher_main_set_app_settings(DasherMain *pSelf, DasherAppSettings *pAppSettings);
void dasher_main_load_state(DasherMain *pSelf);
void dasher_main_save_state(DasherMain *pSelf);
void dasher_main_show(DasherMain *pSelf);
void dasher_main_set_filename(DasherMain *pSelf, const gchar *szFilename);
gboolean dasher_main_topmost(DasherMain *pSelf);
G_END_DECLS

#endif
