#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <iostream>
#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "Gtk2DasherCanvas.h"
#include "Gtk2DasherPane.h"
#include "Gtk2DasherEdit.h"
#include "Gtk2DoubleBuffer.h"
#include "DasherSettingsInterface.h"
#include "Gtk2DasherStore.h"

#include "libdasher.h"

GtkWidget *vbox, *toolbar;
GdkPixbuf *p;
GtkWidget *pw;
Gtk2DasherEdit *dasher_text_view;
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
Gtk2DasherCanvas *dasher_canvas;
Gtk2DasherPane *dasher_pane;
//CDasherInterface *interface;
GtkItemFactory *dasher_menu;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;

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
static void select_open_file(gpointer data, guint action, GtkWidget *widget);
static void select_new_file(gpointer data, guint action, GtkWidget *widget);
static void select_save_file_as();
static void select_append_file();
static void save_file();
static void select_import_file();


void parameter_string_callback( string_param p, const char *value );
void parameter_double_callback( double_param p, double value );
void parameter_int_callback( int_param p, long int value );
void parameter_bool_callback( bool_param p, bool value );

void blank_callback();
void display_callback();

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme);
void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number);
void draw_text_callback(symbol Character, int x1, int y1, int size);
void text_size_callback(symbol Character, int* Width, int* Height, int Size);

void edit_output_callback(symbol Symbol);
void edit_flush_callback(symbol Symbol);
void edit_unflush_callback();

void clipboard_callback( clipboard_action act );

typedef struct {
  Gtk2DasherCanvas *dasher_canvas;
  Gtk2DasherEdit *dasher_text;
} Gtk2DasherComponents;

static GtkItemFactoryEntry entries[] = {
  { "/_File",         NULL,      NULL,         0, "<Branch>" },
  { "/File/_New",     "<CTRL>N", *GtkItemFactoryCallback(select_new_file),     1, "<Item>" },
  { "/File/_Open...", "<CTRL>O", *GtkItemFactoryCallback(select_open_file),    1, "<Item>" },
  { "/File/Save",     NULL, save_file, 0, "<Item>" },
  { "/File/Save As...", NULL, select_save_file_as, 0, "<Item>" },
  { "/File/Append to File...", NULL, select_append_file, 0, "<Item>" },
  { "/File/sep1",     NULL,      NULL,         0, "<Separator>" },
  { "/File/Import Training Text...", NULL, select_import_file, 0, "<Item>" },
  { "/File/sepl", NULL, NULL, 0, "<Separator>" },
  { "/File/_Quit",    "<CTRL>Q", NULL, 0, "<StockItem>", GTK_STOCK_QUIT },
  { "/Edit", NULL, NULL, 0, "<Branch>" },
  { "/Edit/Cut", NULL, clipboard_cut, 0, "<Item>" },
  { "/Edit/Copy", NULL, clipboard_copy, 0, "<Item>" },
  { "/Edit/Paste", NULL, clipboard_paste, 0, "<Item>" },
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
  { "/View/sepl", NULL, NULL, 0, "<Separator>" },
  { "/View/Fix Layout", NULL, NULL, 0, "<CheckItem>" },
  { "/Options", NULL, NULL, 0, "<Branch>" },
  { "/Options/Timestamp New Files", NULL, *GtkItemFactoryCallback(timestamp_files), 1, "<CheckItem>" },
  { "/Options/Copy All on Stop", NULL, *GtkItemFactoryCallback(copy_all_on_stop), 1, "<CheckItem>" },
  { "/Options/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Options/Alphabet...", NULL, *GtkItemFactoryCallback(preferences), 1, "<Item>" },
  { "/Options/File Encoding", NULL, NULL, 0, "<Branch>" },
  { "/Options/File Encoding/User Default", NULL, *GtkItemFactoryCallback(file_encoding), 1, "<RadioItem>" },
  { "/Options/File Encoding/Alphabet Default", NULL, *GtkItemFactoryCallback(file_encoding), 2, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF8", NULL, *GtkItemFactoryCallback(file_encoding), 65004, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF16 (LE)", NULL, *GtkItemFactoryCallback(file_encoding), 1203, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF16 (BE)", NULL, *GtkItemFactoryCallback(file_encoding), 1204, "/Options/File Encoding/User Default" },
  { "/Options/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Options/Font Size", NULL, NULL, 0, "<Branch>" },
  { "/Options/Font Size/Default Fonts", NULL, NULL, 0, "<RadioItem>" },
  { "/Options/Font Size/Large Fonts", NULL, NULL, 0, "/Options/Font Size/Default Fonts" },
  { "/Options/Font Size/Very Large Fonts", NULL, NULL, 0, "/Options/Font Size/Default Fonts" },
  { "/Options/Editing Font...", NULL, NULL, 0, "<Item>" },
  { "/Options/Dasher Font...", NULL, NULL, 0, "<Item>" },
  { "/Options/Reset Fonts", NULL, NULL, 0, "<Item>" },
  { "/Options/One Dimensional", NULL, NULL, 0, "<CheckItem>" },
  { "/Options/Draw Position", NULL, *GtkItemFactoryCallback(DrawMouse), 1, "<CheckItem>" },
  { "/Help", NULL, NULL, 0, "<Branch>" },
  { "/Help/About", NULL, NULL, 0, "<Item>" }
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

guint window_x = 500, window_y = 400;

gboolean setup = FALSE;
gboolean paused = TRUE;
gboolean indrag = FALSE;
gboolean file_modified = FALSE;

gint prev_pos_x;
gint prev_pos_y;

const gchar *filename = NULL;

GtkWidget *window;
GtkWidget *file_selector;

static void 
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

    //    interface->ChangeAlphabet(alphabet);
    dasher_set_parameter_string( STRING_ALPHABET, alph );

    g_free(alph);
  }
}

