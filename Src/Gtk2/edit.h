#include "libdasher.h"
#include <gtk/gtk.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void initialise_edit();
void edit_output_callback(symbol Symbol);
void edit_outputcontrol_callback(void* pointer, int data);
void edit_delete_callback();
void clipboard_callback( clipboard_action act );
void select_all();
void clear_edit();

extern GtkWidget *the_text_view;  
extern GtkTextBuffer *the_text_buffer;
extern GtkClipboard *the_text_clipboard;

void set_editbox_font(std::string FontName);
void reset_edit_font();
void get_new_context_callback( std::string &str, int max );

void write_to_file();

#ifdef GNOME_SPEECH
void speak();
#endif
