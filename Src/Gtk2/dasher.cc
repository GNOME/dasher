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

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

GtkWidget *vbox, *toolbar;
GdkPixbuf *p;
GtkWidget *pw;
GtkWidget *text_view;
GtkWidget *speed_frame;
GtkObject *speed_slider;
GtkWidget *speed_hscale;
GtkWidget *text_scrolled_window;
GtkWidget *canvas_frame;
GtkWidget *ofilesel;
GtkWidget *ifilesel;
GtkWidget *afilesel;
GtkStyle *style;
GtkItemFactory *dasher_menu;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;
GtkWidget *vpane;
GtkFontSelectionDialog *dasherfontdialog;
GtkFontSelectionDialog *editfontdialog;

bool controlmodeon=false;

GtkItemFactoryEntry entries[] = {
  { "/_File",         NULL,      NULL,         0, "<Branch>" },
  { "/File/_New",     "<CTRL>N", *GtkItemFactoryCallback(select_new_file),     1, "<StockItem>", GTK_STOCK_NEW },
  { "/File/_Open...", "<CTRL>O", *GtkItemFactoryCallback(select_open_file),    1, "<StockItem>", GTK_STOCK_OPEN },
  { "/File/Save",     NULL, save_file, 0, "<StockItem>", GTK_STOCK_SAVE },
  { "/File/Save As...", NULL, select_save_file_as, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
  { "/File/Append to File...", NULL, select_append_file, 0, "<Item>" },
  { "/File/sep1",     NULL,      NULL,         0, "<Separator>" },
  { "/File/Import Training Text...", NULL, select_import_file, 0, "<Item>" },
  { "/File/sepl", NULL, NULL, 0, "<Separator>" },
  { "/File/_Quit",    "<CTRL>Q", *GtkItemFactoryCallback(ask_save_before_exit), 0, "<StockItem>", GTK_STOCK_QUIT },
  { "/Edit", NULL, NULL, 0, "<Branch>" },
  { "/Edit/Cut", NULL, clipboard_cut, 0, "<StockItem>", GTK_STOCK_CUT },
  { "/Edit/Copy", NULL, clipboard_copy, 0, "<StockItem>", GTK_STOCK_COPY },
  { "/Edit/Paste", NULL, clipboard_paste, 0, "<StockItem>", GTK_STOCK_PASTE },
  { "/Edit/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Edit/Copy All", NULL, clipboard_copy_all, 0, "<Item>" },
  { "/Edit/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Edit/Select All", NULL, clipboard_select_all, 0, "<Item>" },
  { "/View", NULL, NULL, 0, "<Branch>" },
  { "/View/Orientation", NULL, NULL, 0, "<Branch>" },
  { "/View/Orientation/Alphabet Default", NULL, *GtkItemFactoryCallback(orientation), 1, "<RadioItem>" },
  { "/View/Orientation/sepl", NULL, NULL, 0, "<Separator>" },
  { "/View/Orientation/Left to Right", NULL, *GtkItemFactoryCallback(orientation), 3, "/View/Orientation/Alphabet Default" },
  { "/View/Orientation/Right to Left", NULL, *GtkItemFactoryCallback(orientation), 4, "/View/Orientation/Alphabet Default" },
  { "/View/Orientation/Top to Bottom", NULL, *GtkItemFactoryCallback(orientation), 5, "/View/Orientation/Alphabet Default" },
  { "/View/Orientation/Bottom to Top", NULL, *GtkItemFactoryCallback(orientation), 6, "/View/Orientation/Alphabet Default" },
  { "/View/sepl", NULL, NULL, 0, "<Separator>" },
  { "/View/Show Toolbar", NULL, *GtkItemFactoryCallback(show_toolbar), 1, "<CheckItem>" },
  { "/View/Speed Slider", NULL, *GtkItemFactoryCallback(show_slider), 1, "<CheckItem>" },
  //  { "/View/sepl", NULL, NULL, 0, "<Separator>" },
  //  { "/View/Fix Layout", NULL, NULL, 0, "<CheckItem>" },
  { "/Options", NULL, NULL, 0, "<Branch>" },
  { "/Options/Timestamp New Files", NULL, *GtkItemFactoryCallback(timestamp_files), 1, "<CheckItem>" },
  { "/Options/Copy All on Stop", NULL, *GtkItemFactoryCallback(copy_all_on_stop), 1, "<CheckItem>" },
  { "/Options/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Options/Alphabet...", NULL, *GtkItemFactoryCallback(preferences), 1, "<Item>" },
  { "/Options/File Encoding", NULL, NULL, 0, "<Branch>" },
  { "/Options/File Encoding/User Default", NULL, *GtkItemFactoryCallback(file_encoding), 2, "<RadioItem>" },
  { "/Options/File Encoding/Alphabet Default", NULL, *GtkItemFactoryCallback(file_encoding), 1, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF8", NULL, *GtkItemFactoryCallback(file_encoding), 65004, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF16 (LE)", NULL, *GtkItemFactoryCallback(file_encoding), 1203, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF16 (BE)", NULL, *GtkItemFactoryCallback(file_encoding), 1204, "/Options/File Encoding/User Default" },
  { "/Options/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Options/Font Size", NULL, NULL, 0, "<Branch>" },
  { "/Options/Font Size/Default Fonts", NULL, *GtkItemFactoryCallback(set_dasher_fontsize), 1, "<RadioItem>" },
  { "/Options/Font Size/Large Fonts", NULL, *GtkItemFactoryCallback(set_dasher_fontsize), 2, "/Options/Font Size/Default Fonts" },
  { "/Options/Font Size/Very Large Fonts", NULL, *GtkItemFactoryCallback(set_dasher_fontsize), 4, "/Options/Font Size/Default Fonts" },
  { "/Options/Editing Font...", NULL, *GtkItemFactoryCallback(set_edit_font), 1, "<Item>" },
  { "/Options/Dasher Font...", NULL, *GtkItemFactoryCallback(set_dasher_font), 1, "<Item>" },
  { "/Options/Reset Fonts", NULL, *GtkItemFactoryCallback(reset_fonts), 1, "<Item>" },
  { "/Options/One Dimensional", NULL, *GtkItemFactoryCallback(SetDimension), 1, "<CheckItem>" },
  { "/Options/Draw Position", NULL, *GtkItemFactoryCallback(DrawMouse), 1, "<CheckItem>" },
  { "/Options/Start on Left Mouse", NULL, *GtkItemFactoryCallback(startonleft), 1, "<CheckItem>" },
  { "/Options/Start on Space Bar", NULL, *GtkItemFactoryCallback(startonspace), 1, "<CheckItem>" },
  { "/Options/Keyboard Control", NULL, *GtkItemFactoryCallback(keycontrol), 1, "<CheckItem>" },
  { "/Options/Pause outside Window", NULL, *GtkItemFactoryCallback(windowpause), 1, "<CheckItem>" },
  { "/Options/Control Mode", NULL, *GtkItemFactoryCallback(controlmode), 1, "<CheckItem>" },
  { "/Help", NULL, NULL, 0, "<Branch>" },
  { "/Help/About", NULL, *GtkItemFactoryCallback(about_dasher), 0, "<Item>" }
 };

#define _(_x) gettext(_x)

#define TB_NEW "new"
#define TB_OPEN "open"
#define TB_SAVE "save"
#define TB_CUT "cut"
#define TB_COPY "copy"
#define TB_PASTE "paste"
#define TB_PREFERENCES "preferences"

#define NO_PREV_POS -1

guint window_x = 500, window_y = 500;

gboolean setup = FALSE;
gboolean paused = TRUE;
gboolean indrag = FALSE;
gboolean file_modified = FALSE;
gboolean showtoolbar;
gboolean showslider;
gboolean timestamp;
gboolean startleft;
gboolean startspace;
gboolean keyboardcontrol;
gboolean leavewindowpause;

gint dasherwidth, dasherheight;

extern gboolean timedata;

gint prev_pos_x;
gint prev_pos_y;

gint fileencoding;

gint outputcharacters;
time_t starttime;

const gchar *filename = NULL;

GtkWidget *window;
GtkWidget *file_selector;

void 
load_training_file (const gchar *filename)
{
  dasher_train_file( filename );
}

void alphabet_select(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *alph;
  GdkCursor *waitcursor, *arrowcursor;
  GtkWidget *preferences_window = GTK_WIDGET(data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get(model, &iter, 0, &alph, -1);

    dasher_set_parameter_string( STRING_ALPHABET, alph );

    g_free(alph);
  }
}

void 
preferences(gpointer data, guint action, GtkWidget *widget)
{
  GtkTreeSelection *selection;
  GtkWidget *vbox;
  GtkTreeModel *model;
  GtkWidget *treeview;
  GtkWidget *sw;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkWidget *ok;
  
  list_store = gtk_list_store_new(1,G_TYPE_STRING);

  // FIXME - need to check that this is doing the right thing, no
  // memory leaks due to strings not being dealloced etc...

  const int alphabetlist_size = 128;
  const char *alphabetlist[ alphabetlist_size ];

  int alphabet_count;
  
  alphabet_count = dasher_get_alphabets( alphabetlist, alphabetlist_size );

  for (int i=0; i<alphabet_count; ++i) {
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, 0, alphabetlist[i],-1);
  }

  // FIXME - delete strings here????

  GtkWidget *preferences_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (preferences_window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(preferences_window),8);
  vbox = gtk_vbox_new (FALSE,8);
  gtk_container_add(GTK_CONTAINER(preferences_window),vbox);

  sw=gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX(vbox), sw, TRUE, TRUE, 0);

  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(list_store));  
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), 0);
  gtk_container_add (GTK_CONTAINER(sw), treeview);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION(selection),GTK_SELECTION_SINGLE);

  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("Alphabet",gtk_cell_renderer_text_new(),"text",0,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview),column);

  gtk_window_set_default_size (GTK_WINDOW(preferences_window), 280, 250);

  g_signal_connect (selection, "changed", G_CALLBACK(alphabet_select), preferences_window);

  ok = gtk_button_new_from_stock(GTK_STOCK_CLOSE);

  g_signal_connect_swapped (G_OBJECT (ok), "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT(preferences_window));

  gtk_box_pack_start( GTK_BOX(vbox), ok, false, false, 0);

  gtk_widget_show_all(preferences_window);
}
  
