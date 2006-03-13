#ifndef DASHER_H
#define DASHER_H

// New stuff, but shouldn't need a shared header

#include "dasher_main.h"
#include "DasherAppSettings.h"
#include "dasher_buffer_set.h"
#include "dasher_editor.h"

extern DasherMain *g_pDasherMain;
extern DasherAppSettings *g_pDasherAppSettings;
extern IDasherBufferSet *g_pBufferSet;
extern DasherEditor *g_pEditor;

// Old stuff

#include <gtk/gtk.h>

#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include "fileops.h"

/* #include <gconf/gconf.h> */
/* #include <gconf/gconf-client.h> */
/* #include <gconf/gconf-enum-types.h> */

void interface_setup(GladeXML * xml);
void interface_late_setup();
void interface_cleanup();

void main_handle_parameter_change(int iParameter);

void RefreshWidget(int iParameter);

extern "C" void choose_filename();
extern "C" void uniform_changed(GtkHScale * hscale);

extern GtkWidget *vbox, *toolbar;
extern GdkPixbuf *p;
extern GtkWidget *pw;
extern GtkWidget *text_view;
extern GtkWidget *speed_frame;
extern GtkObject *speed_slider;
extern GtkScale *speed_hscale;
extern GtkWidget *text_scrolled_window;
extern GtkWidget *canvas_frame;
extern GtkWidget *ofilesel;
extern GtkWidget *ifilesel;
extern GtkWidget *afilesel;
extern GtkStyle *style;
extern GtkAccelGroup *dasher_accel;
extern GtkWidget *dasher_menu_bar;
extern const gchar *filename;

//extern gboolean quitting;

extern gboolean file_modified;
extern GtkWidget *pDasherWidget;

extern GtkWidget *window;
extern GtkWidget *g_pHiddenWindow;

GdkFilterReturn dasher_discard_take_focus_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data);

extern gboolean timestamp;

// New Stuff

void InitialiseMainWindow(int argc, char **argv, GladeXML * pGladeXML);
void SaveWindowState();

extern const char *g_szAccessibleContext;

extern int g_iExpectedPosition;
extern int g_iOldPosition;

// ---

#ifdef WITH_GPE
#define DASHERFONT "Serif 10"
#else
#define DASHERFONT "Serif 12"
#endif

#endif
