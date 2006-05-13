#include "../Common/Common.h"

#include <iostream>
#include "DasherControl.h"
#include "Timer.h"
#include "../DasherCore/Event.h"
#include "../DasherCore/WrapperFactory.h"

#include <fcntl.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>
using namespace std;

// 'Private' methods (only used in this file)
extern "C" gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data);
extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
extern "C" gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
extern "C" void canvas_destroy_event(GtkWidget *pWidget, gpointer pUserData);
extern "C" void alphabet_combo_changed(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean canvas_focus_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);

// CDasherControl class definitions

CDasherControl::CDasherControl(GtkVBox *pVBox, GtkDasherControl *pDasherControl) {
  m_pDasherControl = pDasherControl;
  m_pVBox = GTK_WIDGET(pVBox);

  Realize();

  m_iComboCount = 0;
  PopulateAlphabetCombol();
    
  // Start the dasher model

  Start();        // FIXME - should we hold off on this until later?

  // Create input device objects
  // (We create the SocketInput object now even if socket input is not enabled, because
  // we are not allowed to create it in response to a parameter update event later, because
  // that would mean registering a new event listener during the processing of an event.

  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CDasherMouseInput(m_pEventHandler, m_pSettingsStore)));
  RegisterFactory(new CWrapperFactory(m_pEventHandler, m_pSettingsStore, new CSocketInput(m_pEventHandler, m_pSettingsStore)));

  std::cout << "Settings store: " << m_pSettingsStore << std::endl;

  m_pKeyboardHelper = new CKeyboardHelper;
  m_pKeyboardHelper->Grab(GetBoolParameter(BP_GLOBAL_KEYBOARD));

  CreateInput();

//   m_pSocketInput = (CSocketInput *)GetModule(1);
//   m_pSocketInput->Ref();
  
  m_pMouseInput = (CDasherMouseInput *)GetModule(0);
  m_pMouseInput->Ref();

  // Create a pango cache

  m_pPangoCache = new CPangoCache(GetStringParameter(SP_DASHER_FONT));

  // Don't create the screen until we've been realised.

  m_pScreen = NULL;

}

void CDasherControl::SetupUI() {
  m_pCanvas = gtk_drawing_area_new();
  GTK_WIDGET_SET_FLAGS(m_pCanvas, GTK_CAN_FOCUS);
  gtk_widget_set_double_buffered(m_pCanvas, false);

  GtkWidget *pFrame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(pFrame), GTK_SHADOW_IN); 
  
  m_pSpeedFrame = gtk_frame_new("Speed:");

  m_pSpeedHScale = gtk_hscale_new_with_range(0.1, 8.0, 0.1);
  gtk_scale_set_digits( GTK_SCALE(m_pSpeedHScale), 1 );

  gtk_container_add(GTK_CONTAINER(m_pSpeedFrame), m_pSpeedHScale);
  gtk_container_add(GTK_CONTAINER(pFrame), m_pCanvas);


  m_pStatusBar = gtk_hbox_new(false, 2);

  m_pSpin = gtk_spin_button_new_with_range(0.1, 8.0, 0.1);
  m_pCombo = gtk_combo_box_new_text();

  gtk_widget_set_size_request(m_pCombo, 256, -1);

  m_pStatusLabel = gtk_label_new("Characters/min: --");
  gtk_label_set_justify(GTK_LABEL(m_pStatusLabel), GTK_JUSTIFY_RIGHT);

  gtk_box_pack_start(GTK_BOX(m_pStatusBar), gtk_label_new("Speed:"), 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(m_pStatusBar), m_pSpin, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(m_pStatusBar), m_pCombo, 0, 0, 0);
  //  gtk_box_pack_start(GTK_BOX(m_pStatusBar), m_pStatusLabel, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(m_pVBox), pFrame, TRUE, TRUE, 0);
  //  gtk_box_pack_start(GTK_BOX(m_pVBox), m_pSpeedFrame, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(m_pVBox), m_pStatusBar, FALSE, FALSE, 0);

  gtk_widget_show_all(GTK_WIDGET(m_pVBox));

  if(!GetBoolParameter(BP_SHOW_SLIDER))
    gtk_widget_hide(m_pStatusBar);

  // Connect callbacks - note that we need to implement the callbacks
  // as "C" style functions and pass this as user data so they can
  // call the object

  g_signal_connect(m_pCanvas, "button_press_event", G_CALLBACK(button_press_event), this);
  g_signal_connect(m_pCanvas, "button_release_event", G_CALLBACK(button_press_event), this);
  g_signal_connect(m_pSpin, "value-changed", G_CALLBACK(speed_changed), this);
  g_signal_connect_after(m_pCanvas, "realize", G_CALLBACK(realize_canvas), this);
  g_signal_connect(m_pCanvas, "configure_event", G_CALLBACK(canvas_configure_event), this);
  g_signal_connect(m_pCanvas, "destroy", G_CALLBACK(canvas_destroy_event), this);

  g_signal_connect(m_pCombo, "changed", G_CALLBACK(alphabet_combo_changed), this);

  // We'll use the same call back for keyboard events from the canvas
  // and slider - maybe this isn't the right thing to do long term

  g_signal_connect(m_pCanvas, "key-release-event", G_CALLBACK(key_release_event), this);
  g_signal_connect(m_pCanvas, "key_press_event", G_CALLBACK(key_press_event), this);
  g_signal_connect(m_pSpeedHScale, "key_press_event", G_CALLBACK(key_press_event), this);

  g_signal_connect(m_pCanvas, "focus_in_event", G_CALLBACK(canvas_focus_event), this);
}

