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

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#ifdef GNOME_SPEECH
#include <libbonobo.h>
#include <gnome-speech/gnome-speech.h>
#endif

#include "libdasher.h"

#include "dasher.h"
#include "canvas.h"
#include "edit.h"
#include "accessibility.h"
#include "settings_store.h"

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

GtkWidget *vbox, *toolbar;
GdkPixbuf *p;
GtkWidget *pw;
GtkWidget *text_view;
GtkWidget *speed_frame;
GtkWidget *speed_hscale;
GtkWidget *text_scrolled_window;
GtkStyle *style;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;
GtkWidget *vpane;
GtkFontSelectionDialog *dasher_fontselector;
GtkTreeSelection *alphselection, *colourselection;
GtkWidget *preferences_window;
GtkListStore *alph_list_store;
GtkListStore *colour_list_store;
GladeXML *widgets;
GtkWidget *filesel;


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

gint dasherwidth, dasherheight;
long yscale, mouseposstartdist=0;
gboolean coordcalled;

gint buttonnum=0;

extern gboolean timedata;
extern gboolean drawoutline;

gint prev_pos_x;
gint prev_pos_y;

gint fileencoding;

gint outputcharacters;

time_t starttime=0;
time_t starttime2=0;

const gchar *filename = NULL;

GtkWidget *window;
GtkWidget *file_selector;

void 
load_training_file (const gchar *filename)
{
  dasher_train_file( filename );
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

    dasher_set_parameter_string( STRING_ALPHABET, alph );
    
    force_dasher_redraw();

    g_free(alph);

    add_control_tree(gettree());
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
    
    force_dasher_redraw();

    g_free(colour);
  }
}

extern "C" void 
generate_preferences(GtkWidget *widget, gpointer user_data) { 
  int alphabet_count,colour_count;

  const int alphabetlist_size = 128;
  const char *alphabetlist[ alphabetlist_size ];
  const int colourlist_size=128;
  const char *colourlist[ colourlist_size ];
  GtkTreeIter alphiter, colouriter;

  GtkWidget *alphabettreeview = glade_xml_get_widget(widgets,"AlphabetTree");  
  alph_list_store = gtk_list_store_new(1,G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(alphabettreeview), GTK_TREE_MODEL(alph_list_store));
  alphselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(alphabettreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(alphselection),GTK_SELECTION_SINGLE);
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("Alphab\et",gtk_cell_renderer_text_new(),"text",0,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(alphabettreeview),column);

  // Clear the contents of the alphabet list
  gtk_list_store_clear( alph_list_store );

  // And repopulate it with an up to date list
  alphabet_count = dasher_get_alphabets( alphabetlist, alphabetlist_size );

  // Connect up a signal so we can select a new alphabet
  g_signal_connect_after(G_OBJECT(alphselection),"changed",GTK_SIGNAL_FUNC(alphabet_select),NULL);

  for (int i=0; i<alphabet_count; ++i) {
    gtk_list_store_append (alph_list_store, &alphiter);
    gtk_list_store_set (alph_list_store, &alphiter, 0, alphabetlist[i],-1);
  }
  
  // Do the same for colours
  GtkWidget *colourtreeview = glade_xml_get_widget(widgets,"ColourTree");  
  colour_list_store = gtk_list_store_new(1,G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(colourtreeview), GTK_TREE_MODEL(colour_list_store));
  colourselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(colourtreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(colourselection),GTK_SELECTION_SINGLE);
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
  }

}

extern "C" void
preferences_display(GtkWidget *widget, gpointer user_data)
{
  if (preferences_window==NULL)
    preferences_window=glade_xml_get_widget(widgets, "preferences");
  gtk_widget_show_all(preferences_window);
}
  
extern "C" gboolean
preferences_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide_all(preferences_window);
  return TRUE;
}

extern "C" gboolean
button_preferences_show(GtkWidget *widget, gpointer user_data)
{
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
  gtk_widget_show_all(glade_xml_get_widget(widgets,"buttonprefs"));
}

extern "C" gboolean
button_preferences_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide_all(glade_xml_get_widget(widgets,"buttonprefs"));
}

extern "C" gboolean
fontsel_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide_all(GTK_WIDGET(dasher_fontselector));
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

