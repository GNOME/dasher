#ifndef __dasher_app_settings_h__
#define __dasher_app_settings_h__

#include <glib.h>
#include <glib-object.h>

#include "../DasherCore/Parameters.h"
#include "../Common/AppSettingsHeader.h"
#include "GtkDasherControl.h"

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

enum {
  DASHER_TYPE_BOOL,
  DASHER_TYPE_LONG,
  DASHER_TYPE_STRING
};

G_BEGIN_DECLS
#define TYPE_DASHER_APP_SETTINGS            (dasher_app_settings_get_type())
#define DASHER_APP_SETTINGS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_APP_SETTINGS, DasherAppSettings ))
#define DASHER_APP_SETTINGS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_APP_SETTINGS, DasherAppSettingsClass ))
#define IS_DASHER_APP_SETTINGS(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_APP_SETTINGS))
#define IS_DASHER_APP_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_APP_SETTINGS))
#define DASHER_APP_SETTINGS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_APP_SETTINGS, DasherAppSettingsClass))

typedef struct _DasherAppSettings DasherAppSettings;
typedef struct _DasherAppSettingsClass DasherAppSettingsClass;

struct _DasherAppSettings {
  GObject parent;
  gpointer private_data;
};

struct _DasherAppSettingsClass {
  GObjectClass parent_class;
};

DasherAppSettings *dasher_app_settings_new(int argc, char **argv);
GType dasher_app_settings_get_type();

void dasher_app_settings_reset(DasherAppSettings *pSelf, int iParameter);
void dasher_app_settings_set_widget(DasherAppSettings *pSelf, GtkDasherControl *pWidget);

bool dasher_app_settings_get_bool(DasherAppSettings *pSelf, int iParameter);
void dasher_app_settings_set_bool(DasherAppSettings *pSelf, int iParameter, bool bValue);
gint dasher_app_settings_get_long(DasherAppSettings *pSelf, int iParameter);
void dasher_app_settings_set_long(DasherAppSettings *pSelf, int iParameter, gint iValue);
gboolean dasher_app_settings_get_free_long(DasherAppSettings *pSelf, const gchar *szName, gint &iValue);
void dasher_app_settings_set_free_long(DasherAppSettings *pSelf, const gchar *szName, gint iValue);
const gchar *dasher_app_settings_get_string(DasherAppSettings *pSelf, int iParameter);
void dasher_app_settings_set_string(DasherAppSettings *pSelf, int iParameter, const gchar *szValue);

int dasher_app_settings_get_count(DasherAppSettings *pSelf);
int dasher_app_settings_get_parameter_type(DasherAppSettings *pSelf, int iParameter);
const gchar *dasher_app_settings_get_reg_name(DasherAppSettings *pSelf, int iParameter);
const gchar *dasher_app_settings_get_human_name(DasherAppSettings *pSelf, int iParameter);
bool dasher_app_settings_have_advanced(DasherAppSettings *pSelf);
void dasher_app_settings_launch_advanced(DasherAppSettings *pSelf);

GArray *dasher_app_settings_get_allowed_values(DasherAppSettings *pSelf, int iParameter);
gboolean dasher_app_settings_get_module_settings(DasherAppSettings *pSelf, const gchar *szValue, SModuleSettings **pSettings, gint *iCount);

const gchar * dasher_app_settings_cl_set(DasherAppSettings *pSelf, const gchar *szKey, const gchar *szValue);

void option_help();
G_END_DECLS

#endif