static void 
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

  //  std::vector< std::string > alphabetlist;
  //  interface->GetAlphabets( &alphabetlist );

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
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
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
  
static void 
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
  
  dasher_text_view->Clear();
  
  file_modified = 1;

  gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER (dasher_text_view->text_buffer), buffer, file_stat.st_size);

  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW (dasher_text_view->text_view),gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(dasher_text_view->text_buffer)));

  file_modified = 0;
  
  dasher_text_view->flush_count = 0;

  dasher_start();
  dasher_redraw();

  //  dasher_text_view->interface->Start();
  //  dasher_text_view->interface->Redraw();
}

static void
open_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  //  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);

  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  open_file (filename);

  gtk_widget_destroy (GTK_WIDGET(selector));
}


static void
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

static void
new_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  gtk_widget_destroy (GTK_WIDGET(selector));
}


static void
select_new_file(gpointer data, guint action, GtkWidget *widget)
{

  GtkWidget *filew;

  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		    "clicked", G_CALLBACK (new_file_from_filesel), (gpointer) filew);
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			    "clicked", G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (filew));
  gtk_widget_show (filew);
}

static void 
save_file_as (const char *filename, bool append)
{
  FILE *fp;
  gint length;
  gchar *buffer;

  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  if (append == true) {
    fp = fopen (filename, "a");
  } else {
    fp = fopen (filename, "w");
  }

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(dasher_text_view->text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(dasher_text_view->text_buffer),end,-1);

  length = gtk_text_iter_get_offset(end);

  buffer = gtk_text_iter_get_slice (start,end);

  fwrite(buffer,1,length,fp);
  fclose (fp);

  file_modified = 0;
  
}

static void
save_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,FALSE);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

static void
append_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  save_file_as(filename,TRUE);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

static void
select_save_file_as()
{

  GtkWidget *filew;

  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		    "clicked", G_CALLBACK (save_file_from_filesel), (gpointer) filew);
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
			    "clicked", G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (filew));
  gtk_widget_show (filew);
}

static void
import_file_from_filesel ( GtkWidget *selector2, GtkFileSelection *selector )
{
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(selector));

  load_training_file(filename);

  gtk_widget_destroy (GTK_WIDGET(selector));
}

static void
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

static void
save_file ()
{
  if (filename != NULL) {
    save_file_as(filename,FALSE);
  }
  else {
    select_save_file_as();
  }
}

static void
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


static void 
toolbar_save(GtkWidget *widget, gpointer data)
{
}

