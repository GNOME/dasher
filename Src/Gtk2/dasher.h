#ifndef DASHER_H
#define DASHER_H

#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include "libdasher.h"

extern "C" void open_file (const char *filename);
extern "C" void clipboard_copy(void);
extern "C" void clipboard_cut(void);
extern "C" void clipboard_paste(void);
extern "C" void clipboard_copy_all(void);
extern "C" void clipboard_select_all(void);
extern "C" void reset_fonts(GtkWidget *widget, gpointer user_data);
extern "C" bool ask_save_before_exit(GtkWidget *widget, gpointer user_data);
extern "C" void preferences_display(GtkWidget *widget, gpointer user_data);
extern "C" gboolean preferences_hide(GtkWidget *widget, gpointer user_data);
extern "C" void generate_preferences(GtkWidget *widget, gpointer user_data);
extern "C" void orientation(GtkRadioButton *widget, gpointer user_data);
extern "C" void set_dasher_fontsize(GtkWidget *widget, gpointer user_data);
extern "C" void show_toolbar(GtkWidget *widget, gpointer user_data);
extern "C" void show_slider(GtkWidget *widget, gpointer user_data);
extern "C" void timestamp_files(GtkWidget *widget, gpointer user_data);
extern "C" void copy_all_on_stop(GtkWidget *widget, gpointer user_data);
extern "C" void file_encoding(GtkWidget *widget, gpointer user_data);
extern "C" void about_dasher(GtkWidget *widget, gpointer user_data);
extern "C" void DrawMouse(GtkWidget *widget, gpointer user_data);
extern "C" void SetDimension(GtkWidget *widget, gpointer user_data);
extern "C" void SetEyetracker(GtkWidget *widget, gpointer user_data);
extern "C" void startonleft(GtkWidget *widget, gpointer user_data);
extern "C" void startonspace(GtkWidget *widget, gpointer user_data);
extern "C" void startonmousepos(GtkWidget *widget, gpointer user_data);
extern "C" void keycontrol(GtkWidget *widget, gpointer user_data);
extern "C" void windowpause(GtkWidget *widget, gpointer user_data);
extern "C" void controlmode(GtkWidget *widget, gpointer user_data);
extern "C" void keyboardmode(GtkWidget *widget, gpointer user_data);
extern "C" void select_open_file(GtkWidget *widget, gpointer user_data);
extern "C" void select_new_file(GtkWidget *widget, gpointer user_data);
extern "C" void select_save_file_as(GtkWidget *widget, gpointer user_data);
extern "C" void select_save_file_as_and_quit(GtkWidget *widget, gpointer user_data);
extern "C" void select_append_file(GtkWidget *widget, gpointer user_data);
extern "C" void save_file(GtkWidget *widget, gpointer user_data);
extern "C" void save_file_and_quit(GtkWidget *widget, gpointer user_data);
extern "C" void select_import_file(GtkWidget *widget, gpointer user_data);
extern "C" void save_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector );
extern "C" void save_file_from_filesel_and_quit ( GtkWidget *selector2, GtkFileSelection *selector );
extern "C" void filesel_hide(GtkWidget *widget, gpointer user_data);
extern "C" void edit_filesel_hide(GtkWidget *widget, gpointer user_data);
extern "C" void speak(GtkWidget *widget, gpointer user_data);
extern "C" void outlineboxes(GtkWidget *widget, gpointer user_data);
extern "C" void mouseposstart_y_changed(GtkRange *widget, gpointer user_data);
extern "C" void y_scale_changed(GtkRange *widget, gpointer user_data);

void interface_setup(GladeXML *xml);
void interface_late_setup();
void open_window(GladeXML *xml);
void interface_cleanup();
void dasher_clear();
void dasher_start();
void dasher_redraw();

extern "C" void choose_filename();
extern "C" void uniform_changed(GtkHScale *hscale);

void parameter_string_callback( string_param p, const char *value );
void parameter_double_callback( double_param p, double value );
void parameter_int_callback( int_param p, long int value );
void parameter_bool_callback( bool_param p, bool value );


extern GtkWidget *vbox, *toolbar;
extern GdkPixbuf *p;
extern GtkWidget *pw;
//Gtk2DasherEdit *dasher_text_view;
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
//Gtk2DasherCanvas *dasher_canvas;
//Gtk2DasherPane *dasher_pane;
//CDasherInterface *interface;
extern GtkAccelGroup *dasher_accel;
extern GtkWidget *dasher_menu_bar;

extern bool controlmodeon;
extern bool keyboardmodeon;

extern "C" void set_dasher_font(GtkWidget *widget, gpointer user_data);
extern "C" void get_font_from_dialog( GtkWidget *one, GtkWidget *two );
extern "C" void set_edit_font(GtkWidget *widget, gpointer user_data);
extern "C" void get_edit_font_from_dialog( GtkWidget *one, GtkWidget *two );

void stop();

struct button {
  long x;
  long y;
};

void scan_alphabet_files();
void scan_colour_files();
void update_colours();
void update_alphabets();

int alphabet_filter(const gchar* filename);
int colour_filter(const gchar* filename);

#ifdef WITH_GPE
#define DASHERFONT "Serif 10"
#else
#define DASHERFONT "Serif 12"
#endif

#endif