void CDasherControl::SetupPaths() {
  char *home_dir;
  char *user_data_dir;
  char *system_data_dir;

  home_dir = getenv("HOME");
  user_data_dir = new char[strlen(home_dir) + 10];
  sprintf(user_data_dir, "%s/.dasher/", home_dir);

  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

  // PROGDATA is provided by the makefile
#ifdef WITH_MAEMO
  system_data_dir = "/var/lib/install" PROGDATA "/";
#else
  system_data_dir = PROGDATA "/";
#endif

  SetStringParameter(SP_SYSTEM_LOC, system_data_dir);
  SetStringParameter(SP_USER_LOC, user_data_dir);
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
  while((filename = g_dir_read_name(directory))) {
    if(alphabet_filter(filename, alphabetglob)) {
      vFileList.push_back(filename);
    }
  }
  g_dir_close(directory);

  directory = g_dir_open(GetStringParameter(SP_USER_LOC).c_str(), 0, NULL);
  while((filename = g_dir_read_name(directory))) {
    if(alphabet_filter(filename, alphabetglob)) {
      vFileList.push_back(filename);
    }
  }
  g_dir_close(directory);
  // FIXME - need to delete glob?
}

void CDasherControl::ScanColourFiles(std::vector<std::string> &vFileList) {
  GDir *directory;
  G_CONST_RETURN gchar *filename;

  GPatternSpec *colourglob;
  colourglob = g_pattern_spec_new("colour*xml");

  directory = g_dir_open(GetStringParameter(SP_SYSTEM_LOC).c_str(), 0, NULL);
  while((filename = g_dir_read_name(directory))) {
    if(colour_filter(filename, colourglob)) {
      vFileList.push_back(filename);
    }
  }
  g_dir_close(directory);

  directory = g_dir_open(GetStringParameter(SP_USER_LOC).c_str(), 0, NULL);
  while((filename = g_dir_read_name(directory))) {
    if(colour_filter(filename, colourglob)) {
      vFileList.push_back(filename);
    }
  }
  g_dir_close(directory);

  // FIXME - need to delete glob?
}

