#ifndef DASHER_H
#define DASHER_H

#include <gdk/gdkkeysyms.h>

void clipboard_copy(void);
void clipboard_cut(void);
void clipboard_paste(void);
void clipboard_copy_all(void);
void clipboard_select_all(void);
void reset_fonts(gpointer data, guint action, GtkWidget *widget );
void ask_save_before_exit(GtkWidget *widget, gpointer data);
void preferences(gpointer data, guint action, GtkWidget *widget);
void orientation(gpointer data, guint action, GtkWidget  *widget );
void set_dasher_fontsize(gpointer data, guint action, GtkWidget  *widget );
void show_toolbar(gpointer data, guint action, GtkWidget  *widget );
void show_slider(gpointer data, guint action, GtkWidget  *widget );
void timestamp_files(gpointer data, guint action, GtkWidget *widget );
void copy_all_on_stop(gpointer data, guint action, GtkWidget *widget );
void file_encoding(gpointer data, guint action, GtkWidget *widget );
void about_dasher(gpointer data, guint action, GtkWidget *widget );
void DrawMouse(gpointer data, guint action, GtkWidget *widget );
void SetDimension(gpointer data, guint action, GtkWidget *widget );
void startonleft(gpointer data, guint action, GtkWidget *widget );
void startonspace(gpointer data, guint action, GtkWidget *widget );
void keycontrol(gpointer data, guint action, GtkWidget *widget );
void windowpause(gpointer data, guint action, GtkWidget *widget );
void controlmode(gpointer data, guint action, GtkWidget *widget );
void select_open_file(gpointer data, guint action, GtkWidget *widget);
void select_new_file(gpointer data, guint action, GtkWidget *widget);
void select_save_file_as();
void select_append_file();
void save_file();
void select_import_file();
void interface_setup();
GtkWidget* open_window();
void choose_filename();

void parameter_string_callback( string_param p, const char *value );
void parameter_double_callback( double_param p, double value );
void parameter_int_callback( int_param p, long int value );
void parameter_bool_callback( bool_param p, bool value );


void null_log_handler (const gchar *log_domain, GLogLevelFlags log_level, 
		       const gchar *message, gpointer unused_data);

extern GtkWidget *vbox, *toolbar;
extern GdkPixbuf *p;
extern GtkWidget *pw;
//Gtk2DasherEdit *dasher_text_view;
extern GtkWidget *text_view;
extern GtkWidget *speed_frame;
extern GtkObject *speed_slider;
extern GtkWidget *speed_hscale;
extern GtkWidget *text_scrolled_window;
extern GtkWidget *canvas_frame;
extern GtkWidget *ofilesel;
extern GtkWidget *ifilesel;
extern GtkWidget *afilesel;
extern GtkStyle *style;
//Gtk2DasherCanvas *dasher_canvas;
//Gtk2DasherPane *dasher_pane;
//CDasherInterface *interface;
extern GtkItemFactory *dasher_menu;
extern GtkAccelGroup *dasher_accel;
extern GtkWidget *dasher_menu_bar;

extern bool controlmodeon;

void set_dasher_font(gpointer data, guint action, GtkWidget *widget);
void get_font_from_dialog( GtkWidget *one, GtkWidget *two );

#endif