extern "C" void 
open_file (const char *filename)
{
  struct stat file_stat;
  FILE *fp;
  int pos = 0;
  gchar *buffer;

  stat (filename, &file_stat);
  buffer = (gchar *) g_malloc (file_stat.st_size);
  fp = fopen (filename, "r");
  fread (buffer, file_stat.st_size, 1, fp);
  fclose (fp);
  
  dasher_clear();
  
  file_modified = 1;

  gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER (the_text_buffer), buffer, file_stat.st_size);

  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW (the_text_view),gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(the_text_buffer)));

  gtk_window_set_title(GTK_WINDOW(window), filename);

  dasher_start();
  dasher_redraw();
}

extern "C" void
open_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  open_file (filename);

  gtk_widget_destroy (GTK_WIDGET(selector));
}


extern "C" void
select_open_file(GtkWidget *widget, gpointer user_data)
{
  if (filesel==NULL)
    filesel = glade_xml_get_widget(widgets, "dasher_fileselector");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
		    "clicked", G_CALLBACK (open_file_from_filesel), (gpointer) filesel);
  gtk_widget_show (filesel);
}

extern "C" void
select_new_file(GtkWidget *widget, gpointer user_data)
{
  //FIXME - confirm this

  choose_filename();

  clear_edit();
  add_control_tree(gettree());
  paused=false;
  dasher_start();
  dasher_redraw();
  dasher_pause(0,0);
  paused=true;

}

extern "C" void 
save_file_as (const char *filename, bool append)
{
  FILE *fp;
  gint length;
  gchar *inbuffer,*outbuffer = NULL;
  gsize bytes_read, bytes_written;
  GError *error = NULL;
  GIConv cd;

  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  if (append == true) {
    fp = fopen (filename, "a");
  } else {
    fp = fopen (filename, "w");
  }

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1);

  inbuffer = gtk_text_iter_get_slice (start,end);

  length = strlen(inbuffer);

  switch (fileencoding) {
  case Opts::UserDefault:
  case Opts::AlphabetDefault:
    //FIXME - need to call GetAlphabetType and do appropriate stuff
    //FIXME - error handling  
    outbuffer=g_locale_from_utf8(inbuffer,length,&bytes_read,&bytes_written,&error);
    break;
  case Opts::UTF8:
    outbuffer=inbuffer;
    bytes_written=length;
    break;
  case Opts::UTF16LE:
    cd=g_iconv_open("UTF16LE","UTF8");
    outbuffer=g_convert_with_iconv(inbuffer,length,cd,&bytes_read,&bytes_written,&error);
    break;
  case Opts::UTF16BE:
    cd=g_iconv_open("UTF16BE","UTF8");
    outbuffer=g_convert_with_iconv(inbuffer,length,cd,&bytes_read,&bytes_written,&error);
    break;
  }	       
	       
  fwrite(outbuffer,1,bytes_written,fp);
  fclose (fp);

  file_modified = 0;
  gtk_window_set_title(GTK_WINDOW(window), filename);
}

extern "C" void
save_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,FALSE);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

extern "C" void
save_file_from_filesel_and_quit ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,FALSE);

  gtk_exit(0);
}

extern "C" void
append_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,TRUE);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

extern "C" void
select_save_file_as(GtkWidget *widget, gpointer user_data)
{
  if (filesel==NULL)
    filesel = glade_xml_get_widget(widgets, "dasher_fileselector");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
		    "clicked", G_CALLBACK (save_file_from_filesel), (gpointer) filesel);

  if (filename!=NULL)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filesel), filename);

  gtk_widget_show (filesel);
}

extern "C" void
select_save_file_as_and_quit(GtkWidget *widget, gpointer user_data)
{
  if (filesel==NULL)
    filesel = glade_xml_get_widget(widgets, "dasher_fileselector");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
		    "clicked", G_CALLBACK (save_file_from_filesel_and_quit), (gpointer) filesel);

  if (filename!=NULL)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filesel), filename);

  gtk_widget_show (filesel);
}

extern "C" void
import_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  load_training_file(filename);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

extern "C" void
select_append_file(GtkWidget *widget, gpointer user_data)
{
  if (filesel==NULL)
    filesel = glade_xml_get_widget(widgets, "dasher_fileselector");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
		    "clicked", G_CALLBACK (append_file_from_filesel), (gpointer) filesel);
  
  gtk_widget_show (filesel);
}

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
    save_file_as(filename,FALSE);
    gtk_exit(0);
  }
  else {
    select_save_file_as_and_quit(NULL,NULL);
  }
}