void 
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

  file_modified = 0;
  
  gtk_window_set_title(GTK_WINDOW(window), filename);

  dasher_start();
  dasher_redraw();
}

void
open_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  open_file (filename);

  gtk_widget_destroy (GTK_WIDGET(selector));
}


void
select_open_file(gpointer data, guint action, GtkWidget *widget)
{

  GtkWidget *filew;

  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		    "clicked", G_CALLBACK (open_file_from_filesel), (gpointer) filew);
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			    "clicked", G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (filew));
  gtk_widget_show (filew);
}

void
select_new_file(gpointer data, guint action, GtkWidget *widget)
{
  //FIXME - confirm this

  choose_filename();

  clear_edit();
  dasher_start();
  dasher_redraw();

}

void 
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

void
save_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,FALSE);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

void
append_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,TRUE);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

void
select_save_file_as()
{

  GtkWidget *filew;

  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		    "clicked", G_CALLBACK (save_file_from_filesel), (gpointer) filew);
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			    "clicked", G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (filew));

  if (filename!=NULL)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), filename);

  gtk_widget_show (filew);
}

void
import_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{


  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  load_training_file(filename);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

void
select_append_file()
{

  GtkWidget *filew;

  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		    "clicked", G_CALLBACK (append_file_from_filesel), (gpointer) filew);
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			    "clicked", G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (filew));
  gtk_widget_show (filew);
}

