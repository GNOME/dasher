#include "../Common/Common.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>
#include <iostream>
#include "DasherControl.h"
#include "Timer.h"
#include "../DasherCore/Event.h"
#include "../DasherCore/ModuleManager.h"

#include <fcntl.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

// 'Private' methods (only used in this file)
extern "C" gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
extern "C" gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
extern "C" void canvas_destroy_event(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean canvas_focus_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);
extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);

static bool g_iTimeoutID = 0;

// CDasherControl class definitions
CDasherControl::CDasherControl(GtkVBox *pVBox, GtkDasherControl *pDasherControl) {
  m_pPangoCache = NULL;
  m_pScreen = NULL;

  m_pDasherControl = pDasherControl;
  m_pVBox = GTK_WIDGET(pVBox);

  Realize();
 
  //  m_pKeyboardHelper = new CKeyboardHelper(this);
  //  m_pKeyboardHelper->Grab(GetBoolParameter(BP_GLOBAL_KEYBOARD));
}

void CDasherControl::CreateModules() {
  CDasherInterfaceBase::CreateModules(); //create default set first
  // Create locally cached copies of the mouse input objects, as we
  // need to pass coordinates to them from the timer callback
  m_pMouseInput =
    (CDasherMouseInput *)  RegisterModule(new CDasherMouseInput(m_pEventHandler, m_pSettingsStore));
  SetDefaultInputDevice(m_pMouseInput);
  m_p1DMouseInput =
    (CDasher1DMouseInput *)RegisterModule(new CDasher1DMouseInput(m_pEventHandler, m_pSettingsStore));
  RegisterModule(new CSocketInput(m_pEventHandler, m_pSettingsStore));

#ifdef JOYSTICK
  RegisterModule(new CDasherJoystickInput(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CDasherJoystickInputDiscrete(m_pEventHandler, m_pSettingsStore, this));
#endif
  
#ifdef TILT
  RegisterModule(new CDasherTiltInput(m_pEventHandler, m_pSettingsStore, this));
#endif
}

void CDasherControl::SetupUI() {
  m_pCanvas = gtk_drawing_area_new();
  GTK_WIDGET_SET_FLAGS(m_pCanvas, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS(m_pCanvas, GTK_DOUBLE_BUFFERED);

  GtkWidget *pFrame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(pFrame), GTK_SHADOW_IN); 
  gtk_container_add(GTK_CONTAINER(pFrame), m_pCanvas);

  gtk_box_pack_start(GTK_BOX(m_pVBox), pFrame, TRUE, TRUE, 0);
  gtk_widget_show_all(GTK_WIDGET(m_pVBox));

  // Connect callbacks - note that we need to implement the callbacks
  // as "C" style functions and pass this as user data so they can
  // call the object

  g_signal_connect(m_pCanvas, "button_press_event", G_CALLBACK(button_press_event), this);
  g_signal_connect(m_pCanvas, "button_release_event", G_CALLBACK(button_press_event), this);
  g_signal_connect_after(m_pCanvas, "realize", G_CALLBACK(realize_canvas), this);
  g_signal_connect(m_pCanvas, "configure_event", G_CALLBACK(canvas_configure_event), this);
  g_signal_connect(m_pCanvas, "destroy", G_CALLBACK(canvas_destroy_event), this);

  g_signal_connect(m_pCanvas, "key-release-event", G_CALLBACK(key_release_event), this);
  g_signal_connect(m_pCanvas, "key_press_event", G_CALLBACK(key_press_event), this);

  g_signal_connect(m_pCanvas, "focus_in_event", G_CALLBACK(canvas_focus_event), this);
  g_signal_connect(m_pCanvas, "expose_event", G_CALLBACK(canvas_expose_event), this);

  // Create the Pango cache

  // TODO: Use system defaults?
  if(GetStringParameter(SP_DASHER_FONT) == "")
    SetStringParameter(SP_DASHER_FONT, "Sans 10");
 
  m_pPangoCache = new CPangoCache(GetStringParameter(SP_DASHER_FONT));

}


void CDasherControl::SetupPaths() {
  char *home_dir;
  char *user_data_dir;
  const char *system_data_dir;

  home_dir = getenv("HOME");
  user_data_dir = new char[strlen(home_dir) + 10];
  sprintf(user_data_dir, "%s/.dasher/", home_dir);

  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

  // PROGDATA is provided by the makefile
  system_data_dir = PROGDATA "/";

  SetStringParameter(SP_SYSTEM_LOC, system_data_dir);
  SetStringParameter(SP_USER_LOC, user_data_dir);
  delete[] user_data_dir;
}

void CDasherControl::CreateSettingsStore() {
  m_pSettingsStore = new CGnomeSettingsStore(m_pEventHandler);
}

void CDasherControl::ScanAlphabetFiles(std::vector<std::string> &vFileList) {
  GDir *directory;
  G_CONST_RETURN gchar *filename;
  GPatternSpec *alphabetglob;
  alphabetglob = g_pattern_spec_new("alphabet*xml");

  directory = g_dir_open(GetStringParameter(SP_SYSTEM_LOC).c_str(), 0, NULL);

  if(directory) {
    while((filename = g_dir_read_name(directory))) {
      if(g_pattern_match_string(alphabetglob, filename)) 
	vFileList.push_back(filename);
    }
    g_dir_close(directory);
  }

  directory = g_dir_open(GetStringParameter(SP_USER_LOC).c_str(), 0, NULL);

  if(directory) {
    while((filename = g_dir_read_name(directory))) {
      if(g_pattern_match_string(alphabetglob, filename))
	vFileList.push_back(filename);
    }
    g_dir_close(directory);
  }

  g_pattern_spec_free(alphabetglob);
}

void CDasherControl::ScanColourFiles(std::vector<std::string> &vFileList) {
  GDir *directory;
  G_CONST_RETURN gchar *filename;

  GPatternSpec *colourglob;
  colourglob = g_pattern_spec_new("colour*xml");

  directory = g_dir_open(GetStringParameter(SP_SYSTEM_LOC).c_str(), 0, NULL);

  if(directory) {
    while((filename = g_dir_read_name(directory))) {
      if(g_pattern_match_string(colourglob, filename))
	vFileList.push_back(filename);
    }
    g_dir_close(directory);
  }

  directory = g_dir_open(GetStringParameter(SP_USER_LOC).c_str(), 0, NULL);

  if(directory) {
    while((filename = g_dir_read_name(directory))) {
      if(g_pattern_match_string(colourglob, filename))
	vFileList.push_back(filename);
    }
    g_dir_close(directory);
  }

  g_pattern_spec_free(colourglob);
}

CDasherControl::~CDasherControl() {
  if(m_pMouseInput) {
    m_pMouseInput = NULL;
  }

  if(m_p1DMouseInput) {
    m_p1DMouseInput = NULL;
  }

  if(m_pPangoCache) {
    delete m_pPangoCache;
    m_pPangoCache = NULL;
  }

//   if(m_pKeyboardHelper) {
//     delete m_pKeyboardHelper;
//     m_pKeyboardHelper = 0;
//   }
}

bool CDasherControl::FocusEvent(GtkWidget *pWidget, GdkEventFocus *pEvent) {
  if((pEvent->type == GDK_FOCUS_CHANGE) && (pEvent->in)) {
    GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
    gboolean *returnType;
    
    focusEvent->type = GDK_FOCUS_CHANGE;
    focusEvent->window = (GdkWindow *) m_pDasherControl;
    focusEvent->send_event = FALSE;
    focusEvent->in = TRUE;

    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "focus_in_event", GTK_WIDGET(m_pDasherControl), focusEvent, NULL, &returnType);
  }
  return true;
}

