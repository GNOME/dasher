#include "libdasher.h"
#include <gtk/gtk.h>

void initialise_edit();
void edit_output_callback(symbol Symbol);
void edit_flush_callback(symbol Symbol);
void edit_unflush_callback();
void clipboard_callback( clipboard_action act );
void select_all();

extern GtkWidget *the_text_view;  
extern GtkTextBuffer *the_text_buffer;
extern GtkClipboard *the_text_clipboard;
extern int flush_count;
