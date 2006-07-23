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

// Private members
static void module_settings_window_class_init(ModuleSettingsWindowClass * pClass);
static void module_settings_window_init(ModuleSettingsWindow * pControl);
static void module_settings_window_destroy(GObject * pObject);

ModuleSettingsData *module_settings_window_get_settings_data(ModuleSettingsWindow *pSelf, GtkWidget *pWidget);
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

GType module_settings_window_get_type() {

  static GType module_settings_window_type = 0;

  if(!module_settings_window_type) {

    static const GTypeInfo module_settings_window_info = {
      sizeof(ModuleSettingsWindowClass),
      NULL,
      NULL,
      (GClassInitFunc) module_settings_window_class_init,
      NULL,
      NULL,
      sizeof(ModuleSettingsWindow),
      0,
      (GInstanceInitFunc) module_settings_window_init,
      NULL
    };

    module_settings_window_type = g_type_register_static(GTK_TYPE_DIALOG, "ModuleSettingsWindow", &module_settings_window_info, static_cast < GTypeFlags > (0));
  }

  return module_settings_window_type;
}

static void module_settings_window_class_init(ModuleSettingsWindowClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = module_settings_window_destroy;
}
 
static void module_settings_window_init(ModuleSettingsWindow *pDasherControl) {
  ModuleSettingsWindowPrivate *pPrivateData;

  pPrivateData = g_new0(ModuleSettingsWindowPrivate, 1);

  pPrivateData->pFirst = NULL;

  pDasherControl->private_data = pPrivateData;

  GtkWidget *pButton = gtk_dialog_add_button(&(pDasherControl->window), GTK_STOCK_CLOSE, GTK_RESPONSE_NONE);

  g_signal_connect(G_OBJECT(pButton), "clicked", G_CALLBACK(handle_close), pDasherControl);
  g_signal_connect(G_OBJECT(&(pDasherControl->window)), "delete-event", G_CALLBACK(handle_close_event), pDasherControl);
}

static void module_settings_window_destroy(GObject *pObject) {
  ModuleSettingsWindow *pDasherControl = MODULE_SETTINGS_WINDOW(pObject);
  g_free(pDasherControl->private_data);

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

GtkWidget *module_settings_window_new(DasherAppSettings *pAppSettings, const gchar *szName, SModuleSettings *pSettings, int iCount) {
  ModuleSettingsWindow *pDasherControl;
  pDasherControl = MODULE_SETTINGS_WINDOW(g_object_new(module_settings_window_get_type(), NULL));

  gtk_window_set_title(GTK_WINDOW(pDasherControl), "Dasher Module Options");
  
  ModuleSettingsWindowPrivate *pPrivate((ModuleSettingsWindowPrivate *)(pDasherControl->private_data));
  
  pPrivate->pAppSettings = pAppSettings;

  gchar *szFrameTitle = new gchar[strlen(szName)+10];
  strcpy(szFrameTitle, szName);
  strcat(szFrameTitle, " Options:");
  
  GtkWidget *pFrame = gtk_frame_new(szFrameTitle);

  delete[] szFrameTitle;

  gtk_container_add(GTK_CONTAINER(pDasherControl->window.vbox), pFrame);
  g_object_set(G_OBJECT(pFrame), "border-width", 8, NULL);

  GtkWidget *pTable = gtk_table_new(iCount, 2, FALSE);
  g_object_set(G_OBJECT(pTable), "border-width", 8, NULL);
  gtk_container_add(GTK_CONTAINER(pFrame), pTable);
  
 
  for(int i(0); i < iCount; ++i) {
    GtkWidget *pControl;
    
    if(pSettings[i].iType == T_BOOL) {
      pControl = gtk_check_button_new_with_label(pSettings[i].szDescription); 
      
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pControl), 
				   dasher_app_settings_get_bool(pPrivate->pAppSettings, pSettings[i].iParameter)); 
      
      gtk_table_attach_defaults(GTK_TABLE(pTable), pControl, 0, 2, i, i+1);
      g_signal_connect(G_OBJECT(pControl), "toggled", G_CALLBACK(handle_bool_changed), pDasherControl);
      
    }
    else {
      GtkWidget *pLabel = gtk_label_new(pSettings[i].szDescription);
      gtk_misc_set_alignment(GTK_MISC(pLabel), 0, 0.5);
      gtk_table_attach_defaults(GTK_TABLE(pTable), pLabel, 0, 1, i, i+1);
      
      switch(pSettings[i].iType) {
      case T_LONG:
	pControl = gtk_hscale_new_with_range(pSettings[i].iMin / static_cast<double>(pSettings[i].iDivisor), 
					     pSettings[i].iMax / static_cast<double>(pSettings[i].iDivisor), 
					     pSettings[i].iStep / static_cast<double>(pSettings[i].iDivisor));
	
	gtk_range_set_value(GTK_RANGE(pControl), 
			    dasher_app_settings_get_long(pPrivate->pAppSettings, pSettings[i].iParameter) 
			    / static_cast<double>(pSettings[i].iDivisor));
	
	gtk_widget_set_size_request(pControl, 256, -1);
	
	g_signal_connect(G_OBJECT(pControl), "value-changed", G_CALLBACK(handle_long_changed), pDasherControl);
	break;
      case T_LONGSPIN:
      	pControl = gtk_spin_button_new_with_range(pSettings[i].iMin / static_cast<double>(pSettings[i].iDivisor), 
						  pSettings[i].iMax / static_cast<double>(pSettings[i].iDivisor), 
						  pSettings[i].iStep / static_cast<double>(pSettings[i].iDivisor));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(pControl), 
				  dasher_app_settings_get_long(pPrivate->pAppSettings, pSettings[i].iParameter) 
				  / static_cast<double>(pSettings[i].iDivisor));

	gtk_widget_set_size_request(pControl, 256, -1);	

	g_signal_connect(G_OBJECT(pControl), "value-changed", G_CALLBACK(handle_longspin_changed), pDasherControl);
	break;
      case T_STRING:
	pControl = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(pControl),
			   dasher_app_settings_get_string(pPrivate->pAppSettings, pSettings[i].iParameter));
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

ModuleSettingsData *module_settings_window_get_settings_data(ModuleSettingsWindow *pSelf, GtkWidget *pWidget) {
  ModuleSettingsWindowPrivate *pPrivate((ModuleSettingsWindowPrivate *)(pSelf->private_data));

  ModuleSettingsData *pCurrentData = pPrivate->pFirst;

  while(pCurrentData != NULL) {
    if(pCurrentData->pWidget == pWidget)
      return pCurrentData;

    pCurrentData = pCurrentData->pNext;
  }

  return NULL;
}

void module_settings_window_handle_bool_changed(ModuleSettingsWindow *pSelf, GtkToggleButton *pToggleButton) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pToggleButton));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate((ModuleSettingsWindowPrivate *)(pSelf->private_data));
  
  gboolean bNewValue = gtk_toggle_button_get_active(pToggleButton);
  dasher_app_settings_set_bool(pPrivate->pAppSettings, pData->iParameter, bNewValue);
}