void
save_file ()
{
  if (filename != NULL) {
    save_file_as(filename,FALSE);
  }
  else {
    select_save_file_as();
  }
}

void
select_import_file()
{

  GtkWidget *filew;

  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		    "clicked", G_CALLBACK (import_file_from_filesel), (gpointer) filew);
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			    "clicked", G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (filew));
  gtk_widget_show (filew);
}


void 
toolbar_save(GtkWidget *widget, gpointer data)
{
}

void
ask_save_before_exit(GtkWidget *widget, gpointer data)
{
  if (file_modified == TRUE) {
    // FIXME - ask question here
    //        switch(gpe_question_ask ("Save current file before exiting?", _("Question"), "question",_("Don't save"), "stop", _("Save"), "save"))
    //      {
    //      case 1: /* Save */
    //	toolbar_save (widget, data);
    //      case 0: /* Don't Save */
    //	gtk_exit (0);
    //      default:
    //	toolbar_save (widget, data);
    //      } */
  }
  else {
    gtk_exit (0);
  }
}

void 
toolbar_cut(GtkWidget *widget, gpointer data)
{
  gtk_editable_cut_clipboard(GTK_EDITABLE(the_text_view));

  return;
}

void 
toolbar_copy(GtkWidget *widget, gpointer data)
{
  gtk_editable_copy_clipboard(GTK_EDITABLE(the_text_view));

  return;
}

