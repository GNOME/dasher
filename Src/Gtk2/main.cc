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
CDasherInterface *interface;
GtkItemFactory *dasher_menu;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;

void clipboard_copy(void);
void clipboard_cut(void);
void clipboard_paste(void);
void load_training_file_from_filesel ();
static void orientation(gpointer data, guint action, GtkWidget  *widget );
static void show_toolbar(gpointer data, guint action, GtkWidget  *widget );
static void show_slider(gpointer data, guint action, GtkWidget  *widget );

typedef struct {
  Gtk2DasherCanvas *dasher_canvas;
  Gtk2DasherEdit *dasher_text;
} Gtk2DasherComponents;

static GtkItemFactoryEntry entries[] = {
  { "/_File",         NULL,      NULL,         0, "<Branch>" },
  { "/File/_New",     "<CTRL>N", NULL,     0, "<Item>" },
  { "/File/_Open...", "<CTRL>O", NULL,    0, "<Item>" },
  { "/File/Save",     NULL, NULL, 0, "<Item>" },
  { "/File/Save As...", NULL, NULL, 0, "<Item>" },
  { "/File/Append to File...", NULL, NULL, 0, "<Item>" },
  { "/File/sep1",     NULL,      NULL,         0, "<Separator>" },
  { "/File/Import Training Text...", NULL, NULL, 0, "<Item>" },
  { "/File/sepl", NULL, NULL, 0, "<Separator>" },
  { "/File/_Quit",    "<CTRL>Q", NULL, 0, "<StockItem>", GTK_STOCK_QUIT },
  { "/Edit", NULL, NULL, 0, "<Branch>" },
  { "/Edit/Cut", NULL, clipboard_cut, 0, "<Item>" },
  { "/Edit/Copy", NULL, clipboard_copy, 0, "<Item>" },
  { "/Edit/Paste", NULL, clipboard_paste, 0, "<Item>" },
  { "/Edit/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Edit/Copy All", NULL, NULL, 0, "<Item>" },
  { "/Edit/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Edit/Select All", NULL, NULL, 0, "<Item>" },
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
  { "/Options/Timestamp New Files", NULL, NULL, 0, "<CheckItem>" },
  { "/Options/Copy All on Stop", NULL, NULL, 0, "<CheckItem>" },
  { "/Options/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Options/Alphabet...", NULL, NULL, 0, "<Item>" },
  { "/Options/File Encoding", NULL, NULL, 0, "<Branch>" },
  { "/Options/File Encoding/User Default", NULL, NULL, 0, "<RadioItem>" },
  { "/Options/File Encoding/Alphabet Default", NULL, NULL, 0, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF8", NULL, NULL, 0, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF16 (LE)", NULL, NULL, 0, "/Options/File Encoding/User Default" },
  { "/Options/File Encoding/Unicode UTF16 (BE)", NULL, NULL, 0, "/Options/File Encoding/User Default" },
  { "/Options/sepl", NULL, NULL, 0, "<Separator>" },
  { "/Options/Font Size", NULL, NULL, 0, "<Branch>" },
  { "/Options/Font Size/Default Fonts", NULL, NULL, 0, "<RadioItem>" },
  { "/Options/Font Size/Large Fonts", NULL, NULL, 0, "/Options/Font Size/Default Fonts" },
  { "/Options/Font Size/Very Large Fonts", NULL, NULL, 0, "/Options/Font Size/Default Fonts" },
  { "/Options/Editing Font...", NULL, NULL, 0, "<Item>" },
  { "/Options/Dasher Font...", NULL, NULL, 0, "<Item>" },
  { "/Options/Reset Fonts", NULL, NULL, 0, "<Item>" },
  { "/Options/One Dimensional", NULL, NULL, 0, "<CheckItem>" },
  { "/Options/Draw Position", NULL, NULL, 0, "<CheckItem>" },
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

gchar *filename = NULL;

GtkWidget *window;
GtkWidget *file_selector;

static void 
load_training_file (const gchar *filename, CDasherInterface *interface)
{
  interface->TrainFile(filename);
}

static void
load_training_file_from_filesel (GtkFileSelection *selector, gpointer data)
{
  load_training_file (gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector)), dasher_canvas->interface);
  
  gtk_widget_destroy (file_selector);
}

