#ifndef DASHER_H
#define DASHER_H

void clipboard_copy(void);
void clipboard_cut(void);
void clipboard_paste(void);
void clipboard_copy_all(void);
void clipboard_select_all(void);
static void preferences(gpointer data, guint action, GtkWidget *widget);
static void orientation(gpointer data, guint action, GtkWidget  *widget );
static void show_toolbar(gpointer data, guint action, GtkWidget  *widget );
static void show_slider(gpointer data, guint action, GtkWidget  *widget );
static void timestamp_files(gpointer data, guint action, GtkWidget *widget );
static void copy_all_on_stop(gpointer data, guint action, GtkWidget *widget );
static void file_encoding(gpointer data, guint action, GtkWidget *widget );
static void DrawMouse(gpointer data, guint action, GtkWidget *widget );
static void SetDimension(gpointer data, guint action, GtkWidget *widget );
static void select_open_file(gpointer data, guint action, GtkWidget *widget);
static void select_new_file(gpointer data, guint action, GtkWidget *widget);
static void select_save_file_as();
static void select_append_file();
static void save_file();
static void select_import_file();
void open_window();

void select_all();

void parameter_string_callback( string_param p, const char *value );
void parameter_double_callback( double_param p, double value );
void parameter_int_callback( int_param p, long int value );
void parameter_bool_callback( bool_param p, bool value );


void edit_output_callback(symbol Symbol);
void edit_flush_callback(symbol Symbol);
void edit_unflush_callback();

void clipboard_callback( clipboard_action act );

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

extern int flush_count;

#endif




