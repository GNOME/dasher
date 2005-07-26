#include "DasherControl.h"
#include "Timer.h"
#include "../DasherCore/DasherInterface.h"
#include "../DasherCore/Event.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>

// 'Private' methods (only used in this file)

extern "C" gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data);
extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
extern "C" gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
extern "C" void canvas_destroy_event(GtkWidget *pWidget, gpointer pUserData);

// Global variables - Make as many of these local or clas members as possible.

char *system_data_dir;
char *user_data_dir;

// 'Public' member variables

gboolean paused = FALSE;
gboolean exiting = FALSE;
gboolean training = FALSE;
GAsyncQueue *trainqueue;
GtkWidget *train_dialog;
std::string alphabet;
ControlTree *controltree;
bool eyetrackermode = false;
gboolean direction = TRUE;
bool onedmode = false;
gint dasherwidth, dasherheight;
long yscale, mouseposstartdist = 0;
gboolean mouseposstart;
gboolean firstbox = FALSE;
gboolean secondbox = FALSE;
time_t starttime = 0;
time_t starttime2 = 0;
time_t dasherstarttime;

int oldx;
int oldy;

// CDasherControl class definitions

CDasherControl::CDasherControl(GtkVBox *pVBox, GtkDasherControl *pDasherControl) {

  m_pDasherControl = pDasherControl;

  // Set up the GTK widgets

  m_pVBox = GTK_WIDGET(pVBox);

  m_pCanvas = gtk_drawing_area_new();
  GTK_WIDGET_SET_FLAGS(m_pCanvas, GTK_CAN_FOCUS);

  GtkWidget *pFrame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(pFrame), GTK_SHADOW_IN); 
  
  m_pSpeedFrame = gtk_frame_new("Speed:");
  m_pSpeedHScale = gtk_hscale_new_with_range(0.1, 8.0, 0.1);

  gtk_container_add(GTK_CONTAINER(m_pSpeedFrame), m_pSpeedHScale);
  gtk_container_add(GTK_CONTAINER(pFrame), m_pCanvas);

  gtk_box_pack_start(GTK_BOX(m_pVBox), pFrame, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(m_pVBox), m_pSpeedFrame, FALSE, FALSE, 0);

  gtk_widget_show_all(GTK_WIDGET(m_pVBox));

  // Connect callbacks - note that we need to implement the callbacks
  // as "C" style functions and pass this as user data so they can
  // call the object

  g_signal_connect(m_pCanvas, "button_press_event", G_CALLBACK(button_press_event), this);
  g_signal_connect(m_pSpeedHScale, "value-changed", G_CALLBACK(speed_changed), this);
  g_signal_connect_after(m_pCanvas, "realize", G_CALLBACK(realize_canvas), this);
  g_signal_connect(m_pCanvas, "configure_event", G_CALLBACK(canvas_configure_event), this);
  g_signal_connect(m_pCanvas, "destroy", G_CALLBACK(canvas_destroy_event), this);

  // We'll use the same call back for keyboard events from the canvas
  // and slider - maybe this isn't the right thing to do long term

  g_signal_connect(m_pCanvas, "key_press_event", G_CALLBACK(key_press_event), this);
  g_signal_connect(m_pSpeedHScale, "key_press_event", G_CALLBACK(key_press_event), this);

  // Set up directory locations and so on.

  char *home_dir;

  home_dir = getenv("HOME");
  user_data_dir = new char[strlen(home_dir) + 10];
  sprintf(user_data_dir, "%s/.dasher/", home_dir);

  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

  // PROGDATA is provided by the makefile
  system_data_dir = PROGDATA "/";

  SetStringParameter(SP_SYSTEM_LOC, system_data_dir);
  SetStringParameter(SP_USER_LOC, user_data_dir);

  // Add all available alphabets and colour schemes to the core

  scan_alphabet_files();
  scan_colour_files();

  Realize();

  // Start the dasher model

  Start();        // FIXME - should we hold off on this until later?

  // Tell the core that we handle edit events etc.

//   m_pInterface->ChangeEdit(this);
//   m_pInterface->SetSettingsUI(this);

  // Create an input device object - FIXME - should make this more flexible

  m_pMouseInput = new CDasherMouseInput;
  SetInput(m_pMouseInput);

  // Create a pango cache

  m_pPangoCache = new CPangoCache;

  // Don't create the screen until we've been realised.

  m_pScreen = NULL;

}

