#include "libdasher.h"
#include <gtk/gtk.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#ifdef X_HAVE_UTF8_STRING
#include <X11/extensions/XTest.h>
#endif

void initialise_edit();
void cleanup_edit();
void edit_output_callback(symbol Symbol);
void edit_outputcontrol_callback(void* pointer, int data);
void edit_delete_callback();
void edit_move_back();
void edit_move_forward();
void edit_move_start();
void edit_move_end();
void edit_delete_forward_character();
void edit_delete_forward_line();
void edit_delete_forward_word();
void edit_delete_backward_line();
void edit_delete_backward_word();
void clipboard_callback( clipboard_action act );
void select_all();
void clear_edit();

extern GtkWidget *the_text_view;  
extern GtkTextBuffer *the_text_buffer;
extern GtkClipboard *the_text_clipboard;

void set_editbox_font(std::string FontName);
void reset_edit_font();
void get_new_context_callback( std::string &str, int max );
void handle_cursor_move(GtkTextView *textview, GtkMovementStep arg1, gint arg2, gboolean arg3, gpointer data);
void write_to_file();

#ifdef GNOME_SPEECH
void speak();
void speak_last();
void speak_buffer();
#endif
