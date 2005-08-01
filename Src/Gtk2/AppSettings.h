#ifndef __appsettings_h__
#define __appsettings_h__

#include <gtk/gtk.h>
#include "../DasherCore/Parameters.h"
#include "../Common/AppSettingsHeader.h"

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