void CDasherControl::SetFocus() {
  gtk_widget_grab_focus(m_pCanvas);
}

void CDasherControl::GameMessageOut(int message, const void* messagedata) {
  gtk_dasher_control_game_messageout(m_pDasherControl, message, messagedata);
}

GArray *CDasherControl::GetAllowedValues(int iParameter) {
  // Glib version of the STL based core function

  GArray *pRetVal(g_array_new(false, false, sizeof(gchar *)));

  std::vector < std::string > vList;
  GetPermittedValues(iParameter, vList);

  for(std::vector < std::string >::iterator it(vList.begin()); it != vList.end(); ++it) {
    // For internal glib reasons we need to make a variable and then
    // pass - we can't use the iterator directly
    const char *pTemp(it->c_str());
    char *pTempNew = new char[strlen(pTemp) + 1];
    strcpy(pTempNew, pTemp);
    g_array_append_val(pRetVal, pTempNew);
  }

  return pRetVal;
}

void CDasherControl::RealizeCanvas(GtkWidget *pWidget) {
  // TODO: Pointless function - call directly from C callback.
#ifdef DEBUG
  std::cout << "RealizeCanvas()" << std::endl;
#endif
  OnUIRealised();
}

void CDasherControl::StartTimer() {
  // Start the timer loops as everything is set up.
  // Aim for 40 frames per second, computers are getting faster.

  if(g_iTimeoutID == 0) {
    g_iTimeoutID = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 25, timer_callback, this, NULL);
    // TODO: Reimplement this (or at least reimplement some kind of status reporting)
    //g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 5000, long_timer_callback, this, NULL);
  }
}