void 
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
    dasher_tap_on( x, y, get_time() );
  }

  // need non-zero return value so timer repeats

  return 1;
}

gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  gdk_draw_pixmap(the_canvas->window,
		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		  onscreen_buffer,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  return TRUE;
}

gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  rebuild_buffer();

  dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );
  dasher_redraw();

  return FALSE;
}

void
edit_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  dasher_start();
  dasher_redraw();
}

void
edit_key_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if(keycontrol==false) {
    dasher_start();
    dasher_redraw();
  }
}

gint
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
      width = the_canvas->allocation.width;
      height = the_canvas->allocation.height;
      dasher_go_to((int)(0.70*width), (int)(0.20*height));
      return TRUE;
    }
    break;
  case GDK_Down:
    if (keyboardcontrol == true) {
      width = the_canvas->allocation.width;
      height = the_canvas->allocation.height;
      dasher_go_to((int)(0.70*width), (int)(0.80*height));
      return TRUE;
    }
    break;
  case GDK_Left:
    if (keyboardcontrol == true) {
      width = the_canvas->allocation.width;
      height = the_canvas->allocation.height;
      dasher_go_to((int)(0.25*width), (int)(0.50*height));
      return TRUE;
    }
    break;
  case GDK_space:
    if (startspace == TRUE) {
      if (paused == TRUE) {
	dasher_unpause( get_time() );
	paused = FALSE;
      } else {
	dasher_pause(0,0);    
	paused = TRUE;
#ifdef GNOME_SPEECH
	speak();
#endif
      }      
    }
    return TRUE;
    break;
  default:
    return FALSE;
  }  
  return FALSE;
}

void
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
    if (paused == TRUE) {
      dasher_unpause( get_time() );
      starttime=time(NULL);
      outputcharacters=0;
      paused = FALSE;      
    } else {
      dasher_pause( (gint) event->x,(gint) event->y );    
      paused = TRUE;
#ifdef GNOME_SPEECH
      speak();
#endif
      if (timedata==TRUE) {
	printf("%d characters output in %d seconds\n",outputcharacters,
	       time(NULL)-starttime);
      }
    }
  }
  return;
}


gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  dasher_pause( (gint) event->x,(gint) event->y );
  paused = TRUE;

  return FALSE;
}

void speed_changed(GtkAdjustment *adj) {

  dasher_set_parameter_double( DOUBLE_MAXBITRATE, adj->value );
}