CDasherControl::~CDasherControl() {

  WriteTrainFileFull();

  // Delete the input devices

  if(m_pMouseInput != NULL) {
    m_pMouseInput->Unref();
    m_pMouseInput = NULL;
  }

//   if(m_pSocketInput != NULL) {
//     m_pSocketInput->Unref();
//     m_pSocketInput = NULL;
//   }

  if(m_pPangoCache != NULL) {
    delete m_pPangoCache;
    m_pPangoCache = NULL;
  }

  if(m_pKeyboardHelper) {
    delete m_pKeyboardHelper;
    m_pKeyboardHelper = 0;
  }
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

GArray *CDasherControl::GetAllowedValues(int iParameter) {

  // FIXME - this should really be implemented in DasherInterface in
  // place of GetAlphabets and GetColours

  GArray *pRetVal(g_array_new(false, false, sizeof(gchar *)));

  std::vector < std::string > vList;
  GetPermittedValues(iParameter, vList);

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

int CDasherControl::CanvasConfigureEvent() {

  if(m_pScreen != NULL)
    delete m_pScreen;

  m_pScreen = new CCanvas(m_pCanvas, m_pPangoCache);
  ChangeScreen(m_pScreen);
 
  return 0;
}

void CDasherControl::ExternalEventHandler(Dasher::CEvent *pEvent) {
  // Pass events outside
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    HandleParameterNotification(pEvt->m_iParameter);
  }
  // TODO: Horrible - just keep events here
  else if((pEvent->m_iEventType >= 2) && (pEvent->m_iEventType <= 8)) {
    HandleEvent(pEvent);
  }

}

void CDasherControl::WriteTrainFile(const std::string &strNewText) {
  if(strNewText.length() == 0)
    return;

  std::string strFilename(GetStringParameter(SP_USER_LOC) + GetStringParameter(SP_TRAIN_FILE));

  int fd=open(strFilename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,strNewText.c_str(),strNewText.length());
  close(fd);
}

void CDasherControl::ExternalKeyDown(int iKeyVal) {
  if(m_pKeyboardHelper) {
    int iButtonID(m_pKeyboardHelper->ConvertKeycode(iKeyVal));
    
    if(iButtonID != -1)
      KeyDown(get_time(), iButtonID);
  }
}

void CDasherControl::ExternalKeyUp(int iKeyVal) {
  if(m_pKeyboardHelper) {
    int iButtonID(m_pKeyboardHelper->ConvertKeycode(iKeyVal));
    
    if(iButtonID != -1)
      KeyUp(get_time(), iButtonID);
  }
}

void CDasherControl::HandleParameterNotification(int iParameter) {

  switch(iParameter) {
  case SP_DASHER_FONT:
    m_pPangoCache->ChangeFont(GetStringParameter(SP_DASHER_FONT));
    Redraw(true);
    break;
  case LP_MAX_BITRATE:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pSpin), GetLongParameter(LP_MAX_BITRATE) / 100.0);
    break;
  case BP_SHOW_SLIDER:
    if(m_pSpeedFrame != NULL) {
      if(GetBoolParameter(BP_SHOW_SLIDER)) {
        gtk_widget_show(GTK_WIDGET(m_pSpeedFrame));

        gtk_range_set_value(GTK_RANGE(m_pSpeedHScale), GetLongParameter(LP_MAX_BITRATE) / 100.0);
      }
      else {
        gtk_widget_hide(GTK_WIDGET(m_pSpeedFrame));
      }
    }
    break;
  case SP_ALPHABET_ID:
    PopulateAlphabetCombol();
    break;
  case BP_GLOBAL_KEYBOARD:
    if(m_pKeyboardHelper)
      m_pKeyboardHelper->Grab(GetBoolParameter(BP_GLOBAL_KEYBOARD));
    break;
  }

  // Emit a dasher_changed signal to notify the application about changes.
  g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_changed", iParameter);
}

void CDasherControl::PopulateAlphabetCombol() {
  for( int i(m_iComboCount - 1); i >=0; --i)
    gtk_combo_box_remove_text(GTK_COMBO_BOX(m_pCombo), i);
  
  m_iComboCount = 0;
  
  const char *szValue;
  
  szValue = GetStringParameter(SP_ALPHABET_ID).c_str();
  if(strlen(szValue) > 0) {
    gtk_combo_box_append_text(GTK_COMBO_BOX(m_pCombo), szValue);
    ++m_iComboCount;
  }
  
  szValue = GetStringParameter(SP_ALPHABET_1).c_str();
  if(strlen(szValue) > 0) {
    gtk_combo_box_append_text(GTK_COMBO_BOX(m_pCombo), szValue);
    ++m_iComboCount;
  }
  
  szValue = GetStringParameter(SP_ALPHABET_2).c_str();
  if(strlen(szValue) > 0) {
    gtk_combo_box_append_text(GTK_COMBO_BOX(m_pCombo), szValue);
    ++m_iComboCount;
  }
  
  szValue = GetStringParameter(SP_ALPHABET_3).c_str();
  if(strlen(szValue) > 0) {
    gtk_combo_box_append_text(GTK_COMBO_BOX(m_pCombo), szValue);
    ++m_iComboCount;
  }
  
  szValue = GetStringParameter(SP_ALPHABET_4).c_str();
  if(strlen(szValue) > 0) {
    gtk_combo_box_append_text(GTK_COMBO_BOX(m_pCombo), szValue);
    ++m_iComboCount;
  }
  
  gtk_combo_box_append_text(GTK_COMBO_BOX(m_pCombo), "More Alphabets...");
  ++m_iComboCount;

  gtk_combo_box_set_active(GTK_COMBO_BOX(m_pCombo), 0);
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
  else if(pEvent->m_iEventType == 3) {
    CEditContextEvent *pEditContextEvent(static_cast < CEditContextEvent * >(pEvent));
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_context_request", pEditContextEvent->m_iMaxLength);
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
  else if(pEvent->m_iEventType == 7) {
    CLockEvent *pLockEvent(static_cast<CLockEvent *>(pEvent));
    DasherLockInfo sInfo;
    sInfo.szMessage = pLockEvent->m_strMessage.c_str();
    sInfo.bLock = pLockEvent->m_bLock;
    sInfo.iPercent = pLockEvent->m_iPercent;

    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_lock_info", &sInfo);
  }
  else if(pEvent->m_iEventType == 8) {
    CMessageEvent *pMessageEvent(static_cast<CMessageEvent *>(pEvent));
    DasherMessageInfo sInfo;
    sInfo.szMessage = pMessageEvent->m_strMessage.c_str();
    sInfo.iID = pMessageEvent->m_iID;
    sInfo.iType = pMessageEvent->m_iType;

    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_message", &sInfo);
  }
};

int CDasherControl::TimerEvent() {
  int x, y;

  gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);
  m_pMouseInput->SetCoordinates(x, y);

  NewFrame(get_time());

  // Update our UserLog object about the current mouse position
  CUserLog* pUserLog = GetUserLogPtr();
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

      pUserLog->AddMouseLocationNormalized(iMouseX, iMouseY, true, GetNats());
  }

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

  if(event->type == GDK_BUTTON_PRESS)
    KeyDown(get_time(), 100);
  else if(event->type == GDK_BUTTON_RELEASE)
    KeyUp(get_time(), 100);

  return false;
}

