#include "AppSettings.h"
#include "dasher.h"

#include <string.h>



// First level structures with only basic data types because you
// cannot initialize struct tables with objects
// These will be turned into std::strings in the ParamTables() object 


// The only important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
app_bp_table app_boolparamtable[] = {
  {C_BP_DRAW_MOUSE_LINE, "DrawMouseLine", PERS, false, "Draw Mouse Line"},
  {C_BP_DRAW_MOUSE, "DrawMouse", PERS, true, "Draw Mouse Position"},
  {C_BP_SHOW_SLIDER, "ShowSpeedSlider", PERS, true, "ShowSpeedSlider"},
  {C_BP_START_MOUSE, "StartOnLeft", PERS, true, "StartOnLeft"},
  {C_BP_START_SPACE, "StartOnSpace", PERS, false, "StartOnSpace"},
  {C_BP_KEY_CONTROL, "KeyControl", PERS, false, "KeyControl"},
  {C_BP_CONTROL_MODE, "ControlMode", PERS, false, "ControlMode"},
  {C_BP_COLOUR_MODE, "ColourMode", PERS, true, "ColourMode"},
  {C_BP_MOUSEPOS_MODE, "StartOnMousePosition", PERS, false, "StartOnMousePosition"},
  {C_BP_OUTLINE_MODE, "OutlineBoxes", PERS, true, "OutlineBoxes"},
  {C_BP_PALETTE_CHANGE, "PaletteChange", PERS, true, "PaletteChange"},
  {C_BP_NUMBER_DIMENSIONS, "NumberDimensions", PERS, false, "NumberDimensions"},
  {C_BP_EYETRACKER_MODE, "EyetrackerMode", PERS, false, "EyetrackerMode"},
  {C_BP_AUTOCALIBRATE, "Autocalibrate", PERS, false, "Autocalibrate"},
  {C_BP_DASHER_PAUSED, "DasherPaused", !PERS, true, "Dasher Paused"},
  {C_BP_GAME_MODE, "GameMode", PERS, false, "Dasher Game Mode"},
  {C_BP_TRAINING, "Training", !PERS, false, "Provides locking during training"},
  {C_BP_REDRAW, "Redraw", !PERS, false, "Force a full redraw at the next timer event"},
  {C_BP_LM_DICTIONARY, "Dictionary", PERS, true, "Whether the word-based language model uses a dictionary"},
  {C_BP_LM_LETTER_EXCLUSION, "LetterExclusion", PERS, false, "Whether to do letter exclusion in the word-based model"},

  {APP_BP_TIME_STAMP, "TimeStampNewFiles", PERS, true, "TimeStampNewFiles"},
  {APP_BP_SHOW_TOOLBAR, "ViewToolbar", PERS, true, "ViewToolbar"},
  {APP_BP_SHOW_TOOLBAR_TEXT, "ShowToolbarText", PERS, true, "ShowToolbarText"},
  {APP_BP_SHOW_LARGE_ICONS, "ShowLargeIcons", PERS, true, "ShowLargeIcons"},
  {APP_BP_FIX_LAYOUT, "FixLayout", PERS, false, "FixLayout"},
  {APP_BP_COPY_ALL_ON_STOP, "CopyAllOnStop", PERS, false, "CopyAllOnStop"},
  {APP_BP_WINDOW_PAUSE, "PauseOutsideWindow", PERS, false, "PauseOutsideWindow"},
  {APP_BP_SPEECH_MODE, "SpeechEnabled", PERS, false, "SpeechEnabled"},
  {APP_BP_KEYBOARD_MODE, "KeyboardMode", PERS, false, "KeyboardMode"}
};