void interface_setup() {
  dasher_accel = gtk_accel_group_new();
  
    float initial_bitrate = 3.0;

 speed_frame = gtk_frame_new ("Speed");
    speed_slider = gtk_adjustment_new(initial_bitrate, 1.0, 8.0, 1.0, 1.0, 0.0);
    speed_hscale = gtk_hscale_new(GTK_ADJUSTMENT(speed_slider));
    gtk_range_set_update_policy(GTK_RANGE(speed_hscale), GTK_UPDATE_CONTINUOUS);

  initialise_canvas(360,360);
  initialise_edit();

  
  dasher_menu= gtk_item_factory_new( GTK_TYPE_MENU_BAR,
				     "<DasherMenu>",
				     dasher_accel);

  gtk_item_factory_create_items( dasher_menu,
				 57,
				 entries,
				 NULL );

}

void create_toolbar() {
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_NEW, _("New"), _("New"), G_CALLBACK (select_new_file), NULL, -1);
  
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_OPEN, _("Open"), _("Open"), G_CALLBACK (select_open_file), NULL, -1);
  
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_SAVE, _("Save"), _("Save"), G_CALLBACK (save_file), NULL, -1);
  
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_SAVE_AS, _("Save As"), _("Save As"), G_CALLBACK (select_save_file_as), NULL, -1);
    
  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
    
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_CUT, _("Cut"), _("Cut"), G_CALLBACK (clipboard_cut), NULL, -1);
    
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_COPY, _("Copy"), _("Copy"), G_CALLBACK (clipboard_copy), NULL, -1);
    
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_PASTE, _("Paste"), _("Paste"), G_CALLBACK (clipboard_paste), NULL, -1);
}

void create_menu() {
  gtk_window_add_accel_group( GTK_WINDOW(window), dasher_accel);
  dasher_menu_bar=gtk_item_factory_get_widget( dasher_menu, "<DasherMenu>");
}  

void create_edit_box() {
    text_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (text_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (text_scrolled_window), the_text_view);
    gtk_signal_connect(GTK_OBJECT (the_text_view), "button_release_event", GTK_SIGNAL_FUNC (edit_button_release_event), (gpointer) NULL);
    
    gtk_signal_connect(GTK_OBJECT (the_text_view), "key_release_event", GTK_SIGNAL_FUNC (edit_key_release_event), (gpointer) NULL);
    
    gtk_widget_set_events(the_text_view, GDK_BUTTON_RELEASE_MASK | GDK_KEY_RELEASE_MASK);

}

void create_canvas() {
    GTK_WIDGET_SET_FLAGS (GTK_WIDGET (the_canvas), GTK_CAN_FOCUS);
    gtk_signal_connect(GTK_OBJECT (the_canvas), "expose_event", GTK_SIGNAL_FUNC (canvas_expose_event), (gpointer) NULL);
    
    gtk_signal_connect(GTK_OBJECT (the_canvas), "configure_event", GTK_SIGNAL_FUNC (canvas_configure_event), (gpointer) NULL);
    
    gtk_signal_connect(GTK_OBJECT (the_canvas), "button_press_event", GTK_SIGNAL_FUNC (button_press_event), (gpointer) NULL);

    gtk_signal_connect(GTK_OBJECT (the_canvas), "key_press_event", GTK_SIGNAL_FUNC (key_press_event), (gpointer) NULL);
    
    gtk_widget_set_events(the_canvas, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );
    canvas_frame = gtk_frame_new (NULL);
    gtk_container_add (GTK_CONTAINER (canvas_frame), the_canvas);
    
}