CDasherControl::~CDasherControl() {

  // Delete the input device

  if(m_pMouseInput != NULL) {
    delete m_pMouseInput;
    m_pMouseInput = NULL;
  }
}


GArray *CDasherControl::GetAllowedValues(int iParameter) {

  // FIXME - this should really be implemented in DasherInterface in
  // place of GetAlphabets and GetColours

  GArray *pRetVal(g_array_new(false, false, sizeof(gchar *)));

  std::vector < std::string > vList;

  switch (iParameter) {
  case SP_ALPHABET_ID:
    GetAlphabets(&vList);
    break;
  case SP_COLOUR_ID:
    GetColours(&vList);
    break;
  }

  for(std::vector < std::string >::iterator it(vList.begin()); it != vList.end(); ++it) {
    // For internal glib reasons we need to make a variable and then
    // pass - we can't use the iterator directly
    const char *pTemp(it->c_str());
    g_array_append_val(pRetVal, pTemp);
  }

  return pRetVal;
}

void CDasherControl::RealizeCanvas() {
  // Start the timer loops as everything is set up
  // Aim for 20 frames per second

  g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 50, timer_callback, this, NULL);
}

void CDasherControl::CanvasConfigureEvent() {

  if(m_pScreen != NULL)
    delete m_pScreen;

  m_pScreen = new CCanvas(m_pCanvas, m_pPangoCache);
  ChangeScreen(m_pScreen);

  Redraw();
}

void CDasherControl::ExternalEventHandler(Dasher::CEvent *pEvent) {
  // Pass events outside
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    HandleParameterNotification(pEvt->m_iParameter);
  }
  else if((pEvent->m_iEventType >= 2) && (pEvent->m_iEventType <= 6)) {
    HandleEvent(pEvent);
  }

}

void CDasherControl::HandleParameterNotification(int iParameter) {

  if(iParameter == SP_DASHER_FONT) {
    m_pPangoCache->ChangeFont(GetStringParameter(SP_DASHER_FONT));
    Redraw();
  }
  else if(iParameter == LP_MAX_BITRATE) {
    gtk_range_set_value(GTK_RANGE(m_pSpeedHScale), GetLongParameter(LP_MAX_BITRATE) / 100.0);
  }
  else if(iParameter == BP_SHOW_SLIDER) {
    if(m_pSpeedFrame != NULL) {
      if(GetBoolParameter(BP_SHOW_SLIDER)) {
        gtk_widget_show(GTK_WIDGET(m_pSpeedFrame));
        gtk_range_set_value(GTK_RANGE(m_pSpeedHScale), GetLongParameter(LP_MAX_BITRATE) / 100.0);
      }
      else {
        gtk_widget_hide(GTK_WIDGET(m_pSpeedFrame));
      }
    }
  }

  // Emit a dasher_changed signal to notify the application about changes.

  g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_changed", iParameter);
}

void CDasherControl::HandleEvent(CEvent *pEvent) {
  if(pEvent->m_iEventType == 2) {
    CEditEvent *pEditEvent(static_cast < CEditEvent * >(pEvent));

    if(pEditEvent->m_iEditType == 1) {
      // Insert event
      g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_edit_insert", pEditEvent->m_sText.c_str());
    }
    else if(pEditEvent->m_iEditType == 2) {
      // Delete event
      g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_edit_delete", pEditEvent->m_sText.c_str());
    }
  }
  else if(pEvent->m_iEventType == 4) {
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_start");
  }
  else if(pEvent->m_iEventType == 5) {
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_stop");
  }
  else if(pEvent->m_iEventType == 6) {
    CControlEvent *pControlEvent(static_cast < CControlEvent * >(pEvent));
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_control", pControlEvent->m_iID);
  }
};

int CDasherControl::TimerEvent() {

  int x, y;

  gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);
  m_pMouseInput->SetCoordinates(x, y);

  NewFrame(get_time());

  return 1;

  // See CVS for code which used to be here
}

gboolean CDasherControl::ButtonPressEvent(GdkEventButton *event) {

  // Take the focus if we click on the canvas

  GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
  gboolean *returnType;

  focusEvent->type = GDK_FOCUS_CHANGE;
  focusEvent->window = (GdkWindow *) m_pCanvas;
  focusEvent->send_event = FALSE;
  focusEvent->in = TRUE;

  gtk_widget_grab_focus(GTK_WIDGET(m_pCanvas));
  g_signal_emit_by_name(GTK_OBJECT(m_pCanvas), "focus_in_event", GTK_WIDGET(m_pCanvas), focusEvent, NULL, &returnType);

#ifdef WITH_GPE
  // GPE version requires the button to be held down rather than clicked
  if((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_BUTTON_RELEASE))
    return FALSE;
#else
  if((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_2BUTTON_PRESS))
    return FALSE;
#endif
 
  // FIXME - This should be moved into a toggle pause routime in CDasherInterface
  
  if(GetBoolParameter(BP_START_MOUSE)) {
    if(GetBoolParameter(BP_DASHER_PAUSED))
      Unpause(get_time());
    else
      PauseAt(0, 0);
  }

  return false;
}

