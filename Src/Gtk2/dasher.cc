#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <dirent.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#ifdef GNOME_SPEECH
#include <libbonobo.h>
#include <gnome-speech/gnome-speech.h>
#endif

#ifdef GNOME_LIBS
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#endif

#include "libdasher.h"

#include "dasher.h"
#include "canvas.h"
#include "edit.h"
#include "accessibility.h"

#ifdef WITH_GPE
#include "gpesettings_store.h"
#else
#include "settings_store.h"
#endif

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

GtkWidget *vbox, *toolbar;
GtkWidget *about;
GdkPixbuf *p;
GtkWidget *pw;
GtkWidget *text_view;
GtkWidget *speed_frame;
GtkScale *speed_hscale;
GtkWidget *text_scrolled_window;
GtkStyle *style;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;
GtkWidget *vpane;
GtkFontSelectionDialog *dasher_fontselector, *edit_fontselector;
GtkTreeSelection *alphselection, *colourselection;
GtkWidget *alphabettreeview, *colourtreeview;
GtkWidget *preferences_window;
GtkListStore *alph_list_store;
GtkListStore *colour_list_store;
GladeXML *widgets;
GtkWidget *open_filesel;
GtkWidget *save_filesel;
GtkWidget *save_and_quit_filesel;
GtkWidget *import_filesel;
GtkWidget *append_filesel;
std::string alphabet;
std::string colourscheme;
char *system_data_dir;
char *user_data_dir;
GtkWidget *train_dialog;
GThread *trainthread;
GAsyncQueue* trainqueue;
ControlTree *controltree;

bool controlmodeon=false;
bool keyboardmodeon=false;
bool onedmode=false;
bool eyetrackermode=false;
bool cyclickeyboardmodeon=false;

button buttons[10];

#define _(_x) gettext(_x)

#define NO_PREV_POS -1

guint window_x = 500, window_y = 500, editheight = 50;

gboolean setup = FALSE;
gboolean paused = FALSE;
gboolean firsttime = TRUE;
gboolean indrag = FALSE;
gboolean file_modified = FALSE;
gboolean quitting = FALSE;
gboolean showtoolbar;
gboolean showslider;
gboolean timestamp;
gboolean startleft;
gboolean startspace;
gboolean keyboardcontrol;
gboolean leavewindowpause;
gboolean mouseposstart;
gboolean firstbox=FALSE;
gboolean secondbox=FALSE;
gboolean speakonstop=FALSE;
gboolean training=FALSE;
gboolean exiting=FALSE;
gboolean direction=TRUE;

gint dasherwidth, dasherheight;
long yscale, mouseposstartdist=0;
gboolean coordcalled;

gint buttonnum=0;

extern gboolean timedata;
extern gboolean drawoutline;
extern gboolean textentry;
extern gboolean stdoutpipe;

gint prev_pos_x;
gint prev_pos_y;

gint fileencoding;

gint outputcharacters;

time_t lastdirection=0;
time_t dasherstarttime;
time_t starttime=0;
time_t starttime2=0;

const gchar *filename = NULL;
GPatternSpec *alphabetglob, *colourglob;


GtkWidget *window;
GtkWidget *file_selector;

std::string dasherfont="DASHERFONT";
std::string editfont="Sans 10";

double bitrate;

void 
load_training_file (const gchar *filename)
{
  dasher_train_file( filename );
}

gpointer
change_alphabet(gpointer alph)
{
  // This is launched as a separate thread in order to let the main thread
  // carry on updating the training window
  dasher_set_parameter_string( STRING_ALPHABET, (gchar*)alph );
  //  g_free(alph);
  g_async_queue_push(trainqueue,(void *)1);
  g_thread_exit(NULL);
  return NULL;
}


extern "C" void alphabet_select(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *alph;
  GdkCursor *waitcursor, *arrowcursor;
  GtkWidget *preferences_window = GTK_WIDGET(data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get(model, &iter, 0, &alph, -1);    
    // There's no point in training if the alphabet is already selected
    if (alph!=alphabet) {
      alphabet=alph;
#ifndef WITH_GPE
      // Don't let them select another alphabet while we're training the first one
      if (training==true) {
	return;
      }

      // Note that we're training - this is needed in order to avoid
      // doing anything that would conflict with the other thread
      training=TRUE;
      trainqueue=g_async_queue_new();
      trainthread=g_thread_create(change_alphabet,alph,false,NULL);
      train_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,GTK_BUTTONS_NONE, _("Training Dasher, please wait"));
      gtk_window_set_resizable(GTK_WINDOW(train_dialog), FALSE);
      gtk_window_present(GTK_WINDOW(train_dialog));
#else
      // For GPE, we're not so fussed at the moment
      dasher_set_parameter_string( STRING_ALPHABET, (gchar*)alph );
#endif
      g_free(alph);
    } else {
      g_free(alph);
    }
  }
}

void update_colours()
{
  if (training==true) {
    // We can go back and do this after training, but doing it now would
    // break stuff
    return;
  }

  colourscheme=dasher_get_current_colours();
  const int colourlist_size=128;
  const char *colourlist[ colourlist_size ];
  int colour_count = dasher_get_colours( colourlist, colourlist_size );
  for (int i=0; i<colour_count; i++) {
    if (colourscheme==colourlist[i]) {
      // We need to build a path - GTK 2.2 lets us do this nicely, but we
      // want to support 2.0
      gchar ugly_path_hack[100];
      sprintf(ugly_path_hack,"%d",i);
      gtk_tree_selection_select_path(colourselection,gtk_tree_path_new_from_string(ugly_path_hack));
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(colourtreeview),gtk_tree_path_new_from_string(ugly_path_hack),NULL,false);
    }
  }
}

extern "C" void colour_select(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *colour;
  GdkCursor *waitcursor, *arrowcursor;
  GtkWidget *preferences_window = GTK_WIDGET(data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get(model, &iter, 0, &colour, -1);

    dasher_set_parameter_string( STRING_COLOUR, colour );    

    // Reset the colour selection as well
    colourscheme=colour;

    dasher_redraw();

    g_free(colour);
  }
}

extern "C" void 
generate_preferences(GtkWidget *widget, gpointer user_data) { 
  // We need to populate the lists of alphabets and colours

  int alphabet_count, colour_count;

  const int alphabetlist_size = 128;
  const char *alphabetlist[ alphabetlist_size ];
  const int colourlist_size=128;
  const char *colourlist[ colourlist_size ];
  GtkTreeIter alphiter, colouriter;

  // Build the alphabet tree - this is nasty
  alphabettreeview = glade_xml_get_widget(widgets,"AlphabetTree");  
  alph_list_store = gtk_list_store_new(1,G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(alphabettreeview), GTK_TREE_MODEL(alph_list_store));
  alphselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(alphabettreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(alphselection),GTK_SELECTION_BROWSE);
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("Alphabet",gtk_cell_renderer_text_new(),"text",0,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(alphabettreeview),column);

  // Clear the contents of the alphabet list
  gtk_list_store_clear( alph_list_store );

  // And repopulate it with an up to date list
  alphabet_count = dasher_get_alphabets( alphabetlist, alphabetlist_size );

  // Connect up a signal so we can select a new alphabet
  g_signal_connect_after(G_OBJECT(alphselection),"changed",GTK_SIGNAL_FUNC(alphabet_select),NULL);

  // Do the actual list population
  for (int i=0; i<alphabet_count; ++i) {
    gtk_list_store_append (alph_list_store, &alphiter);
    gtk_list_store_set (alph_list_store, &alphiter, 0, alphabetlist[i],-1);
    if (alphabetlist[i]==alphabet) {
      gchar ugly_path_hack[100];
      sprintf(ugly_path_hack,"%d",i);
      gtk_tree_selection_select_iter(alphselection, &alphiter);
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(alphabettreeview),gtk_tree_path_new_from_string(ugly_path_hack),NULL,false);
    }
  }
  
  // Do the same for colours
  colourtreeview = glade_xml_get_widget(widgets,"ColorTree");  

  // Make sure that the colour tree is realized now as we'll need to do
  // stuff with it before it's actually displayed
  gtk_widget_realize(colourtreeview);

  colour_list_store = gtk_list_store_new(1,G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(colourtreeview), GTK_TREE_MODEL(colour_list_store));
  colourselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(colourtreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(colourselection),GTK_SELECTION_BROWSE);
  column = gtk_tree_view_column_new_with_attributes ("Colour",gtk_cell_renderer_text_new(),"text",0,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(colourtreeview),column);

  // Clear the contents of the colour list
  gtk_list_store_clear( colour_list_store );

  // And repopulate it with an up to date list
  colour_count = dasher_get_colours( colourlist, colourlist_size );

  // Connect up a signal so we can select a new colour scheme
  g_signal_connect_after(G_OBJECT(colourselection),"changed",GTK_SIGNAL_FUNC(colour_select),NULL);

  for (int i=0; i<colour_count; ++i) {
    gtk_list_store_append (colour_list_store, &colouriter);
    gtk_list_store_set (colour_list_store, &colouriter, 0, colourlist[i],-1);
    if (colourlist[i]==colourscheme) {
      gchar ugly_path_hack[100];
      sprintf(ugly_path_hack,"%d",i);
      gtk_tree_selection_select_iter(colourselection, &colouriter);
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(colourtreeview),gtk_tree_path_new_from_string(ugly_path_hack),NULL,false);
    }

  }

}

