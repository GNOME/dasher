#include "libdasher.h"
#include <gtk/gtk.h>

void initialise_edit();
void edit_output_callback(symbol Symbol);
void edit_flush_callback(symbol Symbol);
void edit_unflush_callback();
void clipboard_callback( clipboard_action act );
void select_all();
void clear_edit();

extern GtkWidget *the_text_view;  
extern GtkTextBuffer *the_text_buffer;
extern GtkClipboard *the_text_clipboard;
extern int flush_count;

void set_edit_font(gpointer data, guint action, GtkWidget *widget);
void get_edit_font_from_dialog( GtkWidget *one, GtkWidget *two );
void reset_edit_font();