static void
ask_save_before_exit(GtkWidget *widget, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  
  if (file_modified == TRUE) {
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

static void 
toolbar_cut(GtkWidget *widget, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  gtk_editable_cut_clipboard(GTK_EDITABLE(dasher_text_view->text_view));

  return;
}

static void 
toolbar_copy(GtkWidget *widget, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  gtk_editable_copy_clipboard(GTK_EDITABLE(dasher_text_view->text_view));

  return;
}

static void 
toolbar_paste(GtkWidget *widget, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  gtk_editable_paste_clipboard(GTK_EDITABLE(dasher_text_view->text_view));

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

static gint
timer_callback(gpointer data)
{
  Gtk2DasherCanvas *dasher_canvas = static_cast<Gtk2DasherCanvas*>(data);
  
  if (!paused) {
    int x;
    int y;
    
    gdk_window_get_pointer(dasher_canvas->canvas->window, &x, &y, NULL);
    //    dasher_canvas->interface->TapOn(x,y,get_time());

    dasher_tap_on( x, y, get_time() );
  }

  // need non-zero return value so timer repeats

  return 1;
}

static gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  Gtk2DasherCanvas *dasher_canvas = static_cast<Gtk2DasherCanvas*>(data);

  gdk_draw_pixmap(dasher_canvas->canvas->window,
		  dasher_canvas->canvas->style->fg_gc[GTK_WIDGET_STATE (dasher_canvas->canvas)],
		  dasher_canvas->buffer->get_fg(),
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  return TRUE;
}

static gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  Gtk2DasherCanvas *dasher_canvas = static_cast<Gtk2DasherCanvas*>(data);

  dasher_canvas->CreateNewBuffer();  

  dasher_resize_canvas( dasher_canvas->canvas->allocation.width, dasher_canvas->canvas->allocation.height );
  dasher_redraw();

  return FALSE;
}

static void
edit_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  //dasher_text_view->get_new_context();

  dasher_text_view->flush_count = 0;

  //  dasher_text_view->interface->Start();  
  //  dasher_text_view->interface->Redraw();  

  dasher_start();
  dasher_redraw();
}

static void
edit_key_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);

  dasher_text_view->flush_count = 0;

  //  dasher_text_view->interface->Start();  
  //  dasher_text_view->interface->Redraw();  

  dasher_start();
  dasher_redraw();
}

static void
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  Gtk2DasherPane *dasher_pane = static_cast<Gtk2DasherPane*>(data);
  Gtk2DasherCanvas *dasher_canvas = dasher_pane->canvas;
  Gtk2DasherEdit *dasher_text_view = dasher_pane->edit;

  GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
  gboolean *returnType;

  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_2BUTTON_PRESS))
    return;

  focusEvent->type = GDK_FOCUS_CHANGE;
  focusEvent->window = (GdkWindow *) dasher_canvas->canvas;
  focusEvent->send_event = FALSE;
  focusEvent->in = TRUE;

  gtk_widget_grab_focus(GTK_WIDGET(dasher_canvas->canvas));  
  gtk_signal_emit_by_name(GTK_OBJECT(dasher_canvas->canvas), "focus_in_event", GTK_WIDGET(dasher_canvas->canvas), focusEvent, NULL, &returnType);

  g_free(focusEvent);

  if (paused == TRUE) {
    //    dasher_canvas->interface->Unpause(get_time());
    dasher_unpause( get_time() );
    paused = FALSE;
  } else {
    //    dasher_canvas->interface->PauseAt((gint) event->x,(gint) event->y);
    dasher_pause( (gint) event->x,(gint) event->y );
    paused = TRUE;
  }
  //  gint start = GTK_EDITABLE(dasher_text_view->text_view)->selection_start_pos;
  //  gint end = GTK_EDITABLE(dasher_text_view->text_view)->selection_end_pos;

  //  if (end-start != 0)
  //    gtk_editable_delete_selection(GTK_EDITABLE(dasher_text_view->text_view));

  return;
}

static gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  Gtk2DasherCanvas *dasher_canvas = static_cast<Gtk2DasherCanvas*>(data);

  //  dasher_canvas->interface->PauseAt((gint) event->x,(gint) event->y);

  dasher_pause( (gint) event->x,(gint) event->y );
  paused = TRUE;

  return FALSE;
}

static void speed_changed(GtkAdjustment *adj, Gtk2DasherCanvas *dasher_canvas) {

  dasher_set_parameter_double( DOUBLE_MAXBITRATE, adj->value );
}


