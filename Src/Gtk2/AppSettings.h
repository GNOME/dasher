#ifndef __appsettings_h__
#define __appsettings_h__

#include <gtk/gtk.h>

enum { 
  C_BP_DRAW_MOUSE_LINE, C_BP_DRAW_MOUSE, 
  C_BP_SHOW_SLIDER,  C_BP_START_MOUSE,
  C_BP_START_SPACE, C_BP_KEY_CONTROL, 
  C_BP_CONTROL_MODE, C_BP_COLOUR_MODE, C_BP_MOUSEPOS_MODE,
  C_BP_OUTLINE_MODE, C_BP_PALETTE_CHANGE, C_BP_NUMBER_DIMENSIONS,
  C_BP_EYETRACKER_MODE, C_BP_AUTOCALIBRATE, C_BP_DASHER_PAUSED,
  C_BP_GAME_MODE, C_BP_TRAINING, C_BP_REDRAW, C_BP_LM_DICTIONARY, 
  C_BP_LM_LETTER_EXCLUSION, 

  APP_BP_TIME_STAMP, APP_BP_SHOW_TOOLBAR,
  APP_BP_SHOW_TOOLBAR_TEXT, APP_BP_SHOW_LARGE_ICONS, APP_BP_FIX_LAYOUT,APP_BP_COPY_ALL_ON_STOP,APP_BP_WINDOW_PAUSE,APP_BP_SPEECH_MODE, APP_BP_KEYBOARD_MODE, END_OF_APP_BPS
};

enum { 
  C_LP_ORIENTATION = END_OF_APP_BPS, C_LP_REAL_ORIENTATION, C_LP_MAX_BITRATE, 
  C_LP_VIEW_ID, C_LP_LANGUAGE_MODEL_ID,  C_LP_DASHER_FONTSIZE,
  C_LP_UNIFORM, C_LP_YSCALE, C_LP_MOUSEPOSDIST, C_LP_TRUNCATION,
  C_LP_TRUNCATIONTYPE, C_LP_LM_MAX_ORDER, C_LP_LM_EXCLUSION,
  C_LP_LM_UPDATE_EXCLUSION, C_LP_LM_ALPHA, C_LP_LM_BETA,
  C_LP_LM_MIXTURE, C_LP_MOUSE_POS_BOX, C_LP_NORMALIZATION, C_LP_LINE_WIDTH, 
  C_LP_LM_WORD_ALPHA, 


  APP_LP_FILE_ENCODING,APP_LP_EDIT_FONT_SIZE, APP_LP_EDIT_HEIGHT,
  APP_LP_SCREEN_WIDTH, APP_LP_SCREEN_HEIGHT, END_OF_APP_LPS
};

enum {
  C_SP_ALPHABET_ID = END_OF_APP_LPS, C_SP_COLOUR_ID, C_SP_DASHER_FONT,
   C_SP_SYSTEM_LOC, C_SP_USER_LOC, C_SP_GAME_TEXT_FILE,
  C_SP_TRAIN_FILE, 

  APP_SP_EDIT_FONT, END_OF_APP_SPS
};

// Define first int value of the first element of each type.
// Useful for offsetting into specific arrays,
// since each setting is a unique int, but all 3 arrays start at 0
#define FIRST_APP_BP 0
#define FIRST_APP_LP END_OF_APP_BPS
#define FIRST_APP_SP END_OF_APP_LPS

// Define the number of each type of setting
#define NUM_OF_APP_BPS END_OF_APP_BPS
#define NUM_OF_APP_LPS (END_OF_APP_LPS - END_OF_APP_BPS)
#define NUM_OF_APP_SPS (END_OF_APP_SPS - END_OF_APP_LPS)

#define PERS true


struct app_bp_table {
  int key;
  char *regName;
  bool persistent;
  bool value;
  char *humanReadable;
};
struct app_lp_table {
  int key;
  char *regName;
  bool persistent;
  long value;
  char *humanReadable;
};
struct app_sp_table {
  int key;
  char *regName;
  bool persistent;
  char *value;
  char *humanReadable;
};

extern app_bp_table app_boolparamtable[];
extern app_lp_table app_longparamtable[];
extern app_sp_table app_stringparamtable[];

void load_app_parameters();

bool get_app_parameter_bool( int iParameter );
void set_app_parameter_bool( int iParameter, bool bValue );
gint get_app_parameter_long( int iParameter );
void set_app_parameter_long( int iParameter, gint iValue );
const gchar *get_app_parameter_string( int iParameter );
void set_app_parameter_string( int iParameter, const gchar *szValue );

#endif
