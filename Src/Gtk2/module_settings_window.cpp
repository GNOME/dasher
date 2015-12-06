// module_settings_window.cpp
//
// Copyright (c) 2008 The Dasher Team
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <glib/gi18n.h>

#include "module_settings_window.h"

typedef struct _ModuleSettingsData ModuleSettingsData;

struct _ModuleSettingsData {
  ModuleSettingsData *pNext;
  GtkWidget *pWidget;
  int iParameter;
  int iDivisor;
};

struct _ModuleSettingsWindowPrivate {
  DasherAppSettings *pAppSettings;
  ModuleSettingsData *pFirst;
};

typedef struct _ModuleSettingsWindowPrivate ModuleSettingsWindowPrivate;

#define MODULE_SETTINGS_WINDOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_MODULE_SETTINGS_WINDOW, ModuleSettingsWindowPrivate))

G_DEFINE_TYPE(ModuleSettingsWindow, module_settings_window, GTK_TYPE_DIALOG)

/* Private members */
static ModuleSettingsData *module_settings_window_get_settings_data(ModuleSettingsWindow *pSelf, GtkWidget *pWidget);
static void module_settings_window_handle_bool_changed(ModuleSettingsWindow *pSelf, GtkToggleButton *pToggleButton);
static void module_settings_window_handle_long_changed(ModuleSettingsWindow *pSelf, GtkRange *pRange);
static void module_settings_window_handle_longspin_changed(ModuleSettingsWindow *pSelf, GtkSpinButton *pSpinButton);
static void module_settings_window_handle_string_changed(ModuleSettingsWindow *pSelf, GtkEditable *pEditable);
static gboolean module_settings_window_handle_close(ModuleSettingsWindow *pSelf);

// Callbacks (not strictly class members)
extern "C" void handle_bool_changed(GtkToggleButton *pToggleButton, gpointer pUserData);
extern "C" void handle_long_changed(GtkRange *pRange, gpointer pUserData);
extern "C" void handle_longspin_changed(GtkSpinButton *pSpinButton, gpointer pUserData);
extern "C" void handle_string_changed(GtkEditable *pEditable, gpointer pUserData);
extern "C" gboolean handle_close(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean handle_close_event(GtkWidget *pWidget, GdkEvent *pEvent, gpointer pUserData);

static void 
module_settings_window_class_init(ModuleSettingsWindowClass *pClass) {
  //  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  //  pObjectClass->finalize = module_settings_window_destroy;
  g_type_class_add_private(pClass, sizeof(ModuleSettingsWindowPrivate));
}
 
static void 
module_settings_window_init(ModuleSettingsWindow *pDasherControl) {
  ModuleSettingsWindowPrivate *pPrivateData = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pDasherControl);

  pPrivateData->pFirst = NULL;

  GtkWidget *pButton = gtk_dialog_add_button(&(pDasherControl->window), _("_Close"), GTK_RESPONSE_NONE);

  g_signal_connect(G_OBJECT(pButton), "clicked", G_CALLBACK(handle_close), pDasherControl);
  g_signal_connect(G_OBJECT(&(pDasherControl->window)), "delete-event", G_CALLBACK(handle_close_event), pDasherControl);
}