extern "C" void
preferences_display(GtkWidget *widget, gpointer user_data)
{
  if (preferences_window==NULL) {
    preferences_window=glade_xml_get_widget(widgets, "preferences");
  }

  // Keep the preferences window in the correct position relative to the
  // main Dasher window
  gtk_window_set_transient_for(GTK_WINDOW(preferences_window),GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(preferences_window));
}
  
extern "C" gboolean
preferences_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide(preferences_window);
  return TRUE;
}

extern "C" gboolean
button_preferences_show(GtkWidget *widget, gpointer user_data)
{
  // FIXME
  // Ugly, ugly, ugly, ugly. Hmm, could this be done with an enum instead?
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton1")),buttons[1].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton2")),buttons[2].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton3")),buttons[3].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton4")),buttons[4].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton5")),buttons[5].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton6")),buttons[6].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton7")),buttons[7].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton8")),buttons[8].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton9")),buttons[9].x);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton10")),buttons[1].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton11")),buttons[2].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton12")),buttons[3].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton13")),buttons[4].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton14")),buttons[5].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton15")),buttons[6].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton16")),buttons[7].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton17")),buttons[8].y);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(widgets,"spinbutton18")),buttons[9].y);
  gtk_window_set_transient_for(GTK_WINDOW(glade_xml_get_widget(widgets,"buttonprefs")),GTK_WINDOW(preferences_window));
  gtk_window_present(GTK_WINDOW(glade_xml_get_widget(widgets,"buttonprefs")));
  return FALSE;
}

extern "C" gboolean
button_preferences_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide(glade_xml_get_widget(widgets,"buttonprefs"));
  return FALSE;
}

extern "C" gboolean
fontsel_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide(GTK_WIDGET(dasher_fontselector));
  return FALSE;
}

extern "C" gboolean
edit_fontsel_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide(GTK_WIDGET(edit_fontselector));
  return FALSE;
}

extern "C" gboolean
button_coordinates_changed(GtkWidget *widget, gpointer user_data)
{
  GtkSpinButton *spinbutton=GTK_SPIN_BUTTON(widget);
  int value=int(gtk_spin_button_get_value(spinbutton));

  // Really dreadfully hacky stuff to avoid recursion
  //
  // The recursion only seems to happen if the value ends up as 0, so
  // if we read a 0 twice in a row from the same widget then just break
  // out and assume that it really is a 0
  if (coordcalled==true && value==0) {
    return true;
  } else if (value==0) {
    coordcalled=true;
  }
  gtk_spin_button_update(spinbutton);
  coordcalled=false;
  value=int(gtk_spin_button_get_value(spinbutton));
  
  // FIXME
  // See previous comment about enums
  // (Mind you, the whole of this is a mess anyway...)
  if (widget==glade_xml_get_widget(widgets,"spinbutton1")) {
    buttons[1].x=value;
    set_long_option_callback("Button1X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton2")) {
    buttons[2].x=value;
    set_long_option_callback("Button2X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton3")) {
    buttons[3].x=value;
    set_long_option_callback("Button3X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton4")) {
    buttons[4].x=value;
    set_long_option_callback("Button4X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton5")) {
    buttons[5].x=value;
    set_long_option_callback("Button5X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton6")) {
    buttons[6].x=value;
    set_long_option_callback("Button6X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton7")) {
    buttons[7].x=value;
    set_long_option_callback("Button7X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton8")) {
    buttons[8].x=value;
    set_long_option_callback("Button8X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton9")) {
    buttons[9].x=value;
    set_long_option_callback("Button9X",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton10")) {
    buttons[1].y=value;
    set_long_option_callback("Button1Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton11")) {
    buttons[2].y=value;
    set_long_option_callback("Button2Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton12")) {
    buttons[3].y=value;
    set_long_option_callback("Button3Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton13")) {
    buttons[4].y=value;
    set_long_option_callback("Button4Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton14")) {
    buttons[5].y=value;
    set_long_option_callback("Button5Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton15")) {
    buttons[6].y=value;
    set_long_option_callback("Button6Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton16")) {
    buttons[7].y=value;
    set_long_option_callback("Button7Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton17")) {
    buttons[8].y=value;
    set_long_option_callback("Button8Y",value);
  } else if (widget==glade_xml_get_widget(widgets,"spinbutton18")) {
    buttons[9].y=value;
    set_long_option_callback("Button9Y",value);
  }
}

#ifdef GNOME_LIBS
void
vfs_print_error(GnomeVFSResult *result)
{
  // Turns a Gnome VFS error into English
  GtkWidget *error_dialog;
  error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, "Could not open the file \"%s\"\n%s\n", filename,gnome_vfs_result_to_string (*result));
  gtk_dialog_set_default_response(GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
  gtk_dialog_run(GTK_DIALOG(error_dialog));
  gtk_widget_destroy(error_dialog);
  return;
}
#endif

extern "C" void 
open_file (const char *myfilename)
{
  int size;
  gchar *buffer;
  GtkWidget *error_dialog;

  struct stat file_stat;
  FILE *fp;
  int pos = 0;

  stat (myfilename, &file_stat);
  fp = fopen (myfilename, "r");

  if (fp==NULL || S_ISDIR(file_stat.st_mode)) {
    error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, "Could not open the file \"%s\".\n", myfilename);
    gtk_dialog_set_default_response(GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return;
  }

  size=file_stat.st_size;
  buffer = (gchar *) g_malloc (size);
  fread (buffer, size, 1, fp);
  fclose (fp);

  dasher_clear();
  
  file_modified = TRUE;

  if (size!=0) {
    // Don't attempt to insert new text if the file is empty as it makes
    // GTK cry
    if (g_utf8_validate(buffer,size,NULL)==FALSE) {
      // It's not UTF8, so we do the best we can...
      gchar* buffer2=g_locale_to_utf8(buffer,size,NULL,NULL,NULL);
      g_free(buffer);
      buffer=buffer2;
    }
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER (the_text_buffer), buffer, size);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW (the_text_view),gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(the_text_buffer)));
  }

  gtk_window_set_title(GTK_WINDOW(window), myfilename);
  filename=myfilename;

  dasher_start();
  dasher_redraw();
}

extern "C" void
select_new_file(GtkWidget *widget, gpointer user_data)
{
  //FIXME - confirm this. We should check whether the user wants to lose their work.

  choose_filename();

  clear_edit();
  dasher_start();
  dasher_redraw();
  dasher_pause(0,0);
}

