#include "../Common/Common.h"

#include "DasherAppSettings.h"
#include "dasher.h"
#include "GtkDasherControl.h"
#include "Preferences.h"

#include <string.h>

#include <iostream>

#include "AppSettings.h"

// Hmm... very pointless function:

void handle_core_change(int iParameter) {
  handle_parameter_change(iParameter);
}

void handle_parameter_change( int iParameter ) {
  // TODO - pass this to individual components? Do we want a full event framework?

  switch( iParameter ) {
  case APP_BP_SHOW_TOOLBAR: // TODO - update UI widget here
    if( dasher_app_settings_get_bool(g_pDasherAppSettings, APP_BP_SHOW_TOOLBAR))
      gtk_widget_show( toolbar );
    else
      gtk_widget_hide( toolbar );
    break;
  }

  update_advanced(iParameter);
  preferences_handle_parameter_change(iParameter);
  main_handle_parameter_change(iParameter);
}