static void
open_window() {
  //     interface = new CDasherInterface;    
  //   GtkDasherUI *dasherui = new GtkDasherUI(interface);  

  char *system_data_dir;
    char *home_dir;
    char *user_data_dir;

    home_dir = getenv( "HOME" );
    user_data_dir = new char[ strlen( home_dir ) + 10 ];
    sprintf( user_data_dir, "%s/.dasher/", home_dir );
    
    // CHANGE THIS!
    //system_data_dir = "/etc/dasher/";
    system_data_dir = user_data_dir;
    
    //    interface->SetSystemLocation(system_data_dir);
    //    interface->SetSystemLocation(user_data_dir);

    dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
    dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

    dasher_accel = gtk_accel_group_new();
    
    dasher_menu= gtk_item_factory_new( GTK_TYPE_MENU_BAR,
				       "<DasherMenu>",
				       dasher_accel);

     dasher_text_view = new Gtk2DasherEdit ();
    
    gtk_item_factory_create_items( dasher_menu,
				   54,
				   entries,
				   &dasher_text_view );
    
    float initial_bitrate = 3.0;
    

    dasher_canvas = new Gtk2DasherCanvas (360, 360);
    dasher_pane = new Gtk2DasherPane (dasher_canvas, dasher_text_view);
    
    ofilesel = gtk_file_selection_new("Open a file");
    afilesel = gtk_file_selection_new("Append to file");
    ifilesel = gtk_file_selection_new("Import Training Text");
    
    home_dir = getenv( "HOME" );
    user_data_dir = new char[ strlen( home_dir ) + 10 ];
    sprintf( user_data_dir, "%s/.dasher/", home_dir );
    
    // CHANGE THIS!
    //system_data_dir = "/etc/dasher/";
    system_data_dir = user_data_dir;
    
    //    interface->SetSystemLocation(system_data_dir);
    // interface->SetSystemLocation(user_data_dir);

    
    dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
    dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

    vbox = gtk_vbox_new (FALSE, 2);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(window), _("Dasher"));
    gtk_window_set_policy (GTK_WINDOW(window), FALSE, FALSE, FALSE);
    
    gtk_window_add_accel_group( GTK_WINDOW(window), dasher_accel);
    dasher_menu_bar=gtk_item_factory_get_widget( dasher_menu, "<DasherMenu>");
    
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

    gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (ask_save_before_exit), dasher_text_view);
    
    gtk_box_pack_start (GTK_BOX (vbox), dasher_menu_bar, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
    
    text_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (text_scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), text_scrolled_window, FALSE, FALSE, 0);
    
    gtk_container_add (GTK_CONTAINER (text_scrolled_window), dasher_text_view->text_view);
    
    GTK_WIDGET_SET_FLAGS (GTK_WIDGET (dasher_canvas->canvas), GTK_CAN_FOCUS);
    gtk_window_set_focus(GTK_WINDOW(window), GTK_WIDGET(dasher_canvas->canvas));
    
    gtk_signal_connect(GTK_OBJECT (dasher_canvas->canvas), "expose_event", GTK_SIGNAL_FUNC (canvas_expose_event), (gpointer) dasher_canvas);
    
    gtk_signal_connect(GTK_OBJECT (dasher_canvas->canvas), "configure_event", GTK_SIGNAL_FUNC (canvas_configure_event), (gpointer) dasher_canvas);
    
    gtk_signal_connect(GTK_OBJECT (dasher_canvas->canvas), "button_press_event", GTK_SIGNAL_FUNC (button_press_event), (gpointer) dasher_pane);
    
    //gtk_widget_set_events(dasher_canvas->canvas, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
    
    gtk_widget_set_events(dasher_canvas->canvas, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );
    
    gtk_signal_connect(GTK_OBJECT (dasher_text_view->text_view), "button_release_event", GTK_SIGNAL_FUNC (edit_button_release_event), (gpointer) dasher_text_view);
    
    gtk_signal_connect(GTK_OBJECT (dasher_text_view->text_view), "key_release_event", GTK_SIGNAL_FUNC (edit_key_release_event), (gpointer) dasher_text_view);
    
    gtk_widget_set_events(dasher_text_view->text_view, GDK_BUTTON_RELEASE_MASK | GDK_KEY_RELEASE_MASK);

    canvas_frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (vbox), canvas_frame, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (canvas_frame), dasher_canvas->canvas);
    
    speed_frame = gtk_frame_new ("Speed");
    speed_slider = gtk_adjustment_new(initial_bitrate, 1.0, 8.0, 1.0, 1.0, 0.0);
    speed_hscale = gtk_hscale_new(GTK_ADJUSTMENT(speed_slider));
    gtk_range_set_update_policy(GTK_RANGE(speed_hscale), GTK_UPDATE_CONTINUOUS);
    
    gtk_signal_connect (GTK_OBJECT (speed_slider), "value_changed", GTK_SIGNAL_FUNC (speed_changed), dasher_canvas);
    
    gtk_box_pack_start (GTK_BOX (vbox), speed_frame, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (speed_frame), speed_hscale);
    
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_set_usize (GTK_WIDGET (window), window_x, window_y);
    
    gtk_widget_realize (window);
    gtk_widget_realize (vbox);
    gtk_widget_realize (text_scrolled_window);
    gtk_widget_realize (dasher_text_view->text_view);
    gtk_widget_realize (toolbar);
    gtk_widget_realize (canvas_frame);
    gtk_widget_realize (dasher_canvas->canvas);
    gtk_widget_realize (speed_frame);
    gtk_widget_realize (speed_hscale);
    gtk_widget_realize (dasher_menu_bar);
    
    gtk_widget_show (window);
    gtk_widget_show (vbox);
    gtk_widget_show (text_scrolled_window);
    gtk_widget_show (dasher_text_view->text_view);
    gtk_widget_show (toolbar);
    gtk_widget_show (canvas_frame);
    gtk_widget_show (dasher_canvas->canvas);
    gtk_widget_show (speed_frame);
    gtk_widget_show (speed_hscale);
    gtk_widget_show (dasher_menu_bar);

    // FIXME - need to implement this

    //    interface->SettingsDefaults( store );


    //    interface->ChangeLanguageModel(0);
    //interface->ChangeView(0);

    dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
    dasher_set_parameter_int( INT_VIEW, 0 );
    
    //    std::vector< std::string > alphabetlist;
    // interface->GetAlphabets( &alphabetlist );

    const char *alphabet;

    dasher_get_alphabets( &alphabet, 1 );

    dasher_set_parameter_string( STRING_ALPHABET, alphabet );

    // FIXME - need to implement screen and edit classes in the library
    
    //    interface->ChangeEdit(dasher_text_view);
    // interface->ChangeScreen(dasher_canvas->wrapper);


    //    interface->ChangeMaxBitRate(initial_bitrate);
    
    dasher_set_parameter_double( DOUBLE_MAXBITRATE, initial_bitrate );
    
    //    interface->Start();

    dasher_start();

    gtk_timeout_add(50, timer_callback, dasher_canvas);  
    
    setup = TRUE;
}