void CDasherControl::ShutdownTimer() {
  // TODO: Figure out how to implement this - at the moment it's done
  // through a return value from the timer callback, but it would be
  // nicer to prevent any further calls as soon as the shutdown signal
  // has been receieved.
}

int CDasherControl::CanvasConfigureEvent() {

  if(m_pScreen != NULL)
    delete m_pScreen;

  m_pScreen = new CCanvas(m_pCanvas, m_pPangoCache);
  ChangeScreen(m_pScreen);
 
  return 0;
}

void CDasherControl::ExternalEventHandler(Dasher::CEvent *pEvent) {
  // Convert events coming from the core to Glib signals.

  if(pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    HandleParameterNotification(pEvt->m_iParameter);
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_changed", pEvt->m_iParameter);
  }
  else if(pEvent->m_iEventType == EV_EDIT) {
    CEditEvent *pEditEvent(static_cast < CEditEvent * >(pEvent));
    
    if(pEditEvent->m_iEditType == 1) {
      // Insert event
      g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_edit_insert", pEditEvent->m_sText.c_str(), pEditEvent->m_iOffset);
    }
    else if(pEditEvent->m_iEditType == 2) {
      // Delete event
      g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_edit_delete", pEditEvent->m_sText.c_str(), pEditEvent->m_iOffset);
    }
    else if(pEditEvent->m_iEditType == 10) {
      g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_edit_convert");
    }
    else if(pEditEvent->m_iEditType == 11) {
      g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_edit_protect");
    }
  }
  else if(pEvent->m_iEventType == EV_EDIT_CONTEXT) {
    CEditContextEvent *pEditContextEvent(static_cast < CEditContextEvent * >(pEvent));
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_context_request", pEditContextEvent->m_iOffset, pEditContextEvent->m_iLength);
  }
  else if(pEvent->m_iEventType == EV_START) {
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_start");
  }
  else if(pEvent->m_iEventType == EV_STOP) {
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_stop");
  }
  else if(pEvent->m_iEventType == EV_CONTROL) {
    CControlEvent *pControlEvent(static_cast < CControlEvent * >(pEvent));
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_control", pControlEvent->m_iID);
  }
  else if(pEvent->m_iEventType == EV_LOCK) {
    CLockEvent *pLockEvent(static_cast<CLockEvent *>(pEvent));
    DasherLockInfo sInfo;
    sInfo.szMessage = pLockEvent->m_strMessage.c_str();
    sInfo.bLock = pLockEvent->m_bLock;
    sInfo.iPercent = pLockEvent->m_iPercent;

    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_lock_info", &sInfo);
  }
  else if(pEvent->m_iEventType == EV_MESSAGE) {
    CMessageEvent *pMessageEvent(static_cast<CMessageEvent *>(pEvent));
    DasherMessageInfo sInfo;
    sInfo.szMessage = pMessageEvent->m_strMessage.c_str();
    sInfo.iID = pMessageEvent->m_iID;
    sInfo.iType = pMessageEvent->m_iType;

    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_message", &sInfo);
  }
  else if(pEvent->m_iEventType == EV_COMMAND) {
    CCommandEvent *pCommandEvent(static_cast<CCommandEvent *>(pEvent));
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_command", pCommandEvent->m_strCommand.c_str());
  }
};

