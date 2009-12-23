#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WITH_GCONF
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-enum-types.h>
#endif

#include <cstring>
#include "DasherAppSettings.h"
#include "../Common/AppSettingsData.h"

// FIXME - not sure that we're loading parameters from the registry at startup right now

// FIXME - shouldn't need to include this, but for now we need access to the global pointer to the dasher control.

#include "dasher.h"
#include "GtkDasherControl.h"

// FIXME - should really do something to make this a singleton class

// TODO: Rename this file to fit in with naming conventions

// TODO: Bring this into the widget
static GtkWidget *pDasherWidget = NULL;


struct _DasherAppSettingsPrivate {
#ifdef WITH_GCONF
  // GConf interface
  GConfClient *pGConfClient;
#endif
  GtkDasherControl *pWidget;
  gboolean bWidgetSet;
};

typedef struct _DasherAppSettingsPrivate DasherAppSettingsPrivate;

// Private member functions

static void dasher_app_settings_class_init(DasherAppSettingsClass *pClass);
static void dasher_app_settings_init(DasherAppSettings *pAppSettings);
static void dasher_app_settings_destroy(GObject *pObject);

static void dasher_app_settings_init_gconf(DasherAppSettings *pSelf, int argc, char **argv);
static void dasher_app_settings_stop_gconf(DasherAppSettings *pSelf);
static void dasher_app_settings_load(DasherAppSettings *pSelf);

// Function declarations

GType dasher_app_settings_get_type() {

  static GType dasher_app_settings_type = 0;

  if(!dasher_app_settings_type) {
    static const GTypeInfo dasher_app_settings_info = {
      sizeof(DasherAppSettingsClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_app_settings_class_init,
      NULL,
      NULL,
      sizeof(DasherAppSettings),
      0,
      (GInstanceInitFunc) dasher_app_settings_init,
      NULL
    };

    dasher_app_settings_type = g_type_register_static(G_TYPE_OBJECT, "DasherAppSettings", &dasher_app_settings_info, static_cast < GTypeFlags > (0));
  }

  return dasher_app_settings_type;
}

static void dasher_app_settings_class_init(DasherAppSettingsClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_app_settings_destroy;
}

static void dasher_app_settings_init(DasherAppSettings *pDasherControl) {
  DasherAppSettingsPrivate *pPrivate = new DasherAppSettingsPrivate;
  pDasherControl->private_data = pPrivate;

#ifdef WITH_GCONF
  pPrivate->pGConfClient = NULL;
#endif
  pPrivate->pWidget = NULL;
  pPrivate->bWidgetSet = FALSE;
}

static void dasher_app_settings_destroy(GObject *pObject) {
#ifdef WITH_GCONF
  dasher_app_settings_stop_gconf((DasherAppSettings *)pObject);
#endif

  for(int i(0); i < NUM_OF_APP_SPS; ++i)
    delete[] app_stringparamtable[i].value;

  // FIXME - glib routines?
  // FIXME - do we need a typecast here?
  delete (DasherAppSettingsPrivate *)(((DasherAppSettings *)pObject)->private_data);
  
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

void dasher_app_settings_set_widget(DasherAppSettings *pSelf, GtkDasherControl *pWidget) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);

  pDasherWidget = GTK_WIDGET(pWidget);

  pPrivate->pWidget = pWidget;
  pPrivate->bWidgetSet = TRUE;
}

static void dasher_app_settings_init_gconf(DasherAppSettings *pSelf, int argc, char **argv) {
#ifdef WITH_GCONF
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);

  GError *pGConfError;

  if(!gconf_init(argc, argv, &pGConfError))
    g_error("Failed to initialise gconf: %s", pGConfError->message);
  
    pPrivate->pGConfClient = gconf_client_get_default();
#endif
}

static void dasher_app_settings_stop_gconf(DasherAppSettings *pSelf) {
#ifdef WITH_GCONF
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);

  g_object_unref(pPrivate->pGConfClient);
#endif
}

