#ifndef __module_settings_window_h__
#define __module_settings_window_h__

#include "../Common/ModuleSettings.h"
#include "DasherAppSettings.h"

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_MODULE_SETTINGS_WINDOW         (module_settings_window_get_type())
#define MODULE_SETTINGS_WINDOW(obj)         (GTK_CHECK_CAST((obj), TYPE_MODULE_SETTINGS_WINDOW, ModuleSettingsWindow ))
#define MODULE_SETTINGS_WINDOW_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), TYPE_MODULE_SETTINGS_WINDOW, ModuleSettingsWindow ))
#define IS_MODULE_SETTINGS_WINDOW(obj)		(GTK_CHECK_TYPE ((obj), TYPE_MODULE_SETTINGS_WINDOW))
#define IS_MODULE_SETTINGS_WINDOW_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), TYPE_MODULE_SETTINGS_WINDOW))

typedef struct _ModuleSettingsWindow ModuleSettingsWindow;
typedef struct _ModuleSettingsWindowClass ModuleSettingsWindowClass;

struct _ModuleSettingsWindow {
  GtkDialog window;
  gpointer private_data;
};

struct _ModuleSettingsWindowClass {
  GtkDialogClass parent_class;
};

GtkWidget *module_settings_window_new(DasherAppSettings *pAppSettings, const gchar *szName, SModuleSettings *pSettings, int iCount);
GType module_settings_window_get_type();
G_END_DECLS

#endif