int
main(int argc, char *argv[])
{
  gtk_init (&argc, &argv);

  dasher_initialise( 360, 360 );

  dasher_set_string_callback( parameter_string_callback );
  dasher_set_double_callback( parameter_double_callback );
  dasher_set_int_callback( parameter_int_callback );
  dasher_set_bool_callback( parameter_bool_callback );

  dasher_set_blank_callback( blank_callback );
  dasher_set_display_callback( display_callback );
  dasher_set_draw_rectangle_callback( draw_rectangle_callback );
  dasher_set_draw_polyline_callback( draw_polyline_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_text_size_callback( text_size_callback );
  
  dasher_set_edit_output_callback( edit_output_callback );
  dasher_set_edit_flush_callback( edit_flush_callback );
  dasher_set_edit_unflush_callback( edit_unflush_callback );

  dasher_set_clipboard_callback( clipboard_callback );

  setlocale (LC_ALL, "");

  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);

  open_window ();



  gtk_main ();

  dasher_finalise();

  return 0;
}

void clipboard_copy(void) {
  //  dasher_text_view->Copy();
  dasher_copy();
}

void clipboard_cut(void) {
  //  dasher_text_view->Cut();
  dasher_cut();
}

void clipboard_paste(void) {
  //  dasher_text_view->Paste();
  dasher_paste();
}

void clipboard_copy_all(void) {
  // dasher_text_view->CopyAll();
  dasher_copy_all();
}

void clipboard_select_all(void) {
  //  dasher_text_view->SelectAll();
  dasher_select_all();
}

void clipboard_callback( clipboard_action act )
{
  switch( act )
    {
    case CLIPBOARD_CUT:
      dasher_text_view->Cut();
      break;
    case CLIPBOARD_COPY:
      dasher_text_view->Copy();
      break;
    case CLIPBOARD_PASTE:
      dasher_text_view->Paste();
      break;
    case CLIPBOARD_COPYALL:
      dasher_text_view->CopyAll();
      break;
    case CLIPBOARD_SELECTALL:
      dasher_text_view->SelectAll();
      break;
    }
}