extern "C" bool
save_file_as (const char *filename, bool append)
{
  int opened=1;
  gint length;
  gchar *inbuffer,*outbuffer = NULL;
  gsize bytes_read, bytes_written;
  GError *error = NULL;
  GIConv cd;
  GtkWidget *error_dialog;
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  FILE *fp;

  if (append == true) {
    fp = fopen (filename, "a");

    if (fp == NULL) {
      opened = 0;
    }
  } else {
    fp = fopen (filename, "w");
    if (fp == NULL) {
      opened = 0;
    }
  }

  if (!opened) {
    error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, "Could not save the file \"%s\".\n", filename);
    gtk_dialog_set_default_response(GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return false;
  }

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1);

  inbuffer = gtk_text_iter_get_slice (start,end);

  length = gtk_text_iter_get_offset(end)-gtk_text_iter_get_offset(start);
  outbuffer = (char *)malloc((length+1)*sizeof(gchar));
  memcpy((void*)outbuffer,(void*)inbuffer,length*sizeof(gchar));
  outbuffer[length]=0;
  g_free(inbuffer);
  inbuffer=outbuffer;
  outbuffer=NULL;

  switch (fileencoding) {
  case Opts::UserDefault:
  case Opts::AlphabetDefault:
    //FIXME - need to call GetAlphabetType and do appropriate stuff regarding the
    // character set. Arguably we should always be saving in either UTF8 or the user's
    // locale (which may, of course, be UTF8) because otherwise we're going to read
    // in rubbish, and we shouldn't be encouraging weird codepage madness any further
    //FIXME - error handling
    outbuffer=g_locale_from_utf8(inbuffer,-1,&bytes_read,&bytes_written,&error);
    if (outbuffer==NULL) {
      // We can't represent the text in the current locale, so fall back to
      // UTF-8
      outbuffer=inbuffer;
      bytes_written=length;
    }
  case Opts::UTF8:
    outbuffer=inbuffer;
    bytes_written=length;
    break;
  // Does /anyone/ want to save text files in UTF16?
  // (in any case, my opinions regarding encouragement of data formats with
  // endianness damage are almost certainly unprintable)

  case Opts::UTF16LE:
    cd=g_iconv_open("UTF16LE","UTF8");
    outbuffer=g_convert_with_iconv(inbuffer,-1,cd,&bytes_read,&bytes_written,&error);
    break;
  case Opts::UTF16BE:
    cd=g_iconv_open("UTF16BE","UTF8");
    outbuffer=g_convert_with_iconv(inbuffer,-1,cd,&bytes_read,&bytes_written,&error);
    break;
  default:
    outbuffer=inbuffer;
    bytes_written=length;
  }

  fwrite(outbuffer,1,bytes_written,fp);
  fclose (fp);
  
  file_modified = FALSE;
  gtk_window_set_title(GTK_WINDOW(window), filename);
}

#if GTK_CHECK_VERSION(2,3,0)

/* Fudge to avoid Glade complaining about being unable to find this signal 
   handler */
extern "C" void
filesel_hide(GtkWidget *widget, gpointer user_data)
{
  return;
}

extern "C" void
select_open_file(GtkWidget *widget, gpointer user_data)
{
  GtkWidget* filesel = gtk_file_chooser_dialog_new (_("Select File"),GTK_WINDOW(window),GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

  if (gtk_dialog_run (GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
    open_file(filename);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

extern "C" void
select_save_file_as(GtkWidget *widget, gpointer user_data)
{
  GtkWidget* filesel = gtk_file_chooser_dialog_new (_("Select File"),GTK_WINDOW(window),GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
  
  if (gtk_dialog_run (GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
    save_file_as(filename,FALSE);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

extern "C" void
select_append_file(GtkWidget *widget, gpointer user_data)
{
  GtkWidget* filesel = gtk_file_chooser_dialog_new (_("Select File"),GTK_WINDOW(window),GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
  
  if (gtk_dialog_run (GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
    save_file_as(filename,TRUE);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

extern "C" void
select_import_file(GtkWidget *widget, gpointer user_data)
{
  GtkWidget* filesel = gtk_file_chooser_dialog_new (_("Select File"),GTK_WINDOW(window),GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
  
  if (gtk_dialog_run (GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
    load_training_file(filename);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}
#else

extern "C" void
import_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));
  load_training_file(filename);  
  filesel_hide(GTK_WIDGET(selector->ok_button),NULL);
}

extern "C" void               
select_import_file(GtkWidget *widget, gpointer user_data)
{
  if (import_filesel==NULL) {
    import_filesel = glade_xml_get_widget(widgets, "import_fileselector");
    
    g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (import_filesel)->ok_button),
                      "clicked", G_CALLBACK (import_file_from_filesel), (gpointer) import_filesel);
  }
  
  gtk_window_set_transient_for(GTK_WINDOW(import_filesel),GTK_WINDOW(window));
  gtk_window_present (GTK_WINDOW(import_filesel));
}

extern "C" void
open_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button),NULL);
  open_file (filename);
}

extern "C" void
select_open_file(GtkWidget *widget, gpointer user_data)
{
  if (open_filesel==NULL) {
    open_filesel = glade_xml_get_widget(widgets, "open_fileselector");
    g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (open_filesel)->ok_button),
                      "clicked", G_CALLBACK (open_file_from_filesel), (gpointer) open_filesel);
  }
  gtk_window_set_transient_for(GTK_WINDOW(open_filesel),GTK_WINDOW(window));
  gtk_window_present (GTK_WINDOW(open_filesel));
}

extern "C" void
save_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button),NULL);
  save_file_as(filename,FALSE);
}

extern "C" void
save_file_from_filesel_and_quit ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));
  if (save_file_as(filename,FALSE)==false) {
    return;
  } else {
    exiting=TRUE;
    gtk_main_quit();
  }
}

extern "C" void
append_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));
  
  save_file_as(filename,TRUE);
  
  filesel_hide(GTK_WIDGET(selector->ok_button),NULL);
}

extern "C" void
select_save_file_as(GtkWidget *widget, gpointer user_data)
{
  if (save_filesel==NULL) {
    save_filesel = glade_xml_get_widget(widgets, "save_fileselector");
    g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (save_filesel)->ok_button),
                      "clicked", G_CALLBACK (save_file_from_filesel), (gpointer) save_filesel);
  }
  
  if (filename!=NULL)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(save_filesel), filename);
  gtk_window_set_transient_for(GTK_WINDOW(save_filesel),GTK_WINDOW(window));
  gtk_window_present (GTK_WINDOW(save_filesel));
}

extern "C" void
select_save_file_as_and_quit(GtkWidget *widget, gpointer user_data)
{
  if (save_and_quit_filesel==NULL) {
    save_and_quit_filesel = glade_xml_get_widget(widgets, "save_and_quit_fileselector");
    g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (save_and_quit_filesel)->ok_button),
                      "clicked", G_CALLBACK (save_file_from_filesel_and_quit), (gpointer) save_and_quit_filesel);
}
  
  if (filename!=NULL)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(save_and_quit_filesel), filename);
  gtk_window_set_transient_for(GTK_WINDOW(save_and_quit_filesel),GTK_WINDOW(window));
  gtk_window_present (GTK_WINDOW(save_and_quit_filesel));
}

extern "C" void
select_append_file(GtkWidget *widget, gpointer user_data)
{
  if (append_filesel==NULL) {
    append_filesel = glade_xml_get_widget(widgets, "append_fileselector");
    g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (append_filesel)->ok_button),
                      "clicked", G_CALLBACK (append_file_from_filesel), (gpointer) append_filesel);
  }
  
  gtk_window_set_transient_for(GTK_WINDOW(append_filesel),GTK_WINDOW(window));
  gtk_window_present (GTK_WINDOW(append_filesel));
}

extern "C" void
filesel_hide(GtkWidget *widget, gpointer user_data)
{
  // FIXME - uh. Yes. This works, but is it in any way guaranteed to?
  // Of course, if glade let us set user_data stuff properly, this would
  // be a lot easier
  gtk_widget_hide(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget))));
}
#endif

extern "C" void
save_file (GtkWidget *widget, gpointer user_data)
{
  if (filename != NULL) {
    save_file_as(filename,FALSE);
  }
  else {
    select_save_file_as(NULL,NULL);
  }
}

extern "C" void
save_file_and_quit (GtkWidget *widget, gpointer user_data)
{
  if (filename != NULL) {
    if (save_file_as(filename,FALSE)==true) {
      exiting=TRUE;
      gtk_main_quit();
    } else {
      return;
    }
  }
  else {
    select_save_file_as(NULL,NULL);
    gtk_main_quit();
  }
}

