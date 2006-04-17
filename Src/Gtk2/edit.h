//#include "libdasher.h"
#include "GtkDasherControl.h"
#include <gtk/gtk.h>
//#include "dashergtktextview.h"
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

/* #ifdef GNOME_A11Y */
/* #include <cspi/spi.h> */
/* #endif */

//#include <string>

#include <glade/glade.h>


bool edit_handle_control_event(gint iEvent);
void initialise_edit(GladeXML * pGladeXML);
void cleanup_edit();
//void create_buffer();

extern "C" void gtk2_edit_output_callback(GtkDasherControl * pDasherControl, const gchar * szText, gpointer user_data);

void gtk2_edit_outputcontrol_callback(void *pointer, int data);

extern "C" void gtk2_edit_delete_callback(GtkDasherControl * pDasherControl, const gchar * szText, gpointer user_data);

/* void edit_move_back(); */
/* void edit_move_forward(); */
/* void edit_move_start(); */
/* void edit_move_end(); */
/* void edit_delete_forward_character(); */
/* void edit_delete_forward_line(); */
/* void edit_delete_forward_word(); */
/* void edit_delete_backward_line(); */
/* void edit_delete_backward_word(); */
/* void gtk2_clipboard_callback(clipboard_action act); */
/* void select_all(); */
void clear_edit();

void set_mark();
const gchar *get_new_text();
const gchar *get_all_text();

extern GtkWidget *the_text_view;
extern GtkTextBuffer *the_text_buffer;
//extern GtkClipboard *the_text_clipboard;

void reset_edit_font();
void gtk2_get_new_context_callback(std::string & str, int max);
void handle_cursor_move(GtkTextView * textview, GtkMovementStep arg1, gint arg2, gboolean arg3, gpointer data);
void write_to_file();
//void outputpipe();

/* #ifdef GNOME_A11Y */
/* void set_textbox(Accessible * textbox); */
/* #endif */

//gboolean a11y_text_entry();
gboolean grab_focus();