static void dasher_app_settings_load(DasherAppSettings *pSelf) { 
#ifdef WITH_GCONF
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  GError *pGConfError = NULL;
  GConfValue *pGConfValue;
 
  for(int i(0); i < NUM_OF_APP_BPS; ++i ) {
    if(app_boolparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher4/", 256);
      strncat(szName,  app_boolparamtable[i].regName, 255 - strlen( szName ));

      pGConfValue = gconf_client_get_without_default(pPrivate->pGConfClient, szName, &pGConfError);
      
      if(pGConfValue) {
	app_boolparamtable[i].value = gconf_value_get_bool(pGConfValue);

	gconf_value_free(pGConfValue);
      }
    }
  }

  for(int i(0); i < NUM_OF_APP_LPS; ++i ) {
    if(app_longparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher4/", 256);
      strncat(szName,  app_longparamtable[i].regName, 255 - strlen( szName ));

      pGConfValue = gconf_client_get_without_default(pPrivate->pGConfClient, szName, &pGConfError);
      
      if(pGConfValue) {
	app_longparamtable[i].value = gconf_value_get_int(pGConfValue);

	gconf_value_free(pGConfValue);
      }
    }
  }

  for(int i(0); i < NUM_OF_APP_SPS; ++i ) {
    if(app_stringparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher4/", 256);
      strncat(szName,  app_stringparamtable[i].regName, 255 - strlen( szName ));

      pGConfValue = gconf_client_get_without_default(pPrivate->pGConfClient, szName, &pGConfError);
      
      if(pGConfValue) {
	delete[] app_stringparamtable[i].value;

	const gchar *szValue(gconf_value_get_string(pGConfValue));

	gchar *szNew;
	szNew = new gchar[strlen(szValue) + 1];
	strcpy(szNew, szValue);
	
	app_stringparamtable[i].value = szNew;
	gconf_value_free(pGConfValue);
      }
    }
  }
#endif
}

// Public methods

DasherAppSettings *dasher_app_settings_new(int argc, char **argv) {
  DasherAppSettings *pDasherControl;
  pDasherControl = (DasherAppSettings *)(g_object_new(dasher_app_settings_get_type(), NULL));

  for(int i(0); i < NUM_OF_APP_SPS; ++i) {
    gchar *szNew;
    szNew = new gchar[strlen(app_stringparamtable[i].szDefaultValue) + 1];
    strcpy(szNew, app_stringparamtable[i].szDefaultValue);
    app_stringparamtable[i].value = szNew;
  }
#ifdef WITH_GCONF
  dasher_app_settings_init_gconf(pDasherControl, argc, argv);  
#endif
  dasher_app_settings_load(pDasherControl);

  return pDasherControl;
}

void dasher_app_settings_reset(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);

  if(iParameter < END_OF_SPS) {
    if(pPrivate->bWidgetSet)
      gtk_dasher_control_reset_parameter(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
    return;
  }
  else {
    //    pre_parameter_notification(0, iParameter, 0);
    
    if(iParameter < END_OF_APP_BPS)
      app_boolparamtable[ iParameter - FIRST_APP_BP ].value = app_boolparamtable[ iParameter - FIRST_APP_BP ].bDefaultValue;
    else if(iParameter < END_OF_APP_LPS)
      app_longparamtable[ iParameter - FIRST_APP_LP ].value = app_longparamtable[ iParameter - FIRST_APP_LP ].iDefaultValue; 
    else {
      delete[] app_stringparamtable[iParameter - FIRST_APP_SP].value;
      
      gchar *szNew;
      szNew = new gchar[strlen(app_stringparamtable[iParameter - FIRST_APP_SP].szDefaultValue) + 1];
      strcpy(szNew, app_stringparamtable[iParameter - FIRST_APP_SP].szDefaultValue);
      app_stringparamtable[iParameter - FIRST_APP_SP].value = szNew;
    }
  }
  // TODO: Use real signals to achieve this
  parameter_notification(0, iParameter, 0);
}

bool dasher_app_settings_get_bool(DasherAppSettings *pSelf, int iParameter) { 
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);
  if( iParameter < END_OF_BPS ) {
    if(pPrivate->bWidgetSet)
      return gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
    else
      return false;
  }
  else
    return app_boolparamtable[ iParameter - FIRST_APP_BP ].value;
}