gint CDasherControl::KeyReleaseEvent(GdkEventKey *event) {


  if((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R)) {
    if(event->state & GDK_CONTROL_MASK)
      SetLongParameter(LP_BOOSTFACTOR, 25);
    else
      SetLongParameter(LP_BOOSTFACTOR, 100);
  }
  else if((event->keyval == GDK_Control_L) || (event->keyval == GDK_Control_R)) {
    if(event->state & GDK_SHIFT_MASK)
      SetLongParameter(LP_BOOSTFACTOR, 175);
    else
      SetLongParameter(LP_BOOSTFACTOR, 100);
  }
  else {
    int iKeyVal;

    if(m_pKeyboardHelper) {
      int iKeyVal(m_pKeyboardHelper->ConvertKeycode(event->keyval));
      
      if(iKeyVal != -1)
	KeyUp(get_time(), iKeyVal);
    }
  }

  return 0;
}

gint CDasherControl::KeyPressEvent(GdkEventKey *event) {
  if((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R))
    SetLongParameter(LP_BOOSTFACTOR, 175);
  else if((event->keyval == GDK_Control_L) || (event->keyval == GDK_Control_R))
    SetLongParameter(LP_BOOSTFACTOR, 25);
  else {
    int iKeyVal;

    if(m_pKeyboardHelper) {
      int iKeyVal(m_pKeyboardHelper->ConvertKeycode(event->keyval));
      
      if(iKeyVal != -1)
	KeyDown(get_time(), iKeyVal);
    }
  }

//   switch (event->keyval) {
//   case GDK_space:
//     KeyDown(get_time(), 0);
//     break;
//   case GDK_Shift_L:
//   case GDK_Shift_R: //deliberate fall through
//     SetLongParameter(LP_BOOSTFACTOR, 175);
//     break;
//   case GDK_Control_L:
//   case GDK_Control_R: //deliberate fall through
//     SetLongParameter(LP_BOOSTFACTOR, 25);
//     break;
//   case GDK_a:
//     KeyDown(get_time(), 1);
//     break;
//   case GDK_s:
//     KeyDown(get_time(), 2);
//     break;
//   case GDK_w:
//     KeyDown(get_time(), 3);
//     break;
//   case GDK_x:
//     KeyDown(get_time(), 4);
//     break;
//   }
  return 0;
}

void CDasherControl::SliderEvent() {
  //  int iNewValue( static_cast<int>(round(gtk_range_get_value(GTK_RANGE(m_pSpeedHScale)) * 100)));
  int iNewValue( static_cast<int>(round(gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_pSpin)) * 100)));

  if(GetLongParameter(LP_MAX_BITRATE) != iNewValue)
    SetLongParameter(LP_MAX_BITRATE, iNewValue);
}

void CDasherControl::CanvasDestroyEvent() {
  // Delete the screen

  if(m_pScreen != NULL) {
    delete m_pScreen;
    m_pScreen = NULL;
  }
}

void CDasherControl::AlphabetComboChanged() {
  // std::cout << "Alphabet changed to: " << gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pCombo)) << std::endl;

  if(!strcmp("More Alphabets...",  gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pCombo)))) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_pCombo), 0);
    
    g_signal_emit_by_name(GTK_OBJECT(m_pDasherControl), "dasher_request_settings");
  }
  else if(strcmp(GetStringParameter(SP_ALPHABET_ID).c_str(),  gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pCombo))))
    SetStringParameter(SP_ALPHABET_ID, gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pCombo)));
}

// Tell the logging object that a new user trial is starting.
void CDasherControl::UserLogNewTrial()
{
  CUserLog* pUserLog = GetUserLogPtr();
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

extern "C" void alphabet_combo_changed(GtkWidget *pWidget, gpointer pUserData) {
  static_cast<CDasherControl*>(pUserData)->AlphabetComboChanged();
}