void module_settings_window_handle_long_changed(ModuleSettingsWindow *pSelf, GtkRange *pRange) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pRange));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate((ModuleSettingsWindowPrivate *)(pSelf->private_data));

  gint iNewValue = gtk_range_get_value(pRange) * pData->iDivisor;
  dasher_app_settings_set_long(pPrivate->pAppSettings, pData->iParameter, iNewValue);
}

void module_settings_window_handle_longspin_changed(ModuleSettingsWindow *pSelf, GtkSpinButton *pSpinButton) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pSpinButton));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate((ModuleSettingsWindowPrivate *)(pSelf->private_data));

  gint iNewValue = gtk_spin_button_get_value(pSpinButton) * pData->iDivisor;
  dasher_app_settings_set_long(pPrivate->pAppSettings, pData->iParameter, iNewValue);
}

void module_settings_window_handle_string_changed(ModuleSettingsWindow *pSelf, GtkEditable *pEditable) {
  ModuleSettingsData *pData = module_settings_window_get_settings_data(pSelf, GTK_WIDGET(pEditable));

  if(!pData)
    return;

  ModuleSettingsWindowPrivate *pPrivate((ModuleSettingsWindowPrivate *)(pSelf->private_data));

  const gchar *szNewValue = gtk_editable_get_chars(pEditable, 0, -1);
  dasher_app_settings_set_string(pPrivate->pAppSettings, pData->iParameter, szNewValue);
}

gboolean module_settings_window_handle_close(ModuleSettingsWindow *pSelf) {
  gtk_widget_hide(GTK_WIDGET(&(pSelf->window)));
  return TRUE;
}

// Callbacks

extern "C" void handle_bool_changed(GtkToggleButton *pToggleButton, gpointer pUserData) {
  module_settings_window_handle_bool_changed(MODULE_SETTINGS_WINDOW(pUserData), pToggleButton);
}

extern "C" void handle_long_changed(GtkRange *pRange, gpointer pUserData) {
  module_settings_window_handle_long_changed(MODULE_SETTINGS_WINDOW(pUserData), pRange);
}

extern "C" void handle_longspin_changed(GtkSpinButton *pSpinButton, gpointer pUserData) {
  module_settings_window_handle_longspin_changed(MODULE_SETTINGS_WINDOW(pUserData), pSpinButton);
}

extern "C" void handle_string_changed(GtkEditable *pEditable, gpointer pUserData) {
  module_settings_window_handle_string_changed(MODULE_SETTINGS_WINDOW(pUserData), pEditable);
}

extern "C" gboolean handle_close(GtkWidget *pWidget, gpointer pUserData) {
  return module_settings_window_handle_close(MODULE_SETTINGS_WINDOW(pUserData));
}

extern "C" gboolean handle_close_event(GtkWidget *pWidget, GdkEvent *pEvent, gpointer pUserData) {
  return module_settings_window_handle_close(MODULE_SETTINGS_WINDOW(pUserData));
}