void CDasherControl::WriteTrainFile(const std::string &strNewText) {
  if(strNewText.length() == 0)
    return;

  std::string strFilename(GetStringParameter(SP_USER_LOC) + GetStringParameter(SP_TRAIN_FILE));

  int fd=open(strFilename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,strNewText.c_str(),strNewText.length());
  close(fd);
}

// TODO: Sort these methods out
void CDasherControl::ExternalKeyDown(int iKeyVal) {
//   if(m_pKeyboardHelper) {
//     int iButtonID(m_pKeyboardHelper->ConvertKeycode(iKeyVal));

//     if(iButtonID != -1)
//       KeyDown(get_time(), iButtonID);
//   }
  KeyDown(get_time(), iKeyVal);
}

void CDasherControl::ExternalKeyUp(int iKeyVal) {
//   if(m_pKeyboardHelper) {
//     int iButtonID(m_pKeyboardHelper->ConvertKeycode(iKeyVal));
    
//     if(iButtonID != -1)
//       KeyUp(get_time(), iButtonID);
//   }
  KeyUp(get_time(), iKeyVal);
}

void CDasherControl::HandleParameterNotification(int iParameter) {
  switch(iParameter) {
  case SP_DASHER_FONT:
    if(m_pPangoCache) {
      m_pPangoCache->ChangeFont(GetStringParameter(SP_DASHER_FONT));
      ScheduleRedraw();
    }
    break;
  case BP_GLOBAL_KEYBOARD:
    // TODO: reimplement
//     if(m_pKeyboardHelper)
//       m_pKeyboardHelper->Grab(GetBoolParameter(BP_GLOBAL_KEYBOARD));
    break;
  }
}

int CDasherControl::TimerEvent() {
  int x, y;

  gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);
  m_pMouseInput->SetCoordinates(x, y);

  gdk_window_get_pointer(gdk_get_default_root_window(), &x, &y, NULL);

  int iRootWidth;
  int iRootHeight;

  gdk_drawable_get_size(gdk_get_default_root_window(), &iRootWidth, &iRootHeight);

  if(GetLongParameter(LP_YSCALE) < 10)
    SetLongParameter(LP_YSCALE, 10);

  y = (y - iRootHeight / 2);

  m_p1DMouseInput->SetCoordinates(y, GetLongParameter(LP_YSCALE));

  NewFrame(get_time(), false);

  // Update our UserLog object about the current mouse position
  CUserLogBase* pUserLog = GetUserLogPtr();
  if (pUserLog != NULL) {  
      // We want current canvas and window coordinates so normalization
      // is done properly with respect to the canvas.
      GdkRectangle sWindowRect;
      GdkRectangle sCanvasRect;

      gdk_window_get_frame_extents(m_pCanvas->window, &sWindowRect);

      pUserLog->AddWindowSize(sWindowRect.y, 
                              sWindowRect.x, 
                              sWindowRect.y + sWindowRect.height, 
                              sWindowRect.x + sWindowRect.width);

      if (m_pScreen != NULL) {
        if (m_pScreen->GetCanvasSize(&sCanvasRect))
          pUserLog->AddCanvasSize(sCanvasRect.y, 
                                  sCanvasRect.x, 
                                  sCanvasRect.y + sCanvasRect.height, 
                                  sCanvasRect.x + sCanvasRect.width);
      }

      int iMouseX = 0;
      int iMouseY = 0;  
      gdk_window_get_pointer(NULL, &iMouseX, &iMouseY, NULL);

      // TODO: This sort of thing shouldn't be in specialised methods, move into base class somewhere
      pUserLog->AddMouseLocationNormalized(iMouseX, iMouseY, true, GetNats());
  }

  return 1;

  // See CVS for code which used to be here
}

int CDasherControl::LongTimerEvent() {
//   std::cout << "Framerate: " << GetFramerate() << std::endl;
//   std::cout << "Render count: " << GetRenderCount() << std::endl;
  return 1;
}

gboolean CDasherControl::ExposeEvent() {
  NewFrame(get_time(), true);
  return 0;
}