extern "C" bool
ask_save_before_exit(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog = NULL;
  quitting==TRUE;

  if (file_modified != FALSE) {
    // Ask whether to save the modified file, insert filename if it exists.
    if (filename != NULL) {
      dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_QUESTION,GTK_BUTTONS_NONE,"Do you want to save your changes to %s?\n\nYour changes will be lost if you don't save them.", filename);
    }
    else if (filename == NULL) {
      dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_QUESTION,GTK_BUTTONS_NONE,"Do you want to save your changes?\n\nYour changes will be lost if you don't save them.");
    }

    gtk_dialog_add_buttons(GTK_DIALOG(dialog),"Don't save",GTK_RESPONSE_REJECT,"Don't quit",GTK_RESPONSE_CANCEL,"Save and quit",GTK_RESPONSE_ACCEPT,NULL);
    switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_REJECT:
      write_to_file();
      exiting=TRUE;
      gtk_main_quit();
      break;
    case GTK_RESPONSE_CANCEL:
      quitting==FALSE;
      gtk_widget_destroy (GTK_WIDGET(dialog));
      return true;
      break;
    case GTK_RESPONSE_ACCEPT:
      gtk_widget_destroy (GTK_WIDGET(dialog));
      write_to_file();
      save_file_and_quit(NULL,NULL);
    }
  }
  else {
    // It should be noted that write_to_file merely saves the new text to the training
    // file rather than saving it to a file of the user's choice
    write_to_file();
    exiting=TRUE;
    gtk_main_quit();
  }
}

long get_time() {
  // We need to provide a monotonic time source that ticks every millisecond
  long s_now;
  long ms_now;
  
  struct timeval tv;
  struct timezone tz;
  
  gettimeofday (&tv, &tz);
  
  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;
  
  return (s_now * 1000 + ms_now);
}

gint
timer_callback(gpointer data)
{
  if (exiting==TRUE)
    {
      // Exit if we're called when Dasher is exiting
      return 0;
    }
  if (training==TRUE)
    {
      // Check if we're still training, and if so just return non-0 in order to get
      // called again
      if (g_async_queue_try_pop(trainqueue)==NULL) {
	return 1;
      } else {
	// Otherwise, we've just finished training - make everything work again
	training=FALSE;
	// Get rid of the training dialog and thread
	g_async_queue_unref(trainqueue);
	gtk_widget_hide(train_dialog);
	// We need to do this again to get the configuration saved, as we
	// can't do gconf stuff from the other thread
	dasher_set_parameter_string( STRING_ALPHABET, alphabet.c_str() );
	
	// And call update_colours again now that we can do something useful
	update_colours();
	//	deletemenutree();
	// And making bonobo calls from another thread is likely to lead to
	// pain as well. It'd be nice to do this while training, but.
	add_control_tree(controltree);

	dasher_redraw();
      }
    }
  
    if (!paused) {
    
     if (eyetrackermode) {
        if (direction==TRUE) {
            dasher_set_parameter_int(INT_ONEBUTTON, 150);
        }
        if (direction==FALSE) {
            dasher_set_parameter_int(INT_ONEBUTTON, -150); 
        }
        cout << dasher_get_onebutton() << endl;
    }
    int x;
    int y;
    
    if (leavewindowpause==true) {
      gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);
      
      gdk_window_get_pointer(GTK_WIDGET(window)->window, &x, &y, NULL);
      
      if (x>dasherwidth || x<0 || y>dasherheight || y<0) {
	// Don't do anything with the mouse position if we're outside the window. There's a
	// minor issue with this - if the user moves the cursor back in, Dasher will think
	// that lots of time has passed and jerk forwards until it recalculates the framerate
	return 1;
      }
    }
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    if (onedmode==true) {
      // In one dimensional mode, we want to scale the vertical component so that it's possible
      // for the amount of input to cover the entire canvas
      float scalefactor;
      float newy=y;
      gdk_drawable_get_size(the_canvas->window, &dasherwidth, &dasherheight);
      if (yscale==0) {
	// For the magic value 0, we want the canvas size to reflect a full Y deflection
	// otherwise the user can't access the entire range. 2 is actually a slight
	// overestimate, but doing it properly would require thought and the benefit
	// is probably insufficient.
	scalefactor=2;
      } else {
	scalefactor=float(dasherheight)/float(yscale);
      }
      // Transform the real Y coordinate into a fudged Y coordinate
      newy-=dasherheight/2;
      newy=newy*scalefactor;
      newy+=dasherheight/2;
      y=int(newy);
    } 
    // And then provide the mouse position to the core. Of course, the core may then
    // do its own fudging.
    dasher_tap_on( x, y, get_time() );
  }
  
  else {
    // If we're paused, then we still need to work out where the mouse is for two
    // reasons - start on mouse position, and to update the on-screen representation
    int x,y;
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    
    if (onedmode==true) {
      float scalefactor;
      float newy=y;
      gdk_drawable_get_size(the_canvas->window, &dasherwidth, &dasherheight);
      if (yscale==0) {
	scalefactor=2;
      } else {
	scalefactor=float(dasherheight)/float(yscale);
      }
      newy-=dasherheight/2;
      newy=newy*scalefactor;
      newy+=dasherheight/2;
      y=int(newy);
    } 
    
    dasher_draw_mouse_position(x,y);
    
    if (mouseposstart==true) {
      // The user must hold the mouse pointer inside the red box, then the yellow box
      // If the user fails to move to the yellow box, display the red box again and
      // start over
      dasherheight=the_canvas->allocation.height;
      gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
      
      if (firsttime==firstbox==secondbox==false) { // special case for Dasher 
	firstbox=true;                             // startup
	dasher_redraw();
      }
      
      if (y>(dasherheight/2-mouseposstartdist-100) && y<(dasherheight/2-mouseposstartdist) && firstbox==true) {
	// The pointer is inside the red box
	if (starttime==0) {
	  // for the first time
	  starttime=time(NULL);
	} else {
	  // for some period of time
	  if ((time(NULL)-starttime)>2) {
	    // for long enough to trigger the yellow box
	    starttime=time(NULL);
	    secondbox=true;
	    firstbox=false;
	    dasher_redraw();
	  }
	}
      } else if (y<(dasherheight/2+mouseposstartdist+100) && y>(dasherheight/2+mouseposstartdist) && secondbox==true) {      
	// inside the yellow box, and the yellow box has been displayed
	if (starttime2==0) {
	  // for the first time
	  starttime2=time(NULL);
	  starttime=0;
	} else {
	  // for some period of time
	  if ((time(NULL)-starttime2)>2) {
	    // for long enough to trigger starting Dasher
	    secondbox=false;
	    stop(); // Yes, confusingly named
	  }
	}
      } else {
	if (secondbox==true && (starttime2>0 || (time(NULL)-starttime)>3)) {
	  // remove the yellow box if the user moves the pointer outside it
	  // or fails to select it sufficiently quickly
	  secondbox=false;
	  firstbox=true;
	  starttime2=0;
	  starttime=0;
	  dasher_redraw();
	} else if (firstbox==true) {
	  // Start counting again if the mouse is outside the red box and the yellow
	  // box isn't being displayed
	  starttime=0;
	}
      }
    }
  }
  // need non-zero return value so timer repeats
  return 1;
}

extern "C" gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  gdk_draw_drawable(the_canvas->window,
		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		  onscreen_buffer,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  if (firsttime==TRUE) {
    // canvas_expose_event() is the easiest function to catch
    // if we want to know when everything is set up and displayed
    paused=true;
    firsttime=false;
  }

  return TRUE;
}

extern "C" gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  // If the canvas is resized, we need to regenerate all of the buffers
  rebuild_buffer();

  dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );

  dasher_redraw();

  if (setup==TRUE) {
    // If we're set up and resized, then save those settings
    dasher_set_parameter_int(INT_EDITHEIGHT,gtk_paned_get_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1"))));
    gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);
    dasher_set_parameter_int(INT_SCREENHEIGHT, dasherheight);
    dasher_set_parameter_int(INT_SCREENWIDTH, dasherwidth);
  }

  return FALSE;
}

extern "C" gboolean
edit_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if (paused==true) {
    // Dasher needs to update based on the context
    dasher_start();
    dasher_redraw();
    return FALSE;
  }
}