void dasher_app_settings_set_bool(DasherAppSettings *pSelf, int iParameter, bool bValue) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  if( iParameter < END_OF_BPS ) {
    if(pPrivate->bWidgetSet) {
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), iParameter, bValue);
    }
  }
  else {
    if(dasher_app_settings_get_bool(pSelf, iParameter) == bValue)
      return; // Don't attempt to change to the existing value

    app_boolparamtable[ iParameter - FIRST_APP_BP ].value = bValue;
#ifdef WITH_GCONF    
    if(app_boolparamtable[ iParameter - FIRST_APP_BP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher4/", 256);
      strncat(szName,  app_boolparamtable[ iParameter - FIRST_APP_BP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_bool(pPrivate->pGConfClient, szName, bValue, &pGConfError);

      if(pGConfError)
	g_message("Error");
    }
#endif

    // TODO: Use real signals to achieve this
    parameter_notification(0, iParameter, 0);
  }
}

gint dasher_app_settings_get_long(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);
 
  if( iParameter < END_OF_LPS) {
    if(pPrivate->bWidgetSet) 
      return gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
    else
      return false;
  }
  else
    return app_longparamtable[ iParameter - FIRST_APP_LP ].value;
}

void dasher_app_settings_set_long(DasherAppSettings *pSelf, int iParameter, gint iValue) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  if( iParameter < END_OF_LPS) {
    if(pPrivate->bWidgetSet)
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), iParameter, iValue);
  }
  else {
    if(dasher_app_settings_get_long(pSelf, iParameter) == iValue)
      return; // Don't attempt to change to the existing value
    
    //    pre_parameter_notification(0, iParameter, 0);

    app_longparamtable[ iParameter - FIRST_APP_LP ].value = iValue;

#ifdef WITH_GCONF    
    if(app_longparamtable[ iParameter - FIRST_APP_LP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher4/", 256);
      strncat(szName,  app_longparamtable[ iParameter - FIRST_APP_LP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_int(pPrivate->pGConfClient, szName, iValue, &pGConfError);
    }
#endif
    
    // TODO: Use real signals to achieve this
    parameter_notification(0, iParameter, 0);
  }
}

gboolean dasher_app_settings_get_free_long(DasherAppSettings *pSelf, const gchar *szName, gint &iValue) {
#ifdef WITH_GCONF
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  gchar szFullName[256];
      
  strncpy(szFullName, "/apps/dasher4/", 256);
  strncat(szFullName, szName, 255 - strlen(szFullName));

  GConfValue *pGConfValue;
  GError *pGConfError = NULL;

  pGConfValue = gconf_client_get_without_default(pPrivate->pGConfClient, szFullName, &pGConfError);
      
  if(pGConfValue) {
    iValue = gconf_value_get_int(pGConfValue);
    gconf_value_free(pGConfValue);
    return true;
  }
  else {
    return false;
  }
#else
  return false;
#endif
}

void dasher_app_settings_set_free_long(DasherAppSettings *pSelf, const gchar *szName, gint iValue) {   
#ifdef WITH_GCONF
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  gchar szFullName[256];
      
  strncpy(szFullName, "/apps/dasher4/", 256);
  strncat(szFullName, szName, 255 - strlen(szFullName));

  GError *pGConfError = NULL;
  gconf_client_set_int(pPrivate->pGConfClient, szFullName, iValue, &pGConfError);
#endif
}

const gchar *dasher_app_settings_get_string(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);
 
  if( iParameter < END_OF_SPS ) {
    if(pPrivate->bWidgetSet) 
      return gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
    else
      return false;
  }
  else
    return app_stringparamtable[ iParameter - FIRST_APP_SP ].value;
}

void dasher_app_settings_set_string(DasherAppSettings *pSelf, int iParameter, const gchar *szValue) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  if( iParameter < END_OF_SPS ) {
    if(pPrivate->bWidgetSet)
      gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter, szValue);
  }
  else {
    if(!strcmp(dasher_app_settings_get_string(pSelf, iParameter), szValue))
      return; // Don't attempt to change to the existing value

    //    pre_parameter_notification(0, iParameter, 0);
    
    delete[] app_stringparamtable[ iParameter - FIRST_APP_SP ].value;
    
    gchar *szNew;
    szNew = new gchar[strlen(szValue) + 1];
    strcpy(szNew, szValue);
    
    app_stringparamtable[ iParameter - FIRST_APP_SP ].value = szNew;
    
#ifdef WITH_GCONF
    if(app_stringparamtable[ iParameter - FIRST_APP_SP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher4/", 256);
      strncat(szName,  app_stringparamtable[ iParameter - FIRST_APP_SP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_string(pPrivate->pGConfClient, szName, szValue, &pGConfError);
    }
#endif    

    // TODO: Use real signals to achieve this
    parameter_notification(0, iParameter, 0);
  }
}

int dasher_app_settings_get_count(DasherAppSettings *pSelf) {
  return END_OF_APP_SPS;
}

int dasher_app_settings_get_parameter_type(DasherAppSettings *pSelf, int iParameter) {
  if(iParameter < END_OF_BPS)
    return DASHER_TYPE_BOOL;
  else if(iParameter < END_OF_LPS)
    return DASHER_TYPE_LONG;
  else if(iParameter < END_OF_SPS)
    return DASHER_TYPE_STRING;
  else if(iParameter < END_OF_APP_BPS)
    return DASHER_TYPE_BOOL;
  else if(iParameter < END_OF_APP_LPS)
    return DASHER_TYPE_LONG;
  else 
    return DASHER_TYPE_STRING;
}

const gchar *dasher_app_settings_get_reg_name(DasherAppSettings *pSelf, int iParameter) {
  if(iParameter < END_OF_BPS)
    return boolparamtable[iParameter - FIRST_BP].regName;
  else if(iParameter < END_OF_LPS)
    return longparamtable[iParameter - FIRST_LP].regName;
  else if(iParameter < END_OF_SPS)
    return stringparamtable[iParameter - FIRST_SP].regName;
  else if(iParameter < END_OF_APP_BPS)
    return app_boolparamtable[iParameter - FIRST_APP_BP].regName;
  else if(iParameter < END_OF_APP_LPS)
    return app_longparamtable[iParameter - FIRST_APP_LP].regName;
  else 
    return app_stringparamtable[iParameter - FIRST_APP_SP].regName;
}

const gchar *dasher_app_settings_get_human_name(DasherAppSettings *pSelf, int iParameter) {
  if(iParameter < END_OF_BPS)
    return boolparamtable[iParameter - FIRST_BP].humanReadable;
  else if(iParameter < END_OF_LPS)
    return longparamtable[iParameter - FIRST_LP].humanReadable;
  else if(iParameter < END_OF_SPS)
    return stringparamtable[iParameter - FIRST_SP].humanReadable;
  else if(iParameter < END_OF_APP_BPS)
    return app_boolparamtable[iParameter - FIRST_APP_BP].humanReadable;
  else if(iParameter < END_OF_APP_LPS)
    return app_longparamtable[iParameter - FIRST_APP_LP].humanReadable;
  else 
    return app_stringparamtable[iParameter - FIRST_APP_SP].humanReadable;
}

bool dasher_app_settings_have_advanced(DasherAppSettings *pSelf) {
  return(g_find_program_in_path("gconf-editor") != NULL);
}

void dasher_app_settings_launch_advanced(DasherAppSettings *pSelf) {
  gchar *szArgs[3];

  szArgs[0] = "gconf-editor";
  szArgs[1] = "/apps/dasher4";
  szArgs[2] = NULL;

  GError *pError;

  if(!g_spawn_async(NULL, szArgs, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &pError)) {
    g_warning("Could not launch gconf-editor: %s", pError->message);
  }
}

void dasher_app_settings_set_widget(DasherAppSettings *pSelf, GtkWidget *pWidget) {
  pDasherWidget = pWidget;
}

GArray *dasher_app_settings_get_allowed_values(DasherAppSettings *pSelf, int iParameter) {
  return gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
}

gboolean dasher_app_settings_get_module_settings(DasherAppSettings *pSelf, const gchar *szValue, SModuleSettings **pSettings, gint *iCount) {
  return gtk_dasher_control_get_module_settings(GTK_DASHER_CONTROL(pDasherWidget), szValue, pSettings, iCount);
}

// Set the option szKey to szValue.  Return NULL if everything worked, a
// (literal) error string for unrecognized or illegal values.
const gchar *
dasher_app_settings_cl_set(DasherAppSettings *pSelf, const gchar *szKey, const gchar *szValue) {

  for(int i(0); i < NUM_OF_APP_BPS; ++i ) {
    if(!strcmp(app_boolparamtable[i].regName, szKey)) {
      
      if(!strcmp(szValue, "1") || !strcmp(szValue, _("true")))
        dasher_app_settings_set_bool(pSelf, app_boolparamtable[i].key, true);
      else if(!strcmp(szValue, "0") || !strcmp(szValue, _("false")))
        dasher_app_settings_set_bool(pSelf, app_boolparamtable[i].key, false);
      else
        return _("boolean value must be specified as 'true' or 'false'.");
      return 0;
    }
  }

  for(int i(0); i < NUM_OF_APP_LPS; ++i ) {
    if(!strcmp(app_longparamtable[i].regName, szKey)) {
      dasher_app_settings_set_long(pSelf, app_longparamtable[i].key, atoi(szValue));
      return 0;
    }
  }

  for(int i(0); i < NUM_OF_APP_SPS; ++i ) {
    if(!strcmp(app_stringparamtable[i].regName, szKey)) {
      dasher_app_settings_set_string(pSelf, app_stringparamtable[i].key, szValue);
      return 0;
    }
  }  

  return gtk_dasher_control_cl_set(GTK_DASHER_CONTROL(pDasherWidget), szKey, szValue);
}


void option_help()
{
  g_print("\n");
  g_print("%-30s %-12s  %s\n", _("Boolean parameters"), _("Default"), _("Description"));
  g_print("%-30s %-12s  %s\n", "------------------------------", "------------", "------------------------------");
  for(unsigned int i=0; i < sizeof(app_boolparamtable)/sizeof(app_boolparamtable[0]); ++i) {
    g_print("%-30s %-12s  %s\n", app_boolparamtable[i].regName, (app_boolparamtable[i].bDefaultValue ? _("true") : _("false")), app_boolparamtable[i].humanReadable);
  }

  for(unsigned int i = 0; i < sizeof(boolparamtable)/sizeof(boolparamtable[0]); i++) {
    g_print("%-30s %-12s  %s\n", boolparamtable[i].regName, (boolparamtable[i].defaultValue ? _("true") : _("false")), boolparamtable[i].humanReadable);
  }

  g_print("\n");
  g_print("%-30s %-12s  %s\n", _("Integer parameters"), _("Default"), _("Description"));
  g_print("%-30s %-12s  %s\n", "------------------------------", "------------", "------------------------------");
  for(unsigned int i=0; i < sizeof(app_longparamtable)/sizeof(app_longparamtable[0]); ++i) {
    g_print("%-30s %12li  %s\n", app_longparamtable[i].regName, app_longparamtable[i].iDefaultValue, app_longparamtable[i].humanReadable);
  }

  for(unsigned int i = 0; i < sizeof(longparamtable)/sizeof(longparamtable[0]); i++) {
    g_print("%-30s %12li  %s\n", longparamtable[i].regName, longparamtable[i].defaultValue, longparamtable[i].humanReadable);
  }

  g_print("\n");
  g_print("%-30s %-12s  %s\n", _("String parameters"), _("Default"), _("Description"));
  g_print("%-30s %-12s  %s\n", "------------------------------", "------------", "------------------------------");
  for(unsigned int i=0; i < sizeof(app_stringparamtable)/sizeof(app_stringparamtable[0]); ++i) {
    g_print("%-30s %-12s  %s\n", app_stringparamtable[i].regName, app_stringparamtable[i].szDefaultValue, app_stringparamtable[i].humanReadable);
  }

  for(unsigned int i = 0; i < sizeof(stringparamtable)/sizeof(stringparamtable[0]); i++) {
    g_print("%-30s %-12s  %s\n", stringparamtable[i].regName, stringparamtable[i].defaultValue, stringparamtable[i].humanReadable);
  }

}