app_lp_table app_longparamtable[] = {
  {C_LP_ORIENTATION, "ScreenOrientation", PERS, 0, "Screen Orientation"},
  {C_LP_REAL_ORIENTATION, "RealOrientation", !PERS, 0, "Actual screen orientation (allowing for alphabet default)"},
  {C_LP_MAX_BITRATE, "MaxBitRateTimes100", PERS, 150, "Max Bit Rate Times 100"},
  {C_LP_VIEW_ID, "ViewID", PERS, 1, "ViewID"},
  {C_LP_LANGUAGE_MODEL_ID, "LanguageModelID", PERS, 1, "LanguageModelID"},
  {C_LP_DASHER_FONTSIZE, "DasherFontSize", PERS, 1, "DasherFontSize"},
  {C_LP_UNIFORM, "UniformTimes1000", PERS, 50, "UniformTimes1000"},
  {C_LP_YSCALE, "YScaling", PERS, 0, "YScaling"},
  {C_LP_MOUSEPOSDIST, "MousePositionBoxDistance", PERS, 50, "MousePositionBoxDistance"},
  {C_LP_TRUNCATION, "Truncation", PERS, 0, "Truncation"},
  {C_LP_TRUNCATIONTYPE, "TruncationType", PERS, 0, "TruncationType"},
  {C_LP_LM_MAX_ORDER, "LMMaxOrder", PERS, 5, "LMMaxOrder"},
  {C_LP_LM_EXCLUSION, "LMExclusion", PERS, 0, "LMExclusion"},
  {C_LP_LM_UPDATE_EXCLUSION, "LMUpdateExclusion", PERS, 1, "LMUpdateExclusion"},
  {C_LP_LM_ALPHA, "LMAlpha", PERS, 100, "LMAlpha"},
  {C_LP_LM_BETA, "LMBeta", PERS, 100, "LMBeta"},
  {C_LP_LM_MIXTURE, "LMMixture", PERS, 50, "LMMixture"},
  {C_LP_MOUSE_POS_BOX, "MousePosBox", !PERS, -1, "Mouse Position Box Indicator"},
  {C_LP_NORMALIZATION, "Normalization", !PERS, 1 << 16, "Interval for child nodes"},
  {C_LP_LINE_WIDTH, "LineWidth", PERS, 1, "Width to draw crosshair and mouse line"},
  {C_LP_LM_WORD_ALPHA, "WordAlpha", PERS, 50, "Alpha value for word-based model"},

  {APP_LP_FILE_ENCODING, "FileEncodingFormat", PERS, -2, "FileEncodingFormat"},
  {APP_LP_EDIT_FONT_SIZE, "EditFontSize", PERS, 1, "EditFontSize"},
  {APP_LP_EDIT_HEIGHT, "EditHeight", PERS, 75, "EditHeight0"},
  {APP_LP_SCREEN_WIDTH, "ScreenWidth", PERS, 400, "ScreenWidth"},
  {APP_LP_SCREEN_HEIGHT, "ScreenHeight", PERS, 500, "ScreenHeight"}

};

app_sp_table app_stringparamtable[] = {
  {C_SP_ALPHABET_ID, "AlphabetID", PERS, "", "AlphabetID"},
  {C_SP_COLOUR_ID, "ColourID", PERS, "", "ColourID"},
  {C_SP_DASHER_FONT, "DasherFont", PERS, "", "DasherFont"},
  {C_SP_SYSTEM_LOC, "SystemLocation", !PERS, "sys_", "System Directory"},
  {C_SP_USER_LOC, "UserLocation", !PERS, "usr_", "User Directory"},
  {C_SP_GAME_TEXT_FILE, "GameTextFile", !PERS, "", "File with strings to practice writing"},
  {C_SP_TRAIN_FILE, "TrainingFile", !PERS, "", "Training text for alphabet"},

  {APP_SP_EDIT_FONT, "EditFont", PERS, "", "EditFont"}
};

void handle_parameter_change( int iParameter );

bool get_app_parameter_bool( int iParameter ) {
  return app_boolparamtable[ iParameter - FIRST_APP_BP ].value;
}

void set_app_parameter_bool( int iParameter, bool bValue ) {
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

gint get_app_parameter_long( int iParameter ) {
  return app_longparamtable[ iParameter - FIRST_APP_LP ].value;
}

void set_app_parameter_long( int iParameter, gint iValue ) {
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

const gchar *get_app_parameter_string( int iParameter ) {
  return app_stringparamtable[ iParameter - FIRST_APP_SP ].value;
}

void set_app_parameter_string( int iParameter, const gchar *szValue ) {

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

}