gboolean CDasherControl::ButtonPressEvent(GdkEventButton *event) {

  // Take the focus if we click on the canvas

//   GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
//   gboolean *returnType;

//   focusEvent->type = GDK_FOCUS_CHANGE;
//   focusEvent->window = (GdkWindow *) m_pCanvas;
//   focusEvent->send_event = FALSE;
//   focusEvent->in = TRUE;

//   gtk_widget_grab_focus(GTK_WIDGET(m_pCanvas));
//   g_signal_emit_by_name(GTK_OBJECT(m_pCanvas), "focus_in_event", GTK_WIDGET(m_pCanvas), focusEvent, NULL, &returnType);

  // No - don't take the focus - give it to the text area instead
  
  if(event->type == GDK_BUTTON_PRESS)
    HandleClickDown(get_time(), (int)event->x, (int)event->y);
  else if(event->type == GDK_BUTTON_RELEASE)
    HandleClickUp(get_time(), (int)event->x, (int)event->y);

  return false;
}

gint CDasherControl::KeyReleaseEvent(GdkEventKey *event) {
  // TODO: This is seriously flawed - the semantics of of X11 Keyboard
  // events mean the there's no guarantee that key up/down events will
  // be received in pairs.

  if((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R)) {
//     if(event->state & GDK_CONTROL_MASK)
//       SetLongParameter(LP_BOOSTFACTOR, 25);
//     else
//       SetLongParameter(LP_BOOSTFACTOR, 100);
  }
  else if((event->keyval == GDK_Control_L) || (event->keyval == GDK_Control_R)) {
//     if(event->state & GDK_SHIFT_MASK)
//       SetLongParameter(LP_BOOSTFACTOR, 175);
//     else
//       SetLongParameter(LP_BOOSTFACTOR, 100);
  }
  else {
//     if(m_pKeyboardHelper) {
//       int iKeyVal(m_pKeyboardHelper->ConvertKeycode(event->keyval));
      
//       if(iKeyVal != -1)
// 	KeyUp(get_time(), iKeyVal);
//     }
  }

  return 0;
}

gint CDasherControl::KeyPressEvent(GdkEventKey *event) {
  //  if((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R))
    //    SetLongParameter(LP_BOOSTFACTOR, 175);
  //  else if((event->keyval == GDK_Control_L) || (event->keyval == GDK_Control_R))
  //   SetLongParameter(LP_BOOSTFACTOR, 25);
  // else {
//     if(m_pKeyboardHelper) {
//       int iKeyVal(m_pKeyboardHelper->ConvertKeycode(event->keyval));
      
//       if(iKeyVal != -1)
// 	KeyDown(get_time(), iKeyVal);
//     }
    //  }
  return 0;
}

void CDasherControl::CanvasDestroyEvent() {
  // Delete the screen

  if(m_pScreen != NULL) {
    delete m_pScreen;
    m_pScreen = NULL;
  }
}

// Tell the logging object that a new user trial is starting.
void CDasherControl::UserLogNewTrial()
{
  CUserLogBase* pUserLog = GetUserLogPtr();
  if (pUserLog != NULL) { 
    pUserLog->NewTrial();
  }
}

int CDasherControl::GetFileSize(const std::string &strFileName) {
  struct stat sStatInfo;

  if(!stat(strFileName.c_str(), &sStatInfo))
    return sStatInfo.st_size;
  else
    return 0;
}

// "C" style callbacks - these are here just because it's not possible
// (or at least not easy) to connect a callback directly to a C++
// method, so we pass a pointer to th object in the user_data field
// and use a wrapper function. Please do not put any functional code
// here.

extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data) {
  static_cast < CDasherControl * >(user_data)->RealizeCanvas(widget);
}


extern "C" gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
  return static_cast < CDasherControl * >(data)->ButtonPressEvent(event);
}

extern "C" gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  return static_cast < CDasherControl * >(data)->KeyPressEvent(event);
}

extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data) {
  return static_cast < CDasherControl * >(data)->CanvasConfigureEvent();
}

extern "C" void canvas_destroy_event(GtkWidget *pWidget, gpointer pUserData) {
  static_cast<CDasherControl*>(pUserData)->CanvasDestroyEvent();
}

extern "C" gint key_release_event(GtkWidget *pWidget, GdkEventKey *event, gpointer pUserData) {
  return static_cast<CDasherControl*>(pUserData)->KeyReleaseEvent(event);
}

extern "C" gboolean canvas_focus_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return static_cast < CDasherControl * >(data)->FocusEvent(widget, event);
}

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  return ((CDasherControl*)data)->ExposeEvent();
}