void orientation(gpointer data, guint action, GtkWidget  *widget )
{
  signed int RealAction=action-3;

  // FIXME - need to implement this

  //  interface->ChangeOrientation(Dasher::Opts::ScreenOrientations(RealAction));
  // interface->Redraw();
}

void show_toolbar(gpointer data, guint action, GtkWidget  *widget )
{

  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    //    interface->ShowToolbar( TRUE );
    dasher_set_parameter_bool( BOOL_SHOWTOOLBAR, true );
  } else {
    //    interface->ShowToolbar( FALSE );
    dasher_set_parameter_bool( BOOL_SHOWTOOLBAR, false );
  }
}

void show_slider(gpointer data, guint action, GtkWidget  *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    //    interface->ShowSpeedSlider( TRUE );
 dasher_set_parameter_bool( BOOL_SHOWSPEEDSLIDER, true );
  } else {
    //interface->ShowSpeedSlider( FALSE );
    dasher_set_parameter_bool( BOOL_SHOWSPEEDSLIDER, false );
  }
}

void timestamp_files(gpointer data, guint action, GtkWidget *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    //    interface->TimeStampNewFiles( TRUE );
    dasher_set_parameter_bool( BOOL_TIMESTAMPNEWFILES, true );
  } else {
    //    interface->TimeStampNewFiles( FALSE );
    dasher_set_parameter_bool( BOOL_TIMESTAMPNEWFILES, false );
  }
}

void copy_all_on_stop(gpointer data, guint action, GtkWidget *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    //    interface->CopyAllOnStop( TRUE );
    dasher_set_parameter_bool( BOOL_COPYALLONSTOP, true );
  } else {
    //    interface->CopyAllOnStop( FALSE );
    dasher_set_parameter_bool( BOOL_COPYALLONSTOP, false );
  }
}

void file_encoding(gpointer data, guint action, GtkWidget *widget )
{
  signed int realaction = action -3;
  //  interface->SetFileEncoding(Opts::FileEncodingFormats(realaction));
  //FIXME - need to reimplemnt this
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
}

// Callbacks to be notified of when something changes

void parameter_string_callback( string_param p, const char *value )
{
}

void parameter_double_callback( double_param p, double value )
{
}

void parameter_int_callback( int_param p, long int value )
{
}

void parameter_bool_callback( bool_param p, bool value )
{
  cout << "In bool callback" << endl;
  switch(p)
    {
    case BOOL_SHOWTOOLBAR:

      if (toolbar==NULL) 
	break;

      if (value) {
	gtk_widget_show(toolbar);
      } else {
	gtk_widget_hide(toolbar);
      }

      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Show Toolbar")), value);
      break;
    case BOOL_SHOWSPEEDSLIDER:
      if (speed_frame==NULL) 
	break;

      if (value) {
	gtk_widget_show(speed_frame);
      } else {
	gtk_widget_hide(speed_frame);
      }
    
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item (dasher_menu, "/View/Speed Slider")), value);
    }
}

void blank_callback()
{
  gdk_draw_rectangle (dasher_canvas->buffer->get_bg(),
		      
		      dasher_canvas->canvas->style->white_gc,
                      TRUE,
                      0, 0,
		      dasher_canvas->canvas->allocation.width,
		      dasher_canvas->canvas->allocation.height);

}

