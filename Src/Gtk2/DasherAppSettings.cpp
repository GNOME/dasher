#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-enum-types.h>

#include "DasherAppSettings.h"
#include "../Common/AppSettingsData.h"

// FIXME - not sure that we're loading parameters from the registry at startup right now

// FIXME - shouldn't need to include this, but for now we need access to the global pointer to the dasher control.

#include "dasher.h"
#include "GtkDasherControl.h"

// FIXME - accessing global 'handle parameter change' function

#include "AppSettings.h"

// FIXME - should really do something to make this a singleton class

struct _DasherAppSettingsPrivate {
  // GConf interface
  GConfClient *pGConfClient;
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
  pDasherControl->private_data = new DasherAppSettingsPrivate;
}

static void dasher_app_settings_destroy(GObject *pObject) {
  dasher_app_settings_stop_gconf((DasherAppSettings *)pObject);

  for(int i(0); i < NUM_OF_APP_SPS; ++i)
    delete[] app_stringparamtable[i].value;

  // FIXME - glib routines?
  // FIXME - do we need a typecast here?
  delete (DasherAppSettingsPrivate *)(((DasherAppSettings *)pObject)->private_data);
  
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

static void dasher_app_settings_init_gconf(DasherAppSettings *pSelf, int argc, char **argv) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);

  GError *pGConfError;

  if(!gconf_init(argc, argv, &pGConfError)) {
    // FIXME - need proper error reporting
    // std::cerr << "Failed to initialise gconf: " << pGConfError->message << std::endl;
    exit(1);
  }

  // FIXME - apparently there's a function gnome_gconf_get_client - maybe we should use this if building with gnome
  
  pPrivate->pGConfClient = gconf_client_get_default();
}

static void dasher_app_settings_stop_gconf(DasherAppSettings *pSelf) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate*)(pSelf->private_data);

  g_object_unref(pPrivate->pGConfClient);
}

static void dasher_app_settings_load(DasherAppSettings *pSelf) { 
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  GError *pGConfError = NULL;
  GConfValue *pGConfValue;
 
  for(int i(0); i < NUM_OF_APP_BPS; ++i ) {
    if(app_boolparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher/", 256);
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
    
      strncpy(szName, "/apps/dasher/", 256);
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
    
      strncpy(szName, "/apps/dasher/", 256);
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

  dasher_app_settings_init_gconf(pDasherControl, argc, argv);  
  dasher_app_settings_load(pDasherControl);

  return pDasherControl;
}

void dasher_app_settings_reset(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  if(iParameter < END_OF_SPS) {
    gtk_dasher_control_reset_parameter(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
    return;
  }
  else if(iParameter < END_OF_APP_BPS)
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
  
  handle_parameter_change(iParameter);
}

bool dasher_app_settings_get_bool(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

  if( iParameter < END_OF_BPS )
    return gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
  else
    return app_boolparamtable[ iParameter - FIRST_APP_BP ].value;
}

void dasher_app_settings_set_bool(DasherAppSettings *pSelf, int iParameter, bool bValue) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

    if( iParameter < END_OF_BPS )
    gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), iParameter, bValue);
  else {
    app_boolparamtable[ iParameter - FIRST_APP_BP ].value = bValue;
    
    if(app_boolparamtable[ iParameter - FIRST_APP_BP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_boolparamtable[ iParameter - FIRST_APP_BP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_bool(pPrivate->pGConfClient, szName, bValue, &pGConfError);
    }
    handle_parameter_change( iParameter );
  }
}

gint dasher_app_settings_get_long(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

   if( iParameter < END_OF_LPS)
    return gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
  else
    return app_longparamtable[ iParameter - FIRST_APP_LP ].value;

}

void dasher_app_settings_set_long(DasherAppSettings *pSelf, int iParameter, gint iValue) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

    if( iParameter < END_OF_LPS)
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), iParameter, iValue);
  else {
    app_longparamtable[ iParameter - FIRST_APP_LP ].value = iValue;
    
    if(app_longparamtable[ iParameter - FIRST_APP_LP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_longparamtable[ iParameter - FIRST_APP_LP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_int(pPrivate->pGConfClient, szName, iValue, &pGConfError);
    }
    
    handle_parameter_change( iParameter );
  }
}

const gchar *dasher_app_settings_get_string(DasherAppSettings *pSelf, int iParameter) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

 if( iParameter < END_OF_SPS )
    return gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
  else
    return app_stringparamtable[ iParameter - FIRST_APP_SP ].value;
}

void dasher_app_settings_set_string(DasherAppSettings *pSelf, int iParameter, const gchar *szValue) {
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

   if( iParameter < END_OF_SPS )
    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter, szValue);
  else {
    
    delete[] app_stringparamtable[ iParameter - FIRST_APP_SP ].value;
    
    gchar *szNew;
    szNew = new gchar[strlen(szValue) + 1];
    strcpy(szNew, szValue);
    
    app_stringparamtable[ iParameter - FIRST_APP_SP ].value = szNew;
    
    if(app_stringparamtable[ iParameter - FIRST_APP_SP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_stringparamtable[ iParameter - FIRST_APP_SP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_string(pPrivate->pGConfClient, szName, szValue, &pGConfError);
    }
    
    handle_parameter_change( iParameter );
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
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

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
  DasherAppSettingsPrivate *pPrivate = (DasherAppSettingsPrivate *)(pSelf->private_data);

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