GtkWidget*
open_window() {
  char *system_data_dir;
  char *home_dir;
  char *user_data_dir;

  home_dir = getenv( "HOME" );
  user_data_dir = new char[ strlen( home_dir ) + 10 ];
  sprintf( user_data_dir, "%s/.dasher/", home_dir );

  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  // FIXME CHANGE THIS!
  system_data_dir = PROGDATA"/";
  
  dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
  dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window), _("Dasher"));
  gtk_window_set_resizable( GTK_WINDOW(window), TRUE );

  gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (ask_save_before_exit), NULL);

  create_menu();
  create_toolbar();
  create_edit_box();
  create_canvas();

  gtk_window_set_focus(GTK_WINDOW(window), GTK_WIDGET(the_canvas));

  vbox = gtk_vbox_new (FALSE, 0);
  vpane = gtk_vpaned_new();
    
  gtk_box_pack_start (GTK_BOX (vbox), dasher_menu_bar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
    
  gtk_paned_add1(GTK_PANED(vpane),text_scrolled_window);
  gtk_paned_add2(GTK_PANED(vpane),canvas_frame);

  gtk_box_pack_start (GTK_BOX (vbox), vpane, TRUE, TRUE, 0);    
       
  gtk_signal_connect (GTK_OBJECT (speed_slider), "value_changed", GTK_SIGNAL_FUNC (speed_changed), NULL);

  gtk_box_pack_start (GTK_BOX (vbox), speed_frame, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (speed_frame), speed_hscale);

  gtk_container_add (GTK_CONTAINER(window), vbox);

  gtk_widget_set_usize (GTK_WIDGET (window), window_x, window_y);
    
  gtk_widget_realize (window);
  gtk_widget_realize (vpane);
  gtk_widget_realize (vbox);
  gtk_widget_realize (text_scrolled_window);
  gtk_widget_realize (the_text_view);
  gtk_widget_realize (toolbar);
  gtk_widget_realize (canvas_frame);
  gtk_widget_realize (the_canvas);
  gtk_widget_realize (speed_frame);
  gtk_widget_realize (speed_hscale);
  gtk_widget_realize (dasher_menu_bar);
  
  gtk_widget_show (window);
  gtk_widget_show (vpane);
  gtk_widget_show (vbox);
  gtk_widget_show (text_scrolled_window);
  gtk_widget_show (the_text_view);
  gtk_widget_show (canvas_frame);
  gtk_widget_show (the_canvas);
  gtk_widget_show (dasher_menu_bar);
  gtk_widget_show (speed_hscale);

  if (showtoolbar==TRUE)
    gtk_widget_show (toolbar);

  if (showslider==TRUE) {
    gtk_widget_show (speed_frame);
  }

    // FIXME - need to implement this

    //    interface->SettingsDefaults( store );

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

  gtk_timeout_add(50, timer_callback, NULL );  

  setup = TRUE;

  return window;
}

void choose_filename() {
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

void clipboard_copy(void) {
  dasher_copy();
}

void clipboard_cut(void) {
  dasher_cut();
}

void clipboard_paste(void) {
  dasher_paste();
}

void clipboard_copy_all(void) {
  dasher_copy_all();
}

void clipboard_select_all(void) {
  dasher_select_all();
}

void orientation(gpointer data, guint action, GtkWidget  *widget )
{
  signed int RealAction=action-3;

  if( GTK_CHECK_MENU_ITEM(widget)->active)
    {
      dasher_set_orientation( Dasher::Opts::ScreenOrientations(RealAction) );
      dasher_redraw();
    }
}

void set_dasher_fontsize(gpointer data, guint action, GtkWidget  *widget )
{
  if( GTK_CHECK_MENU_ITEM(widget)->active)
    {
      dasher_set_parameter_int( INT_DASHERFONTSIZE, Dasher::Opts::FontSize(action) );
      dasher_redraw();
    }
}

void show_toolbar(gpointer data, guint action, GtkWidget  *widget )
{

  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    dasher_set_parameter_bool( BOOL_SHOWTOOLBAR, true );
  } else {
    dasher_set_parameter_bool( BOOL_SHOWTOOLBAR, false );
  }
}

void show_slider(gpointer data, guint action, GtkWidget  *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    dasher_set_parameter_bool( BOOL_SHOWSPEEDSLIDER, true );
  } else {
    dasher_set_parameter_bool( BOOL_SHOWSPEEDSLIDER, false );
  }
}