static void 
preferences (GtkWidget *widget, gpointer data)
{
  Gtk2DasherCanvas *dasher_canvas = static_cast<Gtk2DasherCanvas*>(data);
  
  GtkWidget *preferences_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (preferences_window), "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

  GtkWidget *training_frame = gtk_frame_new(NULL);
  GtkWidget *alphabet_frame = gtk_frame_new(NULL);
  
  GtkWidget *notebook = gtk_notebook_new();
  GtkWidget *label_training_page = gtk_label_new (_("Language Model"));
  GtkWidget *label_alphabet_page = gtk_label_new (_("Alphabet"));
  
  GtkWidget *vbox_training = gtk_vbox_new (FALSE, 0);
  GtkWidget *vbox_alphabet = gtk_vbox_new (FALSE, 0);
  
  GtkWidget *button_box = gtk_hbox_new (FALSE, 0);
  GtkWidget *button_ok;
  GtkWidget *button_cancel;
  
  GtkWidget *training_box = gtk_vbox_new (FALSE, 2);
  GtkWidget *training_file_box = gtk_hbox_new (FALSE, 0);
  GtkWidget *training_file_location = gtk_entry_new ();
  GtkWidget *training_file_label = gtk_label_new (_("Training file:"));
  GtkWidget *training_file_select;

  GtkWidget *alphabet_box = gtk_vbox_new (FALSE, 2);
  GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL,NULL);
  GtkWidget *alphabet_list = gtk_list_new();    

  gtk_container_add (GTK_CONTAINER (alphabet_frame), alphabet_box);

  gtk_entry_set_editable (GTK_ENTRY(training_file_location), FALSE);
  
  gtk_box_pack_start( GTK_BOX(training_file_box), training_file_label, FALSE, FALSE, 2);
  gtk_box_pack_start( GTK_BOX(training_file_box), training_file_location, TRUE, TRUE, 2);
  
  gtk_box_pack_start (GTK_BOX (training_box), training_file_box, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (training_frame), training_box);
  gtk_box_pack_start (GTK_BOX (vbox_training), training_frame, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox_alphabet), alphabet_frame, TRUE, TRUE, 0);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), alphabet_list);
   
  gtk_box_pack_start (GTK_BOX (alphabet_box), scrolled_window, FALSE, FALSE, 0);

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), TRUE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
  
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox_training, label_training_page);
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox_alphabet, label_alphabet_page);
  
  gtk_container_add (GTK_CONTAINER (preferences_window), notebook);
  gtk_widget_set_usize (GTK_WIDGET (preferences_window), 240, 320);  

  gtk_widget_realize (preferences_window);
  gtk_widget_realize (notebook);

  gtk_widget_show_all (preferences_window);
  gtk_widget_show (notebook);
}
  
static void 
configure (GtkWidget *widget, gpointer data)
{
  Gtk2DasherCanvas *dasher_canvas = static_cast<Gtk2DasherCanvas*>(data);
  
  file_selector = gtk_file_selection_new ("Import Training File ...");
  
  //  gtk_signal_connect (GTK_OBJECT (file_selector), "completed", GTK_SIGNAL_FUNC (load_training_file_from_filesel), (gpointer) dasher_canvas);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (file_selector));

  gtk_widget_show (file_selector);

  return;
}

static void 
toolbar_button_cb(GtkWidget *widget, gpointer data)
{
  return;
}

static void 
toolbar_new(GtkWidget *widget, gpointer data)
{
  Gtk2DasherPane *dasher_pane = static_cast<Gtk2DasherPane*>(data);

  dasher_pane->edit->flush_count = 0;
  dasher_pane->edit->Clear();
  dasher_pane->canvas->interface->Start();
  paused = TRUE;
  dasher_pane->canvas->interface->Redraw();

  return;
}

static void
save_file_as (GtkFileSelection *selector, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  guint text_length;
  FILE *fp;
  gchar *buffer;
  
  //  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
  
  text_length = gtk_text_get_length (GTK_TEXT (dasher_text_view->text_view));
  buffer = (gchar *) g_malloc (text_length);
  buffer = gtk_editable_get_chars (GTK_EDITABLE (dasher_text_view->text_view), 0, text_length);
  
  fwrite (buffer, 1, text_length, fp);
  fclose (fp);
  g_free (buffer);
  
  file_modified = FALSE;
  
  gtk_widget_destroy (file_selector);
}

