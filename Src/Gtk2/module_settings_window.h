// module_settings_window.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __module_settings_window_h__
#define __module_settings_window_h__

#include "../Common/ModuleSettings.h"
#include "DasherAppSettings.h"

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_MODULE_SETTINGS_WINDOW         (module_settings_window_get_type())
#define MODULE_SETTINGS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_MODULE_SETTINGS_WINDOW, ModuleSettingsWindow ))
#define MODULE_SETTINGS_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MODULE_SETTINGS_WINDOW, ModuleSettingsWindow ))
#define IS_MODULE_SETTINGS_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MODULE_SETTINGS_WINDOW))
#define IS_MODULE_SETTINGS_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MODULE_SETTINGS_WINDOW))

typedef struct _ModuleSettingsWindow ModuleSettingsWindow;
typedef struct _ModuleSettingsWindowClass ModuleSettingsWindowClass;

struct _ModuleSettingsWindow {
  GtkDialog window;
  gpointer private_data;
};

struct _ModuleSettingsWindowClass {
  GtkDialogClass parent_class;
};

GType module_settings_window_get_type();
GtkWidget *module_settings_window_new(DasherAppSettings *pAppSettings, const gchar *szName, SModuleSettings *pSettings, int iCount);
G_END_DECLS

#endif