extern "C" void
select_import_file(GtkWidget *widget, gpointer user_data)
{
  if (filesel==NULL)
    filesel = glade_xml_get_widget(widgets, "dasher_fileselector");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
		    "clicked", G_CALLBACK (import_file_from_filesel), (gpointer) filesel);
  gtk_widget_show (filesel);
}

extern "C" void
filesel_hide(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_hide (filesel);
}

extern "C" void 
toolbar_save(GtkWidget *widget, gpointer data)
{
}

extern "C" bool
ask_save_before_exit(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog = NULL;

  if (file_modified == TRUE) {
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
      gtk_exit(0);
      break;
    case GTK_RESPONSE_CANCEL:
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
    write_to_file();
    gtk_exit (0);
  }
}

extern "C" void 
toolbar_cut(GtkWidget *widget, gpointer data)
{
  gtk_editable_cut_clipboard(GTK_EDITABLE(the_text_view));

  return;
}

extern "C" void 
toolbar_copy(GtkWidget *widget, gpointer data)
{
  gtk_editable_copy_clipboard(GTK_EDITABLE(the_text_view));

  return;
}

extern "C" void 
toolbar_paste(GtkWidget *widget, gpointer data)
{
  gtk_editable_paste_clipboard(GTK_EDITABLE(the_text_view));

  return;
}

long get_time() {
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
  if (!paused) {
    int x;
    int y;

    if (leavewindowpause==true) {
      gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);

      gdk_window_get_pointer(GTK_WIDGET(window)->window, &x, &y, NULL);

      if (x>dasherwidth || x<0 || y>dasherheight || y<0) {
	return 1;
      }
    }
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    if (onedmode==true) {
      float scalefactor;
      float newy=y;
      gdk_window_get_size(the_canvas->window, &dasherwidth, &dasherheight);
      if (yscale==0) {
	scalefactor=2;
      } else {
	float scalefactor=dasherheight/yscale;
      }
      newy-=dasherheight/2;
      newy*=scalefactor;
      newy+=dasherheight/2;
      y=int(newy);
    } 
    dasher_tap_on( x, y, get_time() );
  }

  else if (mouseposstart==true) {
    int x,y;
    dasherheight=the_canvas->allocation.height;
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    if (firsttime==firstbox==secondbox==false) {
      firstbox=true;
      draw_mouseposbox(0);
    }
    if (y>(dasherheight/2-mouseposstartdist-100) && y<(dasherheight/2-mouseposstartdist)) {
      if (starttime==0) {
	starttime=time(NULL);
      } else {
	if ((time(NULL)-starttime)>2) {
	  draw_mouseposbox(1);
	  secondbox=true;
	  firstbox=false;
	}
      }
    } else if (y<(dasherheight/2+mouseposstartdist+100) && y>(dasherheight/2+mouseposstartdist) && secondbox==true) {      
      if (starttime2==0) {
	starttime2=time(NULL);
	starttime=0;
      } else {
	if ((time(NULL)-starttime2)>2) {
	  stop(); // Yes, confusingly named
	}
      }
    } else {
      if (secondbox==true && starttime2>0) {
	draw_mouseposbox(0);
	secondbox=false;
	firstbox=true;
      }
      starttime=starttime2=0;
    }
  }

  // need non-zero return value so timer repeats
  return 1;
}

extern "C" gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  gdk_draw_pixmap(the_canvas->window,
		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		  onscreen_buffer,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  if (firsttime==TRUE) {
    paused=true;
    firsttime=false;
    if (mouseposstart==true) {
      draw_mouseposbox(0);
    }
  }

  return TRUE;
}

extern "C" gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  rebuild_buffer();

  dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );

  force_dasher_redraw();

  if (setup==TRUE) {
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
    dasher_start();
    force_dasher_redraw();
    return FALSE;
  }
}