static void 
open_file (char *filename, Gtk2DasherEdit *dasher_text_view)
{
  struct stat file_stat;
  FILE *fp;
  int pos = 0;
  gchar *buffer;

  stat (filename, &file_stat);
  buffer = (gchar *) g_malloc (file_stat.st_size);
  fread (buffer, file_stat.st_size, 1, fp);
  fclose (fp);
  
  dasher_text_view->Clear();
  
  file_modified = 1;
  gtk_editable_insert_text (GTK_EDITABLE (dasher_text_view->text_view), buffer, file_stat.st_size, &pos);
  
  file_modified = 0;
  
  dasher_text_view->flush_count = 0;
  dasher_text_view->interface->Start();
  dasher_text_view->interface->Redraw();
}

static void
open_file_from_filesel (GtkFileSelection *selector, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  
  //  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
  open_file (filename, dasher_text_view);
  
  gtk_widget_destroy (file_selector);
}


static void
select_open_file(GtkWidget *widget, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  
  file_selector = gtk_file_selection_new ("Open File ...");
  
  gtk_signal_connect (GTK_OBJECT (file_selector), "completed", GTK_SIGNAL_FUNC (open_file_from_filesel), (gpointer) dasher_text_view);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (file_selector));

  gtk_widget_show (file_selector);
}

static void
select_save_file_as (Gtk2DasherEdit *dasher_text_view)
{
  gchar *suggested_filename;
  guint text_length;
  
  text_length = gtk_text_get_length (GTK_TEXT (dasher_text_view->text_view));
  if (text_length > 10)
    text_length = 10;

  suggested_filename = (gchar *) g_malloc (text_length);
  suggested_filename = gtk_editable_get_chars (GTK_EDITABLE (dasher_text_view->text_view), 0, text_length);

  file_selector = gtk_file_selection_new ("Save As ..");
  
  gtk_signal_connect (GTK_OBJECT (file_selector), "completed", GTK_SIGNAL_FUNC (save_file_as), (gpointer) dasher_text_view);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (file_selector));

  //  gtk_entry_set_text (GTK_ENTRY (GTK_FILE_SELECTION (file_selector)->entry), suggested_filename);
  
  gtk_widget_show (file_selector);
  
  g_free (suggested_filename);
  while (GTK_IS_WIDGET(file_selector)) {
    while (gtk_events_pending()) {
      gtk_main_iteration_do(FALSE);
    }
  }
}

static void 
toolbar_save(GtkWidget *widget, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  guint text_length;
  FILE *fp;
  gchar *buffer;

  if (filename == NULL) {
    select_save_file_as (dasher_text_view);
  }
  else {
    if ((fp = fopen(filename, "w")) == NULL) {
      //      gpe_perror_box (filename);
    }
    else {
      text_length = gtk_text_get_length (GTK_TEXT (dasher_text_view->text_view));
      buffer = (gchar *) g_malloc (text_length);
      buffer = gtk_editable_get_chars (GTK_EDITABLE (dasher_text_view->text_view), 0, text_length);
      
      fwrite (buffer, 1, text_length, fp);
      fclose (fp);
      g_free (buffer);

      file_modified = FALSE;
    }
  }
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
    dasher_canvas->interface->TapOn(x,y,get_time());
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

  delete (dasher_canvas->wrapper);
  dasher_canvas->wrapper = new Gtk2DasherCanvas::Gtk2ScreenWrapper(dasher_canvas->canvas->allocation.width, dasher_canvas->canvas->allocation.height, dasher_canvas);

  dasher_canvas->interface->ChangeScreen(dasher_canvas->wrapper);

  dasher_canvas->CreateNewBuffer();  
  dasher_canvas->interface->Redraw();

  return FALSE;
}

static void
edit_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);
  //dasher_text_view->get_new_context();

  dasher_text_view->flush_count = 0;

  dasher_text_view->interface->Start();  
  dasher_text_view->interface->Redraw();  
}

static void
edit_key_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  Gtk2DasherEdit *dasher_text_view = static_cast<Gtk2DasherEdit*>(data);

  dasher_text_view->flush_count = 0;

  dasher_text_view->interface->Start();  
  dasher_text_view->interface->Redraw();  
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
    dasher_canvas->interface->Unpause(get_time());
    paused = FALSE;
  } else {
    dasher_canvas->interface->PauseAt((gint) event->x,(gint) event->y);
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

  dasher_canvas->interface->PauseAt((gint) event->x,(gint) event->y);
  paused = TRUE;

  return FALSE;
}