extern "C" gint
key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  int i,width,height;
  if (event->type != GDK_KEY_PRESS)
    return FALSE;

  if (keyboardcontrol == false) {
    // CJB,  2003-08.  If we have a selection, replace it with the new input.
    // This code is duplicated in button_press_event. 
    if (gtk_text_buffer_get_selection_bounds (the_text_buffer, NULL, NULL))
      gtk_text_buffer_cut_clipboard(the_text_buffer,the_text_clipboard,TRUE);
  }

  // Eww. This stuff all needs to be rewritten at some point, anyway
  switch (event->keyval) {    
  case GDK_Up:
    if (keyboardcontrol == true) {
      if (cyclickeyboardmodeon==true) {
	int cycles=0;
	buttonnum++;
	buttonnum=buttonnum%9;
	while(buttons[buttonnum+1].x==0 && buttons[buttonnum+1].y==0 && cycles<10) {
	  buttonnum++;
	  buttonnum=buttonnum%9;
	  cycles++;
	}
	paused=false;
	dasher_draw_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
	paused=true;
	return TRUE;
      }
      if (buttons[1].x==0 && buttons[1].y==0) {
	width = the_canvas->allocation.width;
	height = the_canvas->allocation.height;
	paused=false;
	dasher_go_to((int)(0.70*width), (int)(0.20*height));
	dasher_draw_go_to((int)(0.70*width), (int)(0.20*height));
	paused=true;
	return TRUE;
      } else {
	paused=false;
	dasher_go_to(int(buttons[1].x),int(buttons[1].y));
	dasher_draw_go_to(int(buttons[1].x),int(buttons[1].y));
	paused=true;
	return TRUE;
      }
    }
    break;
  case GDK_Down:
    if (keyboardcontrol == true) {
      if (cyclickeyboardmodeon==true) {
	int cycles=0;
	buttonnum--;
	if (buttonnum<0) {
	  buttonnum=8;
	}
	buttonnum=buttonnum%9;
	while(buttons[buttonnum+1].x==0 && buttons[buttonnum+1].y==0 && cycles<10) {
	  buttonnum--;
	  if (buttonnum<0) {
	    buttonnum=8;
	  }
	  cycles++;
	}

	paused=false;
	dasher_draw_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
	paused=true;
	return TRUE;
      }
      if (buttons[3].x==0 && buttons[3].y==0) {
	width = the_canvas->allocation.width;
	height = the_canvas->allocation.height;
	paused=false;
	dasher_go_to((int)(0.70*width), (int)(0.80*height));
	dasher_draw_go_to((int)(0.70*width), (int)(0.80*height));
	paused=true;
	return TRUE;
      } else {
	paused=false;
	dasher_go_to(int(buttons[3].x),int(buttons[3].y));
	dasher_draw_go_to(int(buttons[3].x),int(buttons[3].y));
	paused=true;
	return TRUE;
      }
    }
    break;
  case GDK_Left:
    if (keyboardcontrol == true) {
      if (cyclickeyboardmodeon==true) {
	return TRUE;
      }
      if (buttons[2].x==0 && buttons[2].y==0) {
	width = the_canvas->allocation.width;
	height = the_canvas->allocation.height;
	paused=false;
	dasher_go_to((int)(0.25*width), (int)(0.50*height));
	dasher_draw_go_to((int)(0.25*width), (int)(0.50*height));
	paused=true;
	return TRUE;
      } else {
	paused=false;
	dasher_go_to(int(buttons[2].x),int(buttons[2].y));
	dasher_draw_go_to(int(buttons[2].x),int(buttons[2].y));
	paused=true;
	return TRUE;
      }
    }
    break;
  case GDK_Right:
    if (keyboardcontrol==true) {
      if (cyclickeyboardmodeon==true) {
	paused=false;
      	dasher_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
	dasher_draw_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
	paused=true;
	return TRUE;
      }
      if (buttons[4].x==0 && buttons[4].y==0) {
	return TRUE;
      } else {
	paused=false;
	dasher_go_to(int(buttons[4].x),int(buttons[4].y));
	dasher_draw_go_to(int(buttons[4].x),int(buttons[4].y));
	paused=true;
	return TRUE;
      }
    }
    break;
  case GDK_space:
    if (startspace == TRUE) {
      stop();      
    }
    return TRUE;
    break;
  case GDK_F12:
    // If the user presses F12, recentre the cursor. Useful for one-dimensional use - 
    // probably should be documented somewhere, really.
    int x, y;
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    XWarpPointer(gdk_x11_get_default_xdisplay(), 0, GDK_WINDOW_XID(the_canvas->window), 0, 0, 0, 0, the_canvas->allocation.width/2, the_canvas->allocation.height/2);
    return TRUE;
    break;
  default:
    return FALSE;
  }  
  return FALSE;
}

extern "C" gint
slider_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  if (event->type != GDK_KEY_PRESS)
    return FALSE;

  switch (event->keyval) {
  case GDK_space:
    if (startspace == TRUE) {
      stop();
    }
    return TRUE;
    break;
  case GDK_F12:
    int x, y;
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    XWarpPointer(gdk_x11_get_default_xdisplay(), 0, GDK_WINDOW_XID(the_canvas->window), 0, 0, 0, 0, the_canvas->allocation.width/2, the_canvas->allocation.height/2);
    return TRUE;
    break;
  default:
    return FALSE;
  }  
  return FALSE;
}

extern "C" gboolean
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
#ifdef WITH_GPE
  // GPE version requires the button to be held down rather than clicked
  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_BUTTON_RELEASE))
    return FALSE;
#else
  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_2BUTTON_PRESS))
    return FALSE;
#endif



  // CJB,  2003-08.  If we have a selection, replace it with the new input.
  // This code is duplicated in key_press_event.
  if (gtk_text_buffer_get_selection_bounds (the_text_buffer, NULL, NULL))
    gtk_text_buffer_cut_clipboard(the_text_buffer, the_text_clipboard, TRUE);

  // CJB.  2004-07.
  // One-button mode; change direction on mouse click.
  direction=!direction;

  if (startleft == TRUE) {
    stop();
  }
  return FALSE;
}


extern "C" gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  dasher_pause( (gint) event->x,(gint) event->y );
  paused = TRUE;

  return FALSE;
}

extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data) {
  dasher_set_parameter_double( DOUBLE_MAXBITRATE, GTK_RANGE(hscale)->adjustment->value );
}

extern "C" void uniform_changed(GtkHScale *hscale) {
  dasher_set_parameter_int( INT_UNIFORM, int(GTK_RANGE(hscale)->adjustment->value*10));
}


void interface_setup(GladeXML *xml) {
  dasher_accel = gtk_accel_group_new();
  
  widgets=xml;

  // What's this doing here? I'm sure we ought to just be using whatever
  // the core provides us with
  float initial_bitrate = 3.0;

  the_canvas=glade_xml_get_widget(xml, "the_canvas");

  // Realize the canvas now so that it can set up the buffers correctly
  gtk_widget_realize(the_canvas);

  text_scrolled_window=glade_xml_get_widget(xml, "text_scrolled_window");
  the_text_view=glade_xml_get_widget(xml, "the_text_view");
  toolbar=glade_xml_get_widget(xml, "toolbar");

  if (textentry==TRUE) {
    gtk_paned_set_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1")),0);
    gtk_widget_hide(text_scrolled_window);
    keyboardmodeon=true;
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"keyboardmode")), true);
    gtk_widget_set_sensitive(glade_xml_get_widget(widgets,"keyboardmode"),false);
    gtk_widget_hide(toolbar);
  }

#ifndef GNOME_SPEECH
  // This ought to be greyed out if not built with speech support
  gtk_widget_set_sensitive(glade_xml_get_widget(widgets,"speakbutton"),false);