extern "C" void
edit_key_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if(keycontrol==false) {
    dasher_start();
    force_dasher_redraw();
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

extern "C" void
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
  gboolean *returnType;

  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_2BUTTON_PRESS))
    return;

  focusEvent->type = GDK_FOCUS_CHANGE;
  focusEvent->window = (GdkWindow *) the_canvas;
  focusEvent->send_event = FALSE;
  focusEvent->in = TRUE;

  gtk_widget_grab_focus(GTK_WIDGET(the_canvas));  
  gtk_signal_emit_by_name(GTK_OBJECT(the_canvas), "focus_in_event", GTK_WIDGET(the_canvas), focusEvent, NULL, &returnType);

  g_free(focusEvent);

  // CJB,  2003-08.  If we have a selection, replace it with the new input.
  // This code is duplicated in key_press_event.
  if (gtk_text_buffer_get_selection_bounds (the_text_buffer, NULL, NULL))
    gtk_text_buffer_cut_clipboard(the_text_buffer, the_text_clipboard, TRUE);
  
  if (startleft == TRUE) {
    stop();
  }
  return;
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

  float initial_bitrate = 3.0;

  the_canvas=glade_xml_get_widget(xml, "the_canvas");
  text_scrolled_window=glade_xml_get_widget(xml, "text_scrolled_window");
  the_text_view=glade_xml_get_widget(xml, "the_text_view");

  speed_frame=glade_xml_get_widget(xml, "speed_frame");
  speed_hscale=glade_xml_get_widget(xml, "speed_hscale");

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

  // Configure the buttons
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
open_window(GladeXML *xml) {
  char *system_data_dir;
  char *home_dir;
  char *user_data_dir;

  home_dir = getenv( "HOME" );
  user_data_dir = new char[ strlen( home_dir ) + 10 ];
  sprintf( user_data_dir, "%s/.dasher/", home_dir );

  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  system_data_dir = PROGDATA"/";
  
  dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
  dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

  window=glade_xml_get_widget(xml, "window");
  vbox=glade_xml_get_widget(xml, "vbox1");
  vpane=glade_xml_get_widget(xml, "vpaned1");
  toolbar=glade_xml_get_widget(xml, "toolbar");
  dasher_menu_bar=glade_xml_get_widget(xml, "dasher_menu_bar");
  dasher_fontselector=GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(xml, "dasher_fontselector"));

  generate_preferences(NULL,NULL);

  gtk_window_set_focus(GTK_WINDOW(window), GTK_WIDGET(the_canvas));

  // Focus the canvas
  GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
  gboolean *returnType;
  
  focusEvent->type = GDK_FOCUS_CHANGE;
  focusEvent->window = (GdkWindow *) the_canvas;
  focusEvent->send_event = FALSE;
  focusEvent->in = TRUE;

  gtk_widget_grab_focus(GTK_WIDGET(the_canvas));  
  gtk_signal_emit_by_name(GTK_OBJECT(the_canvas), "focus_in_event", GTK_WIDGET(the_canvas), focusEvent, NULL, &returnType);

  g_free(focusEvent);

  dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
  dasher_set_parameter_int( INT_VIEW, 0 );
  
  const char *alphabet;

  dasher_get_alphabets( &alphabet, 1 );

  dasher_start();
  force_dasher_redraw();

  gtk_timeout_add(50, timer_callback, NULL );  

  // I have no idea why we need to do this when Glade has theoretically done
  // so already, but...
  gtk_widget_add_events (the_canvas, GDK_BUTTON_PRESS_MASK);

  // We need to monitor the text buffer for mark_set in order to get
  // signals when the cursor is moved
  g_signal_connect(G_OBJECT(the_text_buffer), "mark_set", G_CALLBACK(edit_button_release_event), NULL);
}

extern "C" void choose_filename() {
  if (timestamp==TRUE) {
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

  force_dasher_redraw();
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
    force_dasher_redraw();
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
  //FIXME - need to reimplemnt this
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
  force_dasher_redraw();
}

extern "C" void keyboardmode(GtkWidget *widget, gpointer user_data)
{
  keyboardmodeon=GTK_CHECK_MENU_ITEM(widget)->active;
  dasher_set_parameter_bool( BOOL_KEYBOARDMODE, GTK_CHECK_MENU_ITEM(widget)->active );
}

extern "C" void DrawMouse(GtkWidget *widget, gpointer user_data)
{
  dasher_set_parameter_bool( BOOL_DRAWMOUSE, GTK_TOGGLE_BUTTON(widget)->active );
  force_dasher_redraw();
}

extern "C" void button_cyclical_mode(GtkWidget *widget, gpointer user_data)
{
  cyclickeyboardmodeon=GTK_TOGGLE_BUTTON(widget)->active;
  set_bool_option_callback("Cyclicalbuttons",cyclickeyboardmodeon);
}

extern "C" void about_dasher(GtkWidget *widget, gpointer user_data)
{
  GtkWidget *about = NULL;
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
}

extern "C" void get_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(dasher_fontselector);
  if (font_name) {
    dasher_set_parameter_string( STRING_DASHERFONT, font_name );
    set_canvas_font(font_name);
  }
  fontsel_hide(NULL,NULL);
  force_dasher_redraw();
}