GtkWidget *
module_settings_window_new(DasherAppSettings *pAppSettings, const gchar *szName, SModuleSettings *pSettings, int iCount) {
  ModuleSettingsWindow *pDasherControl;
  pDasherControl = MODULE_SETTINGS_WINDOW(g_object_new(module_settings_window_get_type(), NULL));

  gtk_window_set_title(GTK_WINDOW(pDasherControl), _("Dasher Module Options"));
  
  ModuleSettingsWindowPrivate *pPrivate = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pDasherControl);
  
  pPrivate->pAppSettings = pAppSettings;

  gchar *szFrameTitle = g_strdup_printf(_("%s Options:"), szName);
  
  GtkWidget *pFrame = gtk_frame_new(szFrameTitle);

  g_free(szFrameTitle);

  gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(&(pDasherControl->window))), pFrame);
  g_object_set(G_OBJECT(pFrame), "border-width", 8, NULL);

  GtkWidget *pTable = gtk_table_new(iCount, 2, FALSE);
  g_object_set(G_OBJECT(pTable), "border-width", 8, NULL);
  gtk_container_add(GTK_CONTAINER(pFrame), pTable);
  
 
  for(int i(0); i < iCount; ++i) {
    GtkWidget *pControl = NULL;
    
    if(pSettings[i].iType == T_BOOL) {
      pControl = gtk_check_button_new_with_label(_(pSettings[i].szDescription)); 
      
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pControl), 
				   pPrivate->pAppSettings->GetBool(pSettings[i].iParameter));
      
      gtk_table_attach_defaults(GTK_TABLE(pTable), pControl, 0, 2, i, i+1);
      g_signal_connect(G_OBJECT(pControl), "toggled", G_CALLBACK(handle_bool_changed), pDasherControl);
      
    }
    else {
      GtkWidget *pLabel = gtk_label_new(_(pSettings[i].szDescription));
      gtk_misc_set_alignment(GTK_MISC(pLabel), 0, 0.5);
      gtk_table_attach_defaults(GTK_TABLE(pTable), pLabel, 0, 1, i, i+1);
      
      switch(pSettings[i].iType) {
      case T_LONG:
	pControl = gtk_hscale_new_with_range(pSettings[i].iMin / static_cast<double>(pSettings[i].iDivisor), 
					     pSettings[i].iMax / static_cast<double>(pSettings[i].iDivisor), 
					     pSettings[i].iStep / static_cast<double>(pSettings[i].iDivisor));
	
	gtk_range_set_increments(GTK_RANGE(pControl), pSettings[i].iStep / static_cast<double>(pSettings[i].iDivisor),
				 pSettings[i].iStep / static_cast<double>(pSettings[i].iDivisor));

	gtk_range_set_value(GTK_RANGE(pControl), 
			    pPrivate->pAppSettings->GetLong(pSettings[i].iParameter)
			    / static_cast<double>(pSettings[i].iDivisor));
	
	gtk_widget_set_size_request(pControl, 256, -1);
	
	g_signal_connect(G_OBJECT(pControl), "value-changed", G_CALLBACK(handle_long_changed), pDasherControl);
	break;
      case T_LONGSPIN:
      	pControl = gtk_spin_button_new_with_range(pSettings[i].iMin / static_cast<double>(pSettings[i].iDivisor), 
						  pSettings[i].iMax / static_cast<double>(pSettings[i].iDivisor), 
						  pSettings[i].iStep / static_cast<double>(pSettings[i].iDivisor));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(pControl), 
				  pPrivate->pAppSettings->GetLong(pSettings[i].iParameter)
				  / static_cast<double>(pSettings[i].iDivisor));

	gtk_widget_set_size_request(pControl, 256, -1);	

	g_signal_connect(G_OBJECT(pControl), "value-changed", G_CALLBACK(handle_longspin_changed), pDasherControl);
	break;
      case T_STRING:
	pControl = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(pControl),
			   pPrivate->pAppSettings->GetString(pSettings[i].iParameter).c_str());
	gtk_widget_set_size_request(pControl, 256, -1);

	g_signal_connect(G_OBJECT(pControl), "changed", G_CALLBACK(handle_string_changed), pDasherControl);
	break;
      default:
	g_error("Unknown type specified for module setting");
	break;
      }

      gtk_table_attach_defaults(GTK_TABLE(pTable), pControl, 1, 2, i, i+1);
    }      

    gtk_table_set_row_spacings(GTK_TABLE(pTable), 4);
    gtk_table_set_col_spacings(GTK_TABLE(pTable), 4);
  

    ModuleSettingsData *pNewData = new ModuleSettingsData;

    pNewData->pNext = pPrivate->pFirst;
    pNewData->pWidget = pControl;
    pNewData->iParameter = pSettings[i].iParameter;
    pNewData->iDivisor = pSettings[i].iDivisor;

    pPrivate->pFirst = pNewData;

  }

  gtk_widget_show_all(pFrame);

  return GTK_WIDGET(pDasherControl);
}