#endif

  // Needed so we can make it visible or not as we wish
  speed_frame=glade_xml_get_widget(xml, "speed_frame");
  speed_hscale=GTK_SCALE(glade_xml_get_widget(xml, "speed_hscale"));

  initialise_canvas(360,360);
  initialise_edit();

  // interface specific preferences
  if(get_long_option_callback("Mouseposstartdistance",&mouseposstartdist)!=false) {
    gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets,"mouseposstartscale")),mouseposstartdist);
  }
  
  if(get_long_option_callback("YScale",&yscale)!=false) {
    gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets,"yaxisscale")),yscale);
  }

  if(get_bool_option_callback("Cyclicalbuttons",&cyclickeyboardmodeon)!=false) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"cyclicalbuttons")),cyclickeyboardmodeon);
  }

  // Configure the buttons. FIXME - more enums?
  if (get_long_option_callback("Button1X",&(buttons[1].x))==false) {
    buttons[1].x=0;
  }
  if (get_long_option_callback("Button2X",&(buttons[2].x))==false) {
    buttons[2].x=0;
  }
  if (get_long_option_callback("Button3X",&(buttons[3].x))==false) {
    buttons[3].x=0;
  }
  if (get_long_option_callback("Button4X",&(buttons[4].x))==false) {
    buttons[4].x=0;
  }
  if (get_long_option_callback("Button5X",&(buttons[5].x))==false) {
    buttons[5].x=0;
  }
  if (get_long_option_callback("Button6X",&(buttons[6].x))==false) {
    buttons[6].x=0;
  }
  if (get_long_option_callback("Button7X",&(buttons[7].x))==false) {
    buttons[7].x=0;
  }
  if (get_long_option_callback("Button8X",&(buttons[8].x))==false) {
    buttons[8].x=0;
  }
  if (get_long_option_callback("Button9X",&(buttons[9].x))==false) {
    buttons[9].x=0;
  }
  if (get_long_option_callback("Button1Y",&(buttons[1].y))==false) {
    buttons[1].y=0;
  }
  if (get_long_option_callback("Button2Y",&(buttons[2].y))==false) {
    buttons[2].y=0;
  }
  if (get_long_option_callback("Button3Y",&(buttons[3].y))==false) {
    buttons[3].y=0;
  }
  if (get_long_option_callback("Button4Y",&(buttons[4].y))==false) {
    buttons[4].y=0;
  }
  if (get_long_option_callback("Button5Y",&(buttons[5].y))==false) {
    buttons[5].y=0;
  }
  if (get_long_option_callback("Button6Y",&(buttons[6].y))==false) {
    buttons[6].y=0;
  }
  if (get_long_option_callback("Button7Y",&(buttons[7].y))==false) {
    buttons[7].y=0;
  }
  if (get_long_option_callback("Button8Y",&(buttons[8].y))==false) {
    buttons[8].y=0;
  }
  if (get_long_option_callback("Button9Y",&(buttons[9].y))==false) {
    buttons[9].y=0;
  }
}

void
interface_late_setup() {
  // Stuff that needs to be done after the core has
  // set itself up
  dasher_set_parameter_int(INT_ONEBUTTON, 0);
  alphabet=dasher_get_current_alphabet();
  colourscheme=dasher_get_current_colours();
  generate_preferences(NULL,NULL);
#ifdef WITH_GPE
  // We always want this on in the GPE version, otherwise it's entirely useless
  // Well, I suppose you could give it to kids, or impress primitive tribes,
  // or convince members of the appropriate sex that you're somehow deeply cool,
  // but they're not really our design goals.
  dasher_set_parameter_bool( BOOL_KEYBOARDMODE, true );
#endif
}

void
open_window(GladeXML *xml) {
  char *home_dir;

  home_dir = getenv( "HOME" );
  user_data_dir = new char[ strlen( home_dir ) + 10 ];
  sprintf( user_data_dir, "%s/.dasher/", home_dir );

  // Ooh, I love Unix
  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  // PROGDATA is provided by the makefile
  system_data_dir = PROGDATA"/";
  
  dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
  dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

  // Add all available alphabets and colour schemes to the core
  scan_alphabet_files();
  scan_colour_files();

  window=glade_xml_get_widget(xml, "window");
  vbox=glade_xml_get_widget(xml, "vbox1");
  vpane=glade_xml_get_widget(xml, "vpaned1");
  dasher_menu_bar=glade_xml_get_widget(xml, "dasher_menu_bar");
  dasher_fontselector=GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(xml, "dasher_fontselector"));
  edit_fontselector=GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(xml, "edit_fontselector"));

  dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
  dasher_set_parameter_int( INT_VIEW, 0 );
  
  dasher_start();
  dasher_redraw();

  // Aim for 20 frames per second
  g_timeout_add(50, timer_callback, NULL );  

  // I have no idea why we need to do this when Glade has theoretically done
  // so already, but...
  gtk_widget_add_events (the_canvas, GDK_BUTTON_PRESS_MASK);
  gtk_widget_add_events (the_canvas, GDK_BUTTON_RELEASE_MASK);

  // We need to monitor the text buffer for mark_set in order to get
  // signals when the cursor is moved
  g_signal_connect(G_OBJECT(the_text_buffer), "mark_set", G_CALLBACK(edit_button_release_event), NULL);
}

extern "C" void choose_filename() {
  if (timestamp==TRUE) {
    // Build a filename based on the current time and date
    tm *t_struct;
    time_t ctime;
    
    ctime = time( NULL );
    
    t_struct= localtime( &ctime );
    
    char tbuffer[256];
    
    snprintf( tbuffer, 256, "dasher-%d%d%d-%d%d.txt", (t_struct->tm_year+1900), (t_struct->tm_mon+1), t_struct->tm_mday, t_struct->tm_hour, t_struct->tm_min);
    
    filename = g_strdup (tbuffer);
    gtk_window_set_title(GTK_WINDOW(window), filename);
  } else {
    gtk_window_set_title(GTK_WINDOW(window), "Dasher");
    filename = NULL;
  }
}

extern "C" void clipboard_copy(void) {
  dasher_copy();
}

extern "C" void clipboard_cut(void) {
  dasher_cut();
}

extern "C" void clipboard_paste(void) {
  dasher_paste();
}

extern "C" void clipboard_copy_all(void) {
  dasher_copy_all();
}

extern "C" void clipboard_select_all(void) {
  dasher_select_all();
}

extern "C" void orientation(GtkRadioButton *widget, gpointer user_data)
{
  // Again, this could be neater.
  if (GTK_TOGGLE_BUTTON(widget)->active==TRUE) {
    if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"radiobutton1")) {
      dasher_set_orientation(Alphabet);
    } else if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"radiobutton2")) {
      dasher_set_orientation(LeftToRight);
    } else if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"radiobutton3")) {
      dasher_set_orientation(RightToLeft);
    } else if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"radiobutton4")) {
      dasher_set_orientation(TopToBottom);
    } else if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"radiobutton5")) {
      dasher_set_orientation(BottomToTop);
    }
  }

  dasher_redraw();
}

extern "C" void set_dasher_fontsize(GtkWidget *widget, gpointer user_data)
{
  if (GTK_CHECK_MENU_ITEM(widget)->active==TRUE) {
    if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"fontsizenormal")) {
      dasher_set_parameter_int( INT_DASHERFONTSIZE, Normal);
    } else if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"fontsizelarge")) {
      dasher_set_parameter_int( INT_DASHERFONTSIZE, Big);
    } else if (GTK_WIDGET(widget)==glade_xml_get_widget(widgets,"fontsizevlarge")) {
      dasher_set_parameter_int( INT_DASHERFONTSIZE, VBig);
    }
    dasher_redraw();
  }
}

extern "C" void show_toolbar(GtkWidget *widget, gpointer user_data)
{
  if(GTK_TOGGLE_BUTTON(widget)->active) {
    dasher_set_parameter_bool( BOOL_SHOWTOOLBAR, true );
  } else {
    dasher_set_parameter_bool( BOOL_SHOWTOOLBAR, false );
  }
}

extern "C" void show_slider(GtkWidget *widget, gpointer user_data)
{
  if(GTK_TOGGLE_BUTTON(widget)->active) {
    dasher_set_parameter_bool( BOOL_SHOWSPEEDSLIDER, true );
  } else {
    dasher_set_parameter_bool( BOOL_SHOWSPEEDSLIDER, false );
  }
}

extern "C" void timestamp_files(GtkWidget *widget, gpointer user_data )
{
  if(GTK_TOGGLE_BUTTON(widget)->active) {
    dasher_set_parameter_bool( BOOL_TIMESTAMPNEWFILES, true );
  } else {
    dasher_set_parameter_bool( BOOL_TIMESTAMPNEWFILES, false );
  }
}

extern "C" void copy_all_on_stop(GtkWidget *widget, gpointer user_data)
{
  if(GTK_TOGGLE_BUTTON(widget)->active) {
    dasher_set_parameter_bool( BOOL_COPYALLONSTOP, true );
  } else {
    dasher_set_parameter_bool( BOOL_COPYALLONSTOP, false );
  }
}