extern "C" void set_dasher_font(GtkWidget *widget, gpointer user_data)
{
  g_signal_connect (dasher_fontselector->ok_button, "clicked", G_CALLBACK (get_font_from_dialog), (gpointer) dasher_fontselector);
  gtk_widget_show_all(GTK_WIDGET(dasher_fontselector));
}

extern "C" void get_edit_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(dasher_fontselector);
  if (font_name) {
    dasher_set_parameter_string( STRING_EDITFONT, font_name );
    set_editbox_font(font_name);
  }
  fontsel_hide(NULL,NULL);
  force_dasher_redraw();
}

extern "C" void set_edit_font(GtkWidget *widget, gpointer user_data)
{
  g_signal_connect (dasher_fontselector->ok_button, "clicked", G_CALLBACK (get_edit_font_from_dialog), (gpointer) dasher_fontselector);
  gtk_widget_show_all(GTK_WIDGET(dasher_fontselector));
}

extern "C" void reset_fonts(GtkWidget *widget, gpointer user_data)
{
  reset_edit_font();
  reset_dasher_font();
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
  force_dasher_redraw();
}

extern "C" void mouseposstart_y_changed(GtkRange *widget, gpointer user_data)
{
  mouseposstartdist=int(widget->adjustment->value);
  set_long_option_callback("Mouseposstartdistance",mouseposstartdist);
  force_dasher_redraw();
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
      break;
    case STRING_EDITFONT:
      set_editbox_font(value);
      break;
    }
}

void parameter_double_callback( double_param p, double value )
{
  switch(p)
    {
    case DOUBLE_MAXBITRATE:
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
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"fontsizenormal")), TRUE);
	  break;
	case Opts::VBig:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"fontsizenormal")), TRUE);
	  break;
	}
      break;
    case INT_UNIFORM:
      gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets,"uniformhscale")), float(value)/10);
      break;
    case INT_EDITHEIGHT:
      editheight=value;
      gtk_paned_set_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1")),value);
      force_dasher_redraw();
      break;
    case INT_SCREENWIDTH:
      window_x=value;
      if (setup==true) {
	setup=false;
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
	setup=true;
      } else {
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
      }
      force_dasher_redraw();
      break;
    case INT_SCREENHEIGHT:
      window_y=value;
      if (setup==true) {
	setup=false;
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
	setup=true;
      } else {
	gtk_window_set_default_size (GTK_WINDOW(window), window_x, window_y);
      }
      force_dasher_redraw();
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

      if (value) {
	gtk_widget_show(toolbar);
      } else {
	gtk_widget_hide(toolbar);
      }

      break;
    case BOOL_SHOWSPEEDSLIDER:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"speedsliderbutton")), value);
      showslider=value;

      if (speed_frame==NULL) 
	break;

      if (value) {
	gtk_widget_show(speed_frame);
      } else {
	gtk_widget_hide(speed_frame);
      }
      break;
    case BOOL_DRAWMOUSE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"showmousebutton")), value);
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
      force_dasher_redraw();
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
      force_dasher_redraw();
      break;
    case BOOL_KEYBOARDMODE:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets,"keyboardmode")), value);
      keyboardmodeon=value;
      break;
    case BOOL_OUTLINEMODE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"outlinebutton")), value);
      drawoutline=value;
      break;
    case BOOL_SPEECHMODE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"speakbutton")), value);
      speakonstop=value;
      break;
    }
}

void null_log_handler (const gchar *log_domain, GLogLevelFlags log_level, 
		       const gchar *message, gpointer unused_data) {}
void stop() {
  if (paused == TRUE) {
    dasher_unpause( get_time() );
    paused = FALSE;
    starttime=starttime2=0;
  } else {
    dasher_pause(0,0);    
    paused = TRUE;
#ifdef GNOME_SPEECH
    if (speakonstop==true)
      speak();
#endif
    if (timedata==TRUE) {
      printf("%d characters output in %d seconds\n",outputcharacters,
	     time(NULL)-starttime);
    }
    if (mouseposstart==TRUE) {
      draw_mouseposbox(0);
    }
  }
}

void force_dasher_redraw() {
  if (paused==true) {
    paused=false;    
    dasher_redraw();    
    paused=true;
    if (firstbox==true) {
      draw_mouseposbox(0);
    } else if (secondbox==true) {
      draw_mouseposbox(1);
    }
  } else {
    dasher_redraw();
  }
}
