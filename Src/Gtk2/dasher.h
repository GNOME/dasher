#ifndef DASHER_H
#define DASHER_H

#include <gtk/gtk.h>

#include "dasher_editor.h"
#include "dasher_main.h"
#include "DasherAppSettings.h"
#include "GtkDasherControl.h"
#include "Preferences.h"

// Global objects - eventually to be made members of the appropriate classes
extern DasherMain *g_pDasherMain;
extern DasherAppSettings *g_pDasherAppSettings;
extern DasherEditor *g_pEditor;
extern DasherPreferencesDialogue *g_pPreferencesDialogue;

// Misc. objects - eventually these need to be figured out and moved to the right places
extern const gchar *filename;
extern GtkWidget *the_text_view;
extern GtkTextBuffer *the_text_buffer;
extern gboolean file_modified;
extern GtkWidget *pDasherWidget;
extern GtkWidget *window;
extern GtkWidget *g_pHiddenWindow;

//GdkFilterReturn dasher_discard_take_focus_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data); 
extern "C" void parameter_notification(GtkDasherControl *pDasherControl, gint iParameter, gpointer data); 
extern "C" void pre_parameter_notification(GtkDasherControl *pDasherControl, gint iParameter, gpointer data); 
extern "C" void preferences_display(GtkWidget *widget, gpointer user_data);

// TODO: Reimplement
/* #ifdef WITH_GPE */
/* #define DASHERFONT "Serif 10" */
/* #else */
/* #define DASHERFONT "Serif 12" */
/* #endif */

#endif