extern "C" void file_encoding(GtkWidget *widget, gpointer user_data)
{
    //  signed int realaction = action -3;
  //  if( GTK_TOGGLE_BUTTON(widget)->active) {
  //    dasher_set_encoding( Dasher::Opts::FileEncodingFormats(realaction) );
  //  }
  //  interface->SetFileEncoding(Opts::FileEncodingFormats(realaction));
  //FIXME - need to reimplement this
  //Actually, I'd be inclined to just get rid of the damn thing.
  //When we open the file, we have no real idea what format it's in - 
  //We assume that it's UTF-8 because that's sane. We should save files
  //based on the user locale and just forget about it.
}

extern "C" void SetDimension(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool(BOOL_DIMENSIONS, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void SetEyetracker(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool(BOOL_EYETRACKER, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void startonleft(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_STARTONLEFT, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void startonspace(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_STARTONSPACE, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void startonmousepos(GtkWidget *widget, gpointer user_data)
{
 dasher_set_parameter_bool( BOOL_MOUSEPOSSTART, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void keycontrol(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_KEYBOARDCONTROL, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void windowpause(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_WINDOWPAUSE, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void controlmode(GtkWidget *widget, gpointer user_data)
{
  controlmodeon=GTK_CHECK_MENU_ITEM(widget)->active;
  dasher_set_parameter_bool( BOOL_CONTROLMODE, GTK_CHECK_MENU_ITEM(widget)->active );
  dasher_start();
  dasher_redraw();
}

extern "C" void keyboardmode(GtkWidget *widget, gpointer user_data)
{
  if (textentry==TRUE) {
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"keyboardmode")), true);
  } else {
    keyboardmodeon=GTK_CHECK_MENU_ITEM(widget)->active;
    dasher_set_parameter_bool( BOOL_KEYBOARDMODE, GTK_CHECK_MENU_ITEM(widget)->active );
  }
}

extern "C" void DrawMouse(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_DRAWMOUSE, GTK_TOGGLE_BUTTON(widget)->active );
  dasher_redraw();
}

extern "C" void DrawMouseLine(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_DRAWMOUSELINE, GTK_TOGGLE_BUTTON(widget)->active );
  dasher_redraw();
}

extern "C" void button_cyclical_mode(GtkWidget *widget, gpointer user_data)
{
  cyclickeyboardmodeon=GTK_TOGGLE_BUTTON(widget)->active;
  set_bool_option_callback("Cyclicalbuttons",cyclickeyboardmodeon);
}

extern "C" void about_dasher(GtkWidget *widget, gpointer user_data)
{
#ifdef GNOME_LIBS
  // Give them a lovely Gnome-style about box
  GdkPixbuf* pixbuf = NULL;

  // In alphabetical order
  gchar *authors[] = {
    "Chris Ball",
    "Phil Cowens",
    "Matthew Garrett",
    "Iain Murray",
    "Hanna Wallach",
    "David Ward",
    NULL
  };
    
  // Yeah, should really do some Gnome documentation for it...
  gchar *documenters[] = {
    "Matthew Garrett",
    NULL
  };

  // This gets pulled out via gettext
  gchar *translator_credits = _("translator_credits");
  
  about = gnome_about_new (_("Dasher"), 
			   PACKAGE_VERSION, 
			   "Copyright The Dasher Project\n",
			   _("Dasher is a predictive text entry application"),
			   (const char **)authors,
			   (const char **)documenters,
			   strcmp (translator_credits, "translator_credits") != 0 ? (const char *)translator_credits : NULL,
			   NULL);
  
  gtk_window_set_transient_for (GTK_WINDOW(about), GTK_WINDOW (window));
  //  g_signal_connect (G_OBJECT (about), "destory", G_CALLBACK (gtk_widget_destroyed), &about);
  gtk_widget_show(about);
#else
  // EAT UGLY ABOUT BOX, PHILISTINE
  GtkWidget *label, *button;
  char *tmp;
  
  about = gtk_dialog_new();

  gtk_dialog_set_has_separator(GTK_DIALOG(about), FALSE);
  gtk_window_set_title(GTK_WINDOW(about), "About Dasher");

  tmp = g_strdup_printf("Dasher Version %s ", VERSION);
  label = gtk_label_new(tmp);
  gtk_widget_show(label);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(about)->vbox),label, FALSE, FALSE, 0);
  
  label = gtk_label_new("http://www.inference.phy.cam.ac.uk/dasher/");
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new("Copyright The Dasher Project");
  gtk_widget_show(label);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(about)->vbox),label, TRUE, TRUE, 0);
  
  button = gtk_button_new_from_stock(GTK_STOCK_OK);
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox),button, FALSE, FALSE, 0);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(about));

  gtk_widget_show (about);
#endif
}

extern "C" void get_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(dasher_fontselector);
  if (font_name) {
    dasher_set_parameter_string( STRING_DASHERFONT, font_name );
    dasherfont=font_name;
    set_canvas_font(font_name);
  }
  fontsel_hide(NULL,NULL);
  dasher_redraw();
}

extern "C" void set_dasher_font(GtkWidget *widget, gpointer user_data)
{
  g_signal_connect (dasher_fontselector->ok_button, "clicked", G_CALLBACK (get_font_from_dialog), (gpointer) dasher_fontselector);
  gtk_window_set_transient_for(GTK_WINDOW(dasher_fontselector),GTK_WINDOW(window));
  gtk_font_selection_dialog_set_font_name(dasher_fontselector,dasherfont.c_str());
  gtk_window_present(GTK_WINDOW(dasher_fontselector));
}

extern "C" void get_edit_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(edit_fontselector);
  if (font_name) {
    dasher_set_parameter_string( STRING_EDITFONT, font_name );
    set_editbox_font(font_name);
    editfont=font_name;
  }
  edit_fontsel_hide(NULL,NULL);
  dasher_redraw();
}

extern "C" void set_edit_font(GtkWidget *widget, gpointer user_data)
{
  g_signal_connect (edit_fontselector->ok_button, "clicked", G_CALLBACK (get_edit_font_from_dialog), (gpointer) edit_fontselector);
  gtk_window_set_transient_for(GTK_WINDOW(edit_fontselector),GTK_WINDOW(window));
  gtk_font_selection_dialog_set_font_name(edit_fontselector,editfont.c_str());
  GtkWidget *cancel_butto3 = glade_xml_get_widget(widgets,"cancel_butto3");
  gtk_widget_hide(cancel_butto3);
  gtk_window_present(GTK_WINDOW(edit_fontselector));
}

extern "C" void reset_fonts(GtkWidget *widget, gpointer user_data)
{
  reset_edit_font();
  reset_dasher_font();
  dasher_set_parameter_string( STRING_DASHERFONT, "DASHERFONT" );
  dasher_set_parameter_string( STRING_EDITFONT, "Sans 10" );
  editfont="Sans 10";
  dasherfont="DASHERFONT";
}

