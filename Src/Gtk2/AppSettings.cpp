#include "AppSettings.h"
#include "dasher.h"
#include "GtkDasherControl.h"
#include "Preferences.h"

#include <string.h>

#include <iostream>

#include "../Common/AppSettingsData.h"

void handle_parameter_change( int iParameter );

void init_app_settings() {
}

void delete_app_settings() {
}

bool get_app_parameter_bool( int iParameter ) {
  if( iParameter < END_OF_BPS )
    return gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
  else
    return app_boolparamtable[ iParameter - FIRST_APP_BP ].value;
}

void set_app_parameter_bool( int iParameter, bool bValue ) {
  if( iParameter < END_OF_BPS )
    gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), iParameter, bValue);
  else {
    app_boolparamtable[ iParameter - FIRST_APP_BP ].value = bValue;
    
    if(app_boolparamtable[ iParameter - FIRST_APP_BP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_boolparamtable[ iParameter - FIRST_APP_BP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_bool(g_pGConfClient, szName, bValue, &pGConfError);
    }
    handle_parameter_change( iParameter );
  }
}

gint get_app_parameter_long( int iParameter ) { 
  if( iParameter < END_OF_LPS)
    return gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
  else
    return app_longparamtable[ iParameter - FIRST_APP_LP ].value;
}

void set_app_parameter_long( int iParameter, gint iValue ) {
  if( iParameter < END_OF_LPS)
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), iParameter, iValue);
  else {
    app_longparamtable[ iParameter - FIRST_APP_LP ].value = iValue;
    
    if(app_longparamtable[ iParameter - FIRST_APP_LP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_longparamtable[ iParameter - FIRST_APP_LP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_int(g_pGConfClient, szName, iValue, &pGConfError);
    }
    
    handle_parameter_change( iParameter );
  }
}

const gchar *get_app_parameter_string( int iParameter ) {
  if( iParameter < END_OF_SPS )
    return gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter);
  else
    return app_stringparamtable[ iParameter - FIRST_APP_SP ].value;
}

void set_app_parameter_string( int iParameter, const gchar *szValue ) {
  if( iParameter < END_OF_SPS )
    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter, szValue);
  else {
    
    // FIXME - free old string?
    
    gchar *szNew;
    szNew = new gchar[strlen(szValue) + 1];
    strcpy(szNew, szValue);
    
    app_stringparamtable[ iParameter - FIRST_APP_SP ].value = szNew;
    
    if(app_stringparamtable[ iParameter - FIRST_APP_SP ].persistent) {
      gchar szName[256];
      
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_stringparamtable[ iParameter - FIRST_APP_SP ].regName, 255 - strlen( szName ));
      
      GError *pGConfError = NULL;
      gconf_client_set_string(g_pGConfClient, szName, szValue, &pGConfError);
    }
    
    handle_parameter_change( iParameter );
  }
}


void load_app_parameters() {
  GError *pGConfError = NULL;
  GConfValue *pGConfValue;
 
  for(int i(0); i < NUM_OF_APP_BPS; ++i ) {
    if(app_boolparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_boolparamtable[i].regName, 255 - strlen( szName ));

      pGConfValue = gconf_client_get_without_default(g_pGConfClient, szName, &pGConfError);
      
      if(pGConfValue)
	app_boolparamtable[i].value = gconf_value_get_bool(pGConfValue);

      gconf_value_free(pGConfValue);
    }
  }

  for(int i(0); i < NUM_OF_APP_LPS; ++i ) {
    if(app_longparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_longparamtable[i].regName, 255 - strlen( szName ));

      pGConfValue = gconf_client_get_without_default(g_pGConfClient, szName, &pGConfError);
      
      if(pGConfValue)
	app_longparamtable[i].value = gconf_value_get_int(pGConfValue);

      gconf_value_free(pGConfValue);
    }
  }

  for(int i(0); i < NUM_OF_APP_SPS; ++i ) {
    if(app_stringparamtable[i].persistent) {
      gchar szName[256];
    
      strncpy(szName, "/apps/dasher/", 256);
      strncat(szName,  app_stringparamtable[i].regName, 255 - strlen( szName ));

      pGConfValue = gconf_client_get_without_default(g_pGConfClient, szName, &pGConfError);
      
      if(pGConfValue) {
	// FIXME - Free old value?

	const gchar *szValue(gconf_value_get_string(pGConfValue));

	gchar *szNew;
	szNew = new gchar[strlen(szValue) + 1];
	strcpy(szNew, szValue);
	
	app_stringparamtable[i].value = szNew;
      }

      gconf_value_free(pGConfValue);
    }
  }
}

void handle_core_change(int iParameter) {
  if( iParameter < FIRST_LP )
    handle_parameter_change(iParameter);
  else if( iParameter < FIRST_SP )
    handle_parameter_change(iParameter);
  else
    handle_parameter_change(iParameter);
}

void handle_parameter_change( int iParameter ) {
  // TODO - pass this to individual components? Do we want a full event framework?

  switch( iParameter ) {
  case APP_BP_SHOW_TOOLBAR: // TODO - update UI widget here
    if( get_app_parameter_bool( APP_BP_SHOW_TOOLBAR ))
      gtk_widget_show( toolbar );
    else
      gtk_widget_hide( toolbar );
    break;
  }

  update_advanced(iParameter);

}

int GetParameterCount() {
  return END_OF_APP_SPS;
};

int GetParameterType(int iParameter) {
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

const gchar *GetParameterRegName(int iParameter) {
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

const gchar *GetParameterHumanName(int iParameter) {
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