void display_callback()
{ 
  GdkRectangle update_rect;
  dasher_canvas->buffer->swap_buffers();

  gdk_draw_rectangle ( dasher_canvas->buffer->get_bg(),
		       dasher_canvas->canvas->style->white_gc,
                      TRUE,
                      0, 0,
		      dasher_canvas->canvas->allocation.width,
		      dasher_canvas->canvas->allocation.height);
  
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = dasher_canvas->canvas->allocation.width;
  update_rect.height = dasher_canvas->canvas->allocation.height;

  gtk_widget_draw(dasher_canvas->canvas,&update_rect);
}

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;

  GdkColor black = {0, 0, 0, 0};
  GdkColor new_foreground = { 0, 45000, 45000, 45000 };
  GdkColor foreground = {0, ((ColorScheme * 3 + Color) & 1) * 30000 + 30000, 
			 ((ColorScheme * 3 + Color) >> 1 ) * 30000 + 30000, 
			 ((ColorScheme * 3 + Color) >> 2 ) * 30000 + 30000 };
  
  graphics_context = dasher_canvas->canvas->style->fg_gc[GTK_WIDGET_STATE (dasher_canvas->canvas)];
  colormap = gdk_colormap_get_system();

  foreground = dasher_canvas->get_color(Color, ColorScheme);

  gdk_color_alloc(colormap, &foreground);
  gdk_gc_set_foreground (graphics_context, &foreground);
  
  if( x2 > x1 ) {
    if( y2 > y1 ) {
      gdk_draw_rectangle (dasher_canvas->buffer->get_bg(), graphics_context, TRUE, x1, y1, x2-x1, y2-y1);
    }
    else {
      gdk_draw_rectangle (dasher_canvas->buffer->get_bg(), graphics_context, TRUE, x1, y2, x2-x1, y1-y2);
    }
  }
  else {
    if( y2 > y1 ) {
      gdk_draw_rectangle (dasher_canvas->buffer->get_bg(), graphics_context, TRUE, x2, y1, x1-x2, y2-y1);
    }
    else {
      gdk_draw_rectangle (dasher_canvas->buffer->get_bg(), graphics_context, TRUE, x2, y2, x1-x2, y1-y2);
    }
  }

  gdk_gc_set_foreground (graphics_context, &black);
}

void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number)
{ 
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;

  GdkColor black = {0, 0, 0, 0};
  GdkPoint gdk_points[Number];

  graphics_context = dasher_canvas->canvas->style->fg_gc[GTK_WIDGET_STATE (dasher_canvas->canvas)];
  colormap = gdk_colormap_get_system();

  gdk_color_alloc(colormap, &black);
  gdk_gc_set_foreground (graphics_context, &black);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(dasher_canvas->buffer->get_bg(), graphics_context, gdk_points, Number);
}

void draw_text_callback(symbol Character, int x1, int y1, int size)
{
  std::string symbol;
  GdkRectangle update_rect;
  GdkFont *chosen_font;  
  PangoRectangle *ink,*logical;

  ink = new PangoRectangle;
  logical = new PangoRectangle;

  //  symbol = interface->GetDisplayText(Character);
  symbol = dasher_get_display_text( Character );
  
  chosen_font = dasher_canvas->GetFont(size);

  pango_layout_set_text(dasher_canvas->pangolayout,symbol.c_str(),-1);

  pango_layout_get_pixel_extents(dasher_canvas->pangolayout,ink,logical);

  gdk_draw_layout (dasher_canvas->buffer->get_bg(),
		   dasher_canvas->canvas->style->black_gc,
		   x1, y1-(ink->height/2.0), dasher_canvas->pangolayout);
}

void text_size_callback(symbol Character, int* Width, int* Height, int Size)
{
  GdkFont *chosen_font;

  chosen_font = dasher_canvas->GetFont(Size);
    
  *Width = gdk_char_height(chosen_font, ('A'));
  *Height = gdk_char_height(chosen_font, ('A'));
}

void edit_output_callback(symbol Symbol)
{
  std::string label;
  label = dasher_get_edit_text( Symbol );

  gtk_text_buffer_insert_at_cursor(dasher_text_view->text_buffer, label.c_str(), -1);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(dasher_text_view->text_view),gtk_text_buffer_get_insert(dasher_text_view->text_buffer));
}

void edit_flush_callback(symbol Symbol)
{
  std::string label;
  label = dasher_get_edit_text( Symbol );

  gtk_text_buffer_insert_at_cursor(dasher_text_view->text_buffer, label.c_str(), -1);

  if (label!="") {
    dasher_text_view->flush_count++;
  }

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(dasher_text_view->text_view),gtk_text_buffer_get_insert(dasher_text_view->text_buffer));
}

void edit_unflush_callback()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(dasher_text_view->text_buffer,end,gtk_text_buffer_get_insert(dasher_text_view->text_buffer));

  *start=*end;  

  gtk_text_iter_backward_chars(start, dasher_text_view->flush_count);

  gtk_text_buffer_delete(dasher_text_view->text_buffer,start,end);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(dasher_text_view->text_view),gtk_text_buffer_get_insert(dasher_text_view->text_buffer));
  dasher_text_view->flush_count=0;

}