void timestamp_files(gpointer data, guint action, GtkWidget *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    dasher_set_parameter_bool( BOOL_TIMESTAMPNEWFILES, true );
  } else {
    dasher_set_parameter_bool( BOOL_TIMESTAMPNEWFILES, false );
  }
}

void copy_all_on_stop(gpointer data, guint action, GtkWidget *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    dasher_set_parameter_bool( BOOL_COPYALLONSTOP, true );
  } else {
    dasher_set_parameter_bool( BOOL_COPYALLONSTOP, false );
  }
}

void file_encoding(gpointer data, guint action, GtkWidget *widget )
{
  signed int realaction = action -3;
  if( GTK_CHECK_MENU_ITEM(widget)->active) {
    dasher_set_encoding( Dasher::Opts::FileEncodingFormats(realaction) );
  }
  //  interface->SetFileEncoding(Opts::FileEncodingFormats(realaction));
  //FIXME - need to reimplemnt this
}

void SetDimension(gpointer data, guint action, GtkWidget *widget )
{
  // FIXME - rewrite this sanely, ie:
  // dasher_set_parameter_bool( BOOL_DRAWMOUSE, GTK_CHECK_MENU_ITEM(widget)->active  );
  // plus the same for the above routines

  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    //    interface->DrawMouse( TRUE );
    dasher_set_parameter_bool( BOOL_DIMENSIONS, true );
  } else {
    //    interface->DrawMouse( FALSE );
    dasher_set_parameter_bool( BOOL_DIMENSIONS, false );
  }
}

void startonleft(gpointer data, guint action, GtkWidget *widget )
{
  dasher_set_parameter_bool( BOOL_STARTONLEFT, GTK_CHECK_MENU_ITEM(widget)->active );
}

void startonspace(gpointer data, guint action, GtkWidget *widget )
{
  dasher_set_parameter_bool( BOOL_STARTONSPACE, GTK_CHECK_MENU_ITEM(widget)->active );
}

void keycontrol(gpointer data, guint action, GtkWidget *widget )
{
  dasher_set_parameter_bool( BOOL_KEYBOARDCONTROL, GTK_CHECK_MENU_ITEM(widget)->active );
}

void windowpause(gpointer data, guint action, GtkWidget *widget )
{
  dasher_set_parameter_bool( BOOL_WINDOWPAUSE, GTK_CHECK_MENU_ITEM(widget)->active );
}

void controlmode(gpointer data, guint action, GtkWidget *widget )
{
  controlmodeon=GTK_CHECK_MENU_ITEM(widget)->active;
  dasher_set_parameter_bool( BOOL_CONTROLMODE, GTK_CHECK_MENU_ITEM(widget)->active );
}

void DrawMouse(gpointer data, guint action, GtkWidget *widget )
{
  // FIXME - rewrite this sanely, ie:
  // dasher_set_parameter_bool( BOOL_DRAWMOUSE, GTK_CHECK_MENU_ITEM(widget)->active  );
  // plus the same for the above routines

  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    //    interface->DrawMouse( TRUE );
    dasher_set_parameter_bool( BOOL_DRAWMOUSE, true );
  } else {
    //    interface->DrawMouse( FALSE );
    dasher_set_parameter_bool( BOOL_DRAWMOUSE, false );
  }

  dasher_redraw();
}

void about_dasher(gpointer data, guint action, GtkWidget *widget )
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

void get_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(dasherfontdialog);
  if (font_name) {
    dasher_set_parameter_string( STRING_DASHERFONT, font_name );
    set_canvas_font(font_name);
  }
  gtk_widget_destroy (GTK_WIDGET(dasherfontdialog));
}