static void speed_changed(GtkAdjustment *adj, Gtk2DasherCanvas *dasher_canvas) {
  dasher_canvas->interface->ChangeMaxBitRate(adj->value);
}

class GtkDasherUI : Dasher::CDasherSettingsInterface
{
public:
  
  GtkDasherUI(Dasher::CDasherInterface *interface) {

    char *system_data_dir;
    char *home_dir;
    char *user_data_dir;

    GtkDasherStore *store = new GtkDasherStore;
    interface->SetSettingsStore( store );
    interface->SetSettingsUI( this );
    
    dasher_accel = gtk_accel_group_new();
    
    dasher_menu= gtk_item_factory_new( GTK_TYPE_MENU_BAR,
				       "<DasherMenu>",
				       dasher_accel);
    
    gtk_item_factory_create_items( dasher_menu,
				   54,
				   entries,
				   NULL );
    
    float initial_bitrate = 3.0;
    
    dasher_canvas = new Gtk2DasherCanvas (360, 360, interface);
    dasher_text_view = new Gtk2DasherEdit (interface);
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
    
    interface->SetSystemLocation(system_data_dir);
    interface->SetSystemLocation(user_data_dir);
    
    vbox = gtk_vbox_new (FALSE, 2);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(window), _("Dasher"));
    gtk_window_set_policy (GTK_WINDOW(window), FALSE, FALSE, FALSE);
    
    gtk_window_add_accel_group( GTK_WINDOW(window), dasher_accel);
    dasher_menu_bar=gtk_item_factory_get_widget( dasher_menu, "<DasherMenu>");
    
    toolbar = gtk_toolbar_new ();
    gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
    
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
    
    interface->ChangeLanguageModel(0);
    interface->ChangeView(0);
    
    std::vector< std::string > alphabetlist;
    interface->GetAlphabets( &alphabetlist );  
    
    interface->ChangeAlphabet( alphabetlist[0] );
    
    // load trainig data file -- CHANGE THIS!
    char training_file[ strlen(system_data_dir) + 10 ];
    
    sprintf( training_file, "%strain.txt", system_data_dir );
    interface->TrainFile(std::string(training_file));
    
    interface->ChangeEdit(dasher_text_view);
    interface->ChangeScreen(dasher_canvas->wrapper);
    interface->ChangeMaxBitRate(initial_bitrate);
    
    interface->Start();
    
    gtk_timeout_add(5, timer_callback, dasher_canvas);  
    
    setup = TRUE;

    load_training_file ("/usr/share/dasher/training_english_GB.txt", interface);

  };

  void GtkDasherUI::ShowToolbar(bool Value) {
    if (toolbar==NULL) 
      return;

    if (Value==TRUE) {
      gtk_widget_show(toolbar);
    } else {
      gtk_widget_hide(toolbar);
    }
  };

  void GtkDasherUI::ShowSpeedSlider(bool Value) {
    if (speed_frame==NULL) 
      return;

    if (Value==TRUE) {
      gtk_widget_show(speed_frame);
    } else {
      gtk_widget_hide(speed_frame);
    }
  };

};

static void
open_window() {
     interface = new CDasherInterface;    
     GtkDasherUI *dasherui = new GtkDasherUI(interface);
}

int
main(int argc, char *argv[])
{
  gtk_init (&argc, &argv);

  setlocale (LC_ALL, "");

  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);

  open_window ();
  
  gtk_main ();

  return 0;
}

void clipboard_copy(void) {
  dasher_text_view->Copy();
}

void clipboard_cut(void) {
  dasher_text_view->Cut();
}

void clipboard_paste(void) {
  dasher_text_view->Paste();
}

void orientation(gpointer data, guint action, GtkWidget  *widget )
{
  signed int RealAction=action-3;
  interface->ChangeOrientation(Dasher::Opts::ScreenOrientations(RealAction));
}

void show_toolbar(gpointer data, guint action, GtkWidget  *widget )
{

  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    interface->ShowToolbar( TRUE );
  } else {
    interface->ShowToolbar( FALSE );
  }

}

void show_slider(gpointer data, guint action, GtkWidget  *widget )
{
  if(GTK_CHECK_MENU_ITEM(widget)->active) {
    interface->ShowSpeedSlider( TRUE );
  } else {
    interface->ShowSpeedSlider( FALSE );
  }
}

