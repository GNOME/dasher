#ifndef __appsettings_h__
#define __appsettings_h__

#include <gtk/gtk.h>
#include "../DasherCore/Parameters.h"

enum { 
  APP_BP_TIME_STAMP = END_OF_SPS, APP_BP_SHOW_TOOLBAR,
  APP_BP_SHOW_TOOLBAR_TEXT, APP_BP_SHOW_LARGE_ICONS, APP_BP_FIX_LAYOUT,
  APP_BP_COPY_ALL_ON_STOP,APP_BP_WINDOW_PAUSE,APP_BP_SPEECH_MODE, 
  APP_BP_KEYBOARD_MODE, END_OF_APP_BPS
};

enum { 
  APP_LP_FILE_ENCODING = END_OF_APP_BPS, APP_LP_EDIT_FONT_SIZE, APP_LP_EDIT_HEIGHT,
  APP_LP_SCREEN_WIDTH, APP_LP_SCREEN_HEIGHT, END_OF_APP_LPS
};

enum {
  APP_SP_EDIT_FONT = END_OF_APP_LPS, END_OF_APP_SPS
};

// Define first int value of the first element of each type.
// Useful for offsetting into specific arrays,
// since each setting is a unique int, but all 3 arrays start at 0
#define FIRST_APP_BP END_OF_SPS
#define FIRST_APP_LP END_OF_APP_BPS
#define FIRST_APP_SP END_OF_APP_LPS

// Define the number of each type of setting
#define NUM_OF_APP_BPS (END_OF_APP_BPS - END_OF_SPS)
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

void init_app_settings();
void delete_app_settings();
void load_app_parameters();

bool get_app_parameter_bool( int iParameter );
void set_app_parameter_bool( int iParameter, bool bValue );
gint get_app_parameter_long( int iParameter );
void set_app_parameter_long( int iParameter, gint iValue );
const gchar *get_app_parameter_string( int iParameter );
void set_app_parameter_string( int iParameter, const gchar *szValue );

void handle_core_change(int iParameter);

#endif