void set_dasher_font(gpointer data, guint action, GtkWidget *widget)
{
  dasherfontdialog = GTK_FONT_SELECTION_DIALOG(gtk_font_selection_dialog_new("Choose Dasher Font"));
  g_signal_connect (dasherfontdialog->cancel_button, "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT (dasherfontdialog));
  g_signal_connect (dasherfontdialog->ok_button, "clicked", G_CALLBACK (get_font_from_dialog), (gpointer) dasherfontdialog);
  gtk_widget_show(GTK_WIDGET(dasherfontdialog));
}

void get_edit_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(editfontdialog);
  if (font_name) {
    dasher_set_parameter_string( STRING_EDITFONT, font_name );
    set_editbox_font(font_name);
  }
  gtk_widget_destroy (GTK_WIDGET(editfontdialog));
}

void set_edit_font(gpointer data, guint action, GtkWidget *widget)
{
  editfontdialog = GTK_FONT_SELECTION_DIALOG(gtk_font_selection_dialog_new("Choose Dasher Font"));
  g_signal_connect (editfontdialog->cancel_button, "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT (editfontdialog));
  g_signal_connect (editfontdialog->ok_button, "clicked", G_CALLBACK (get_edit_font_from_dialog), (gpointer) editfontdialog);
  gtk_widget_show(GTK_WIDGET(editfontdialog));
}

void reset_fonts(gpointer data, guint action, GtkWidget *widget )
{
  reset_edit_font();
  reset_dasher_font();
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
      gtk_adjustment_set_value(GTK_ADJUSTMENT(speed_slider),  value);
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
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Orientation/Alphabet Default")), TRUE);
	  break;
	case Opts::LeftToRight:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Orientation/Left to Right")), TRUE);
	  break;
	case Opts::RightToLeft:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Orientation/Right to Left")), TRUE);
	  break;
	case Opts::TopToBottom:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Orientation/Top to Bottom")), TRUE);
	  break;
	case Opts::BottomToTop:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Orientation/Bottom to Top")), TRUE);
	  break;
	}
      break;
    case INT_ENCODING:
      fileencoding=value;
      switch(value)
	{
	case Opts::UserDefault:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/File Encoding/User Default")), TRUE);
	  break;
	case Opts::AlphabetDefault:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/File Encoding/Alphabet Default")), TRUE);
	  break;
	case Opts::UTF8:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/File Encoding/Unicode UTF8")), TRUE);
	  break;
	case Opts::UTF16LE:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/File Encoding/Unicode UTF16 (LE)")), TRUE);
	  break;
	case Opts::UTF16BE:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/File Encoding/Unicode UTF16 (BE)")), TRUE);
	  break;
	}
      break;
    case INT_DASHERFONTSIZE:
      switch(value)
	{
	case Opts::Normal:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Font Size/Default Fonts")), TRUE);
	  break;
	case Opts::Big:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Font Size/Large Fonts")), TRUE);
	  break;
	case Opts::VBig:
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Font Size/Very Large Fonts")), TRUE);
	  break;
	}
      break;
    }
}

void parameter_bool_callback( bool_param p, bool value )
{
  switch(p)
    {
    case BOOL_SHOWTOOLBAR:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Show Toolbar")), value);
      
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
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Speed Slider")), value);

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
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Draw Position")), value);
      break;
    case BOOL_DIMENSIONS:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/One Dimensional")), value);
      break;
    case BOOL_TIMESTAMPNEWFILES:
      timestamp=value;
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Timestamp New Files")), value);
      break;
    case BOOL_COPYALLONSTOP:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Copy All on Stop")), value);
      break;
    case BOOL_STARTONLEFT:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Start on Left Mouse")), value);
      startleft=value;
      break;
    case BOOL_STARTONSPACE:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Start on Space Bar")), value);
      startspace=value;
      break;
    case BOOL_KEYBOARDCONTROL:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Keyboard Control")), value);
      keyboardcontrol=value;
      break;
    case BOOL_WINDOWPAUSE:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Pause outside Window")), value);
      leavewindowpause=value;
      break;
    case BOOL_CONTROLMODE:
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/Options/Control Mode")), value);
      break;
    }
}

void null_log_handler (const gchar *log_domain, GLogLevelFlags log_level, 
		       const gchar *message, gpointer unused_data) {}