extern "C" void speak(GtkWidget *widget, gpointer user_data)
{
  speakonstop=GTK_TOGGLE_BUTTON(widget)->active;
  dasher_set_parameter_bool( BOOL_SPEECHMODE, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void outlineboxes(GtkWidget *widget, gpointer user_data)
{
  drawoutline=GTK_TOGGLE_BUTTON(widget)->active;
  dasher_set_parameter_bool( BOOL_OUTLINEMODE, GTK_TOGGLE_BUTTON(widget)->active );
  // Instant apply looks significantly less instant otherwise
  dasher_redraw();
}

extern "C" void palettechange(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_PALETTECHANGE, GTK_TOGGLE_BUTTON(widget)->active );
  dasher_redraw();
}

extern "C" void mouseposstart_y_changed(GtkRange *widget, gpointer user_data)
{
  mouseposstartdist=int(widget->adjustment->value);
  set_long_option_callback("Mouseposstartdistance",mouseposstartdist);
  // Need to redraw the boxes
  dasher_redraw();
}

extern "C" void y_scale_changed(GtkRange *widget, gpointer user_data)
{
  yscale=int(widget->adjustment->value);
  set_long_option_callback("YScale",yscale);
}

// Callbacks to be notified of when something changes

void parameter_string_callback( string_param p, const char *value )
{
  switch(p)
    {
    case STRING_DASHERFONT:
      set_canvas_font(value);
      dasherfont=value;
      if (dasherfont=="") {
	dasherfont="DASHERFONT";
      }
      break;
    case STRING_EDITFONT:
      set_editbox_font(value);
      editfont=value;
      if (editfont=="") {
	editfont="Sans 10";
      }
      break;
    }
}

void parameter_double_callback( double_param p, double value )
{
  switch(p)
    {
    case DOUBLE_MAXBITRATE:
      bitrate=value;
      gtk_range_set_value(GTK_RANGE(speed_hscale), value);
      break;
    }
}

void parameter_int_callback( int_param p, long int value )
{
  switch(p)
    {
    case INT_ORIENTATION:
      switch(value)
	{
	case Opts::Alphabet:
	  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton1"))) != TRUE)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton1")), TRUE);
	  break;
	case Opts::LeftToRight:
	  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton2"))) != TRUE)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton2")), TRUE);
	  break;	  
	case Opts::RightToLeft:
	  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton3"))) != TRUE)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton3")), TRUE);
	  break;
	case Opts::TopToBottom:
	  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton4"))) != TRUE)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton4")), TRUE);
	  break;
	case Opts::BottomToTop:
	  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton5"))) != TRUE)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton5")), TRUE);
	  break;
	}
      break;
    case INT_ENCODING:
      fileencoding=value;
      switch(value)
	{
	case Opts::UserDefault:
	  //	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu_bar, "/Options/File Encoding/User Default")), TRUE);
	  break;
	case Opts::AlphabetDefault:
	  //	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu_bar, "/Options/File Encoding/Alphabet Default")), TRUE);
	  break;
	case Opts::UTF8:
	  //	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu_bar, "/Options/File Encoding/Unicode UTF8")), TRUE);
	  break;
	case Opts::UTF16LE:
	  //	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu_bar, "/Options/File Encoding/Unicode UTF16 (LE)")), TRUE);
	  break;
	case Opts::UTF16BE:
	  //	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu_bar, "/Options/File Encoding/Unicode UTF16 (BE)")), TRUE);
	  break;
	}
      break;
    case INT_DASHERFONTSIZE:
      switch(value)
	{
	case Opts::Normal:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"fontsizenormal")), TRUE);
	  break;
	case Opts::Big:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"fontsizelarge")), TRUE);
	  break;
	case Opts::VBig:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"fontsizevlarge")), TRUE);
	  break;
	}
      break;
    case INT_UNIFORM:
      gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets,"uniformhscale")), float(value)/10);
      break;
    case INT_EDITHEIGHT:
      if (textentry==FALSE) {
	editheight=value;
	gtk_paned_set_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1")),value);
	dasher_redraw();
      }
      break;
    case INT_SCREENWIDTH:
      window_x=value;
#ifndef WITH_GPE
      if (setup==true) {
	setup=false;
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
	setup=true;
      } else {
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
      }
#endif
      dasher_redraw();
      break;
    case INT_SCREENHEIGHT:
      window_y=value;
#ifndef WITH_GPE
      // We don't want to remember window size if we're running under GPE
      // - we should just be the right size
      if (setup==true) {
	setup=false;
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
	setup=true;
      } else {
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
      }
#endif
      dasher_redraw();
      break;
    }
}

void parameter_bool_callback( bool_param p, bool value )
{
  switch(p)
    {
    case BOOL_SHOWTOOLBAR:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"toolbarbutton")), value);
      showtoolbar=value;

      if (toolbar==NULL) 
	break;

#ifndef WITH_GPE // Don't show the toolbar if running under GPE
      if (value) {
	if (textentry==FALSE) {
	  gtk_widget_show(toolbar);
	}
      } else {
	gtk_widget_hide(toolbar);
      }
#endif

      break;
    case BOOL_SHOWSPEEDSLIDER:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"speedsliderbutton")), value);
      showslider=value;

      if (speed_frame==NULL) 
	break;

      if (value) {
	gtk_widget_show(speed_frame);
	gtk_range_set_value(GTK_RANGE(speed_hscale), bitrate);
      } else {
	gtk_widget_hide(speed_frame);
      }
      break;
    case BOOL_DRAWMOUSE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"showmousebutton")), value);
      break;
    case BOOL_DRAWMOUSELINE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"showmouselinebutton")), value);
      break;
    case BOOL_DIMENSIONS:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"onedbutton")), value);
      onedmode=value;
      break;
    case BOOL_EYETRACKER:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"eyetrackerbutton")), value);
      eyetrackermode=value;
      break;
    case BOOL_TIMESTAMPNEWFILES:
      timestamp=value;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"timestampbutton")), value);
      break;
    case BOOL_COPYALLONSTOP:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"copyallstopbutton")), value);
      break;
    case BOOL_STARTONLEFT:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"leftbutton")), value);
      startleft=value;
      break;
    case BOOL_STARTONSPACE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"spacebutton")), value);
      startspace=value;
      break;
    case BOOL_MOUSEPOSSTART:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"mouseposbutton")), value);
      mouseposstart=value;
      firstbox=value;
      secondbox=false;
      // Make them appear now
      dasher_redraw();
      break;
    case BOOL_KEYBOARDCONTROL:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"keyboardbutton")), value);
      keyboardcontrol=value;
      break;
    case BOOL_WINDOWPAUSE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"winpausebutton")), value);
      leavewindowpause=value;
      break;
    case BOOL_CONTROLMODE:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"controlmode")), value);
      controlmodeon=value;
      dasher_start();
      dasher_redraw();
      break;
    case BOOL_KEYBOARDMODE:
      if (textentry==FALSE) {
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"keyboardmode")), value);
	keyboardmodeon=value;
      }
      break;
    case BOOL_OUTLINEMODE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"outlinebutton")), value);
      drawoutline=value;
      break;
    case BOOL_PALETTECHANGE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"palettebutton")), value);
      break;
    case BOOL_SPEECHMODE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"speakbutton")), value);
      speakonstop=value;
      break;
    }
}

void interface_cleanup() {
  // Functions that need to be called as we're shutting down
  cleanup_edit();
}

void stop() {
  // Actually starts Dasher if we're already stopped. I'd rename it,
  // but I derive perverse satisfaction from this.
  if (paused == TRUE) {
    dasher_unpause( get_time() );
    paused = FALSE;
    starttime=starttime2=0;
    dasherstarttime=time(NULL);
  } else {
    // should really be the current position, but that's not necessarily anywhere near the canvas
    // and it doesn't seem to actually matter in any case
    dasher_pause(0,0);    
    if (onedmode==true) {
      // Don't immediately jump back to full speed if started in one-dimensional mode
      // (I wonder how many of our heuristics violate the principle of least surprise?)
      dasher_halt();
    }
    paused = TRUE;
#ifdef GNOME_SPEECH
    if (speakonstop==true)
      speak();
#endif
    if (stdoutpipe==true) {
      outputpipe();
    }
    if (timedata==TRUE) {
      // Just a debugging thing, output to the console
      printf(_("%d characters output in %d seconds\n"),outputcharacters,
	     time(NULL)-dasherstarttime);
      outputcharacters=0;
    }
    if (mouseposstart==true) {
      firstbox=true;
      dasher_redraw();
    }
  }
}

void scan_alphabet_files()
{
  // Hurrah for glib making this a nice easy thing to do
  // rather than the WORLD OF PAIN it would otherwise be
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  alphabetglob=g_pattern_spec_new("alphabet*xml");
  directory = g_dir_open(system_data_dir,0,NULL);

  while(filename=g_dir_read_name(directory)) {
    if (alphabet_filter(filename)) {
      add_alphabet_filename(filename);
    }
  }

  directory = g_dir_open(user_data_dir,0,NULL);

  while(filename=g_dir_read_name(directory)) {
    if (alphabet_filter(filename)) {
      add_alphabet_filename(filename);
    }
  }
}

void scan_colour_files()
{
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  colourglob=g_pattern_spec_new("colour*xml");
  directory = g_dir_open(system_data_dir,0,NULL);

  while(filename=g_dir_read_name(directory)) {
    if (colour_filter(filename)) {
      add_colour_filename(filename);
    }
  }

  directory = g_dir_open(user_data_dir,0,NULL);

  while(filename=g_dir_read_name(directory)) {
    if (colour_filter(filename)) {
      add_colour_filename(filename);
    }
  }
}

int alphabet_filter(const gchar* filename)
{
  return int(g_pattern_match_string(alphabetglob,filename));
}

int colour_filter(const gchar* filename)
{
  return int(g_pattern_match_string(colourglob,filename));
}