gint CDasherControl::KeyPressEvent(GdkEventKey *event) {

  switch (event->keyval) {
  case GDK_space:
    // FIXME - wrap this in a 'start/stop' method (and use for buttons as well as keys)

    if(GetBoolParameter(BP_START_SPACE)) {
      if(GetBoolParameter(BP_DASHER_PAUSED))
        Unpause(get_time());
      else
        PauseAt(0, 0);
    }
    break;
  }
  
  return 0;
}

void CDasherControl::SliderEvent() {
  if(GetLongParameter(LP_MAX_BITRATE) != long(GTK_RANGE(m_pSpeedHScale)->adjustment->value * 100))
    SetLongParameter(LP_MAX_BITRATE, long(GTK_RANGE(m_pSpeedHScale)->adjustment->value * 100));
}

void CDasherControl::CanvasDestroyEvent() {
  // Delete the screen

  if(m_pScreen != NULL) {
    delete m_pScreen;
    m_pScreen = NULL;
  }
}

void CDasherControl::scan_alphabet_files() {
  // Hurrah for glib making this a nice easy thing to do
  // rather than the WORLD OF PAIN it would otherwise be
  GDir *directory;
  G_CONST_RETURN gchar *filename;
  GPatternSpec *alphabetglob;
  alphabetglob = g_pattern_spec_new("alphabet*xml");
  directory = g_dir_open(system_data_dir, 0, NULL);

  while((filename = g_dir_read_name(directory))) {
    if(alphabet_filter(filename, alphabetglob)) {
      AddAlphabetFilename(filename);
    }
  }

  directory = g_dir_open(user_data_dir, 0, NULL);

  while((filename = g_dir_read_name(directory))) {
    if(alphabet_filter(filename, alphabetglob)) {
      AddAlphabetFilename(filename);
    }
  }

  // FIXME - need to delete glob?
}

void CDasherControl::scan_colour_files() {
  GDir *directory;
  G_CONST_RETURN gchar *filename;

  GPatternSpec *colourglob;
  colourglob = g_pattern_spec_new("colour*xml");

  directory = g_dir_open(system_data_dir, 0, NULL);

  while((filename = g_dir_read_name(directory))) {
    if(colour_filter(filename, colourglob)) {
      AddColourFilename(filename);
    }
  }

  directory = g_dir_open(user_data_dir, 0, NULL);

  while((filename = g_dir_read_name(directory))) {
    if(colour_filter(filename, colourglob)) {
      AddColourFilename(filename);
    }
  }

  // FIXME - need to delete glob?
}

// FIXME - these two methods seem a bit pointless!

int CDasherControl::alphabet_filter(const gchar *filename, GPatternSpec *alphabetglob) {
  return int (g_pattern_match_string(alphabetglob, filename));
}

int CDasherControl::colour_filter(const gchar *filename, GPatternSpec *colourglob) {
  return int (g_pattern_match_string(colourglob, filename));
}

// "C" style callbacks - these are here just because it's not possible
// (or at least not easy) to connect a callback directly to a C++
// method, so we pass a pointer to th object in the user_data field
// and use a wrapper function. Please do not put any functional code
// here.

extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data) {
  static_cast < CDasherControl * >(user_data)->RealizeCanvas();
}

extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data) {
  static_cast < CDasherControl * >(user_data)->SliderEvent();
}

extern "C" gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
  return static_cast < CDasherControl * >(data)->ButtonPressEvent(event);
}

extern "C" gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  return static_cast < CDasherControl * >(data)->KeyPressEvent(event);
}

extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data) {
  ((CDasherControl *) data)->CanvasConfigureEvent();

  // TODO - implement code in UI (ie not here) to save window dimensions on resize

  return FALSE;
}

extern "C" void canvas_destroy_event(GtkWidget *pWidget, gpointer pUserData) {
  static_cast<CDasherControl*>(pUserData)->CanvasDestroyEvent();
}