/* private members */
static ModuleSettingsData *
module_settings_window_get_settings_data(ModuleSettingsWindow *pSelf, GtkWidget *pWidget) {
  ModuleSettingsWindowPrivate *pPrivate = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pSelf);

  ModuleSettingsData *pCurrentData = pPrivate->pFirst;

  while(pCurrentData != NULL) {
    if(pCurrentData->pWidget == pWidget)
      return pCurrentData;

    pCurrentData = pCurrentData->pNext;
  }

  return NULL;
}

static void 
module_settings_window_handle_bool_changed(ModuleSettingsWindow *pSelf, GtkToggleButton *pToggleButton) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pToggleButton));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pSelf);
  
  gboolean bNewValue = gtk_toggle_button_get_active(pToggleButton);
  pPrivate->pAppSettings->SetBool(pData->iParameter, bNewValue);
}

static void 
module_settings_window_handle_long_changed(ModuleSettingsWindow *pSelf, GtkRange *pRange) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pRange));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pSelf);

  gint iNewValue = (gint)(gtk_range_get_value(pRange) * pData->iDivisor);
  pPrivate->pAppSettings->SetLong(pData->iParameter, iNewValue);
}

static void 
module_settings_window_handle_longspin_changed(ModuleSettingsWindow *pSelf, GtkSpinButton *pSpinButton) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pSpinButton));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pSelf);

  gint iNewValue = (gint)(gtk_spin_button_get_value(pSpinButton) * pData->iDivisor);
  pPrivate->pAppSettings->SetLong(pData->iParameter, iNewValue);
}

static void 
module_settings_window_handle_string_changed(ModuleSettingsWindow *pSelf, GtkEditable *pEditable) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pEditable));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate = MODULE_SETTINGS_WINDOW_GET_PRIVATE(pSelf);

  const gchar *szNewValue = gtk_editable_get_chars(pEditable, 0, -1);
  pPrivate->pAppSettings->SetString(pData->iParameter, szNewValue);
}

static gboolean 
module_settings_window_handle_close(ModuleSettingsWindow *pSelf) {
  gtk_widget_destroy(GTK_WIDGET(&(pSelf->window)));
  //TODO what do we need to do to free/unref the ModuleSettingsWindow object?
  // gtk_object_unref(GTK_OBJECT(pSelf)); prints error/diagnostics that the pointer is not a GtkObject (?!)
  // trying to "delete pSelf;" aborts immediately...?
  return TRUE;
}

// Callbacks

extern "C" void 
handle_bool_changed(GtkToggleButton *pToggleButton, gpointer pUserData) {
  module_settings_window_handle_bool_changed(MODULE_SETTINGS_WINDOW(pUserData), pToggleButton);
}

extern "C" void 
handle_long_changed(GtkRange *pRange, gpointer pUserData) {
  module_settings_window_handle_long_changed(MODULE_SETTINGS_WINDOW(pUserData), pRange);
}

extern "C" void 
handle_longspin_changed(GtkSpinButton *pSpinButton, gpointer pUserData) {
  module_settings_window_handle_longspin_changed(MODULE_SETTINGS_WINDOW(pUserData), pSpinButton);
}

extern "C" void 
handle_string_changed(GtkEditable *pEditable, gpointer pUserData) {
  module_settings_window_handle_string_changed(MODULE_SETTINGS_WINDOW(pUserData), pEditable);
}

extern "C" gboolean 
handle_close(GtkWidget *pWidget, gpointer pUserData) {
  return module_settings_window_handle_close(MODULE_SETTINGS_WINDOW(pUserData));
}

extern "C" gboolean 
handle_close_event(GtkWidget *pWidget, GdkEvent *pEvent, gpointer pUserData) {
  return module_settings_window_handle_close(MODULE_SETTINGS_WINDOW(pUserData));
}
