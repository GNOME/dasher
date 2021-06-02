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
#include "dasher_main.h"
#include "../DasherCore/GameModule.h"
#include "../Common/Globber.cpp"

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
extern "C" gint canvas_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data);

static bool g_iTimeoutID = 0;

// CDasherControl class definitions
CDasherControl::CDasherControl(GtkVBox *pVBox, GtkDasherControl *pDasherControl,
                               CSettingsStore* settings)
 : CDashIntfScreenMsgs(settings, &file_utils_) {
  m_pScreen = NULL;

  m_pDasherControl = pDasherControl;
  m_pVBox = GTK_WIDGET(pVBox);
  pClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  m_pCanvas = gtk_drawing_area_new();
  gtk_widget_set_can_focus(m_pCanvas, TRUE);
  gtk_widget_set_double_buffered(m_pCanvas, FALSE);

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
  g_signal_connect(m_pCanvas, "draw", G_CALLBACK(canvas_draw_event), this);

  char *home_dir = getenv("HOME");
  char *user_data_dir = new char[strlen(home_dir) + 10];
  sprintf(user_data_dir, "%s/.dasher/", home_dir);
  m_user_data_dir = user_data_dir;

  m_pScreen = new CCanvas(m_pCanvas);
  ChangeScreen(m_pScreen);

  //This was done in old SetupUI, i.e. the first thing in Realize().
  // TODO: Use system defaults?
  if(GetStringParameter(SP_DASHER_FONT) == "")
    SetStringParameter(SP_DASHER_FONT, "Sans 10");
  else
    m_pScreen->SetFont(GetStringParameter(SP_DASHER_FONT));
  Realize(get_time());
 
  //  m_pKeyboardHelper = new CKeyboardHelper(this);
  //  m_pKeyboardHelper->Grab(GetBoolParameter(BP_GLOBAL_KEYBOARD));
}

void CDasherControl::CreateModules() {
  CDasherInterfaceBase::CreateModules(); //create default set first
  // Create locally cached copies of the mouse input objects, as we
  // need to pass coordinates to them from the timer callback
  m_pMouseInput =
    (CDasherMouseInput *)  RegisterModule(new CDasherMouseInput());
  SetDefaultInputDevice(m_pMouseInput);
  m_p1DMouseInput =
    (CDasher1DMouseInput *)RegisterModule(new CDasher1DMouseInput());
  RegisterModule(new CSocketInput(this, this));

#ifdef JOYSTICK
  RegisterModule(new CDasherJoystickInput(this));
  RegisterModule(new CDasherJoystickInputDiscrete(this));
#endif
  
#ifdef TILT
  RegisterModule(new CDasherTiltInput(this));
#endif
}


void CDasherControl::ClearAllContext() {
  gtk_dasher_control_clear_all_context(m_pDasherControl);
  //SetBuffer(0); //the editor's clear method emits a "buffer_changed" signal,
                  //which does this for us automatically.
}

std::string CDasherControl::GetAllContext() {
  return gtk_dasher_control_get_all_text(m_pDasherControl);
}

int CDasherControl::GetAllContextLenght()
{
  auto text = gtk_dasher_control_get_all_text(m_pDasherControl);
  return g_utf8_strlen(text.c_str(),-1);
}

std::string CDasherControl::GetTextAroundCursor(CControlManager::EditDistance dist) {
  return gtk_dasher_control_get_text_around_cursor(m_pDasherControl, dist);
}

std::string CDasherControl::GetContext(unsigned int iStart, unsigned int iLength) {
  return gtk_dasher_control_get_context(m_pDasherControl, iStart, iLength);
}

bool CDasherControl::SupportsClipboard() {
  return true;
}

void CDasherControl::CopyToClipboard(const std::string &strText) {
  const gchar *the_text(strText.c_str());
  gtk_clipboard_set_text(pClipboard, the_text, strlen(the_text));
}

#ifdef WITH_SPEECH
bool CDasherControl::SupportsSpeech() {
  return m_Speech.Init();
}

void CDasherControl::Speak(const std::string &strText, bool bInterrupt) {
  string lang = GetActiveAlphabet()->GetLanguageCode();
  m_Speech.Speak(strText, bInterrupt, lang);
}
#endif

CDasherControl::~CDasherControl() {
  if(m_pMouseInput) {
    m_pMouseInput = NULL;
  }

  if(m_p1DMouseInput) {
    m_p1DMouseInput = NULL;
  }

  delete[] m_user_data_dir;

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

    g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "focus_in_event", GTK_WIDGET(m_pDasherControl), focusEvent, NULL, &returnType);
  }
  return true;
}

void CDasherControl::SetFocus() {
  gtk_widget_grab_focus(m_pCanvas);
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
  // Start the timer loops as everything is set up.
  // Aim for 40 frames per second, computers are getting faster.

  if(g_iTimeoutID == 0) {
    g_iTimeoutID = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 25, timer_callback, this, NULL);
    // TODO: Reimplement this (or at least reimplement some kind of status reporting)
    //g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 5000, long_timer_callback, this, NULL);
  }
}

int CDasherControl::CanvasConfigureEvent() {
  GtkAllocation a;

  gtk_widget_get_allocation(m_pCanvas, &a);

  m_pScreen->resize(a.width,a.height);
  ScreenResized(m_pScreen);
 
  return 0;
}

void CDasherControl::HandleEvent(int iParameter) {
  CDashIntfScreenMsgs::HandleEvent(iParameter);
  switch(iParameter) {
  case SP_DASHER_FONT:
      m_pScreen->SetFont(GetStringParameter(SP_DASHER_FONT));
      ScheduleRedraw();
    break;
  case BP_GLOBAL_KEYBOARD:
    // TODO: reimplement
//     if(m_pKeyboardHelper)
//       m_pKeyboardHelper->Grab(GetBoolParameter(BP_GLOBAL_KEYBOARD));
    break;
  }
  // Convert events coming from the core to Glib signals.
  g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_changed", iParameter);
}

void CDasherControl::editOutput(const std::string &strText, CDasherNode *pNode) {
  if (!GetGameModule()) //GameModule sets editbox directly
    g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_edit_insert", strText.c_str(), pNode->offset());
  CDasherInterfaceBase::editOutput(strText, pNode);
}

void CDasherControl::editDelete(const std::string &strText, CDasherNode *pNode) {
  if (!GetGameModule()) //GameModule sets editbox directly
    g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_edit_delete", strText.c_str(), pNode->offset());
  CDasherInterfaceBase::editDelete(strText, pNode);
}

void CDasherControl::editConvert(CDasherNode *pNode) {
  g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_edit_convert");
  CDasherInterfaceBase::editConvert(pNode);
}

void CDasherControl::editProtect(CDasherNode *pNode) {
  g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_edit_protect");
  CDasherInterfaceBase::editProtect(pNode);
}

void CDasherControl::SetLockStatus(const string &strText, int iPercent) {
    DasherLockInfo sInfo;
    sInfo.szMessage = strText.c_str();
    sInfo.iPercent = iPercent;
    sInfo.time = get_time();

    //Uniquely, the call to gtk to handle events and update the progress
    // dialogue, also renders the canvas. So let's have a message there too...
    CDasherInterfaceBase::SetLockStatus(strText,iPercent);
    g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_lock_info", &sInfo);
}

//TODO do we want to do something like this?
// ATM the only message is actually from the auto-speed control,
// so definitely _doesn't_ want to be modal; could introduce a boolean
// 'ok to interrupt user?' param to Message()?
//void CDasherControl::Message(const std::string &strText)
//  GtkMessageDialog *pDialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, strText.c_str()));
//  gtk_dialog_run(GTK_DIALOG(pDialog));
//  gtk_widget_destroy(GTK_WIDGET(pDialog));
//}

unsigned int CDasherControl::ctrlMove(bool bForwards, CControlManager::EditDistance dist) {
  return gtk_dasher_control_ctrl_move(m_pDasherControl,bForwards,dist);
}

unsigned int CDasherControl::ctrlDelete(bool bForwards, CControlManager::EditDistance dist) {
  return gtk_dasher_control_ctrl_delete(m_pDasherControl,bForwards,dist);
}

class GtkGameModule : public CGameModule {
public:
  GtkGameModule(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CDasherView *pView, CDasherModel *pModel, GtkTextBuffer *pBuffer)
  : CGameModule(pCreator, pInterface, pView, pModel), m_pBuffer(pBuffer) {
    m_tEntered = gtk_text_buffer_create_tag(m_pBuffer, "entered", "foreground", "#00FF00", NULL);
    m_tWrong = gtk_text_buffer_create_tag(m_pBuffer, "wrong", "foreground", "#FF0000", "strikethrough", TRUE, NULL);
    GtkTextIter start,end;
    gtk_text_buffer_get_start_iter(m_pBuffer, &start);
    m_mEntered = gtk_text_buffer_create_mark(m_pBuffer, NULL, &start, true);
    gtk_text_buffer_get_end_iter(m_pBuffer, &end);
    m_mTarget = gtk_text_buffer_create_mark(m_pBuffer, NULL, &end, false);
    gtk_text_buffer_delete(m_pBuffer, &start, &end);
  }

  ~GtkGameModule() {
    GtkTextTagTable *table = gtk_text_buffer_get_tag_table(m_pBuffer);
    gtk_text_tag_table_remove(table, m_tEntered);
    gtk_text_tag_table_remove(table, m_tWrong);

    gtk_text_buffer_delete_mark(m_pBuffer,m_mEntered);
    gtk_text_buffer_delete_mark(m_pBuffer,m_mTarget);
  }

  void ChunkGenerated() {
    string sText;
    for (vector<symbol>::const_iterator it=targetSyms().begin(); it!=targetSyms().end(); it++)
      sText += m_pAlph->GetText(*it);
    gtk_text_buffer_set_text(m_pBuffer, sText.c_str(), -1); //-1 for length = null-terminated
    GtkTextIter start,end;
    gtk_text_buffer_get_start_iter(m_pBuffer, &start);
    gtk_text_buffer_move_mark(m_pBuffer, m_mEntered, &start);
    gtk_text_buffer_move_mark(m_pBuffer, m_mTarget, &start);
    gtk_text_buffer_get_end_iter(m_pBuffer, &end);
    gtk_text_buffer_remove_all_tags(m_pBuffer, &start, &end);
  }
  void HandleEvent(const Dasher::CEditEvent *pEvt) {
    const int iPrev(lastCorrectSym());
    CGameModule::HandleEvent(pEvt);
    if (iPrev==lastCorrectSym()) {
      GtkTextIter start,end; //of "wrong" text
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &start, m_mEntered);
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &end, m_mTarget);
      gtk_text_buffer_delete(m_pBuffer, &start, &end); //invalidates end, brings m_mEntered & m_mTarget together
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &start, m_mEntered);
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &end, m_mTarget);
      gtk_text_buffer_insert(m_pBuffer, &start, m_strWrong.c_str(), -1); //moves m_mEntered & m_mTarget apart
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &start, m_mEntered);
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &end, m_mTarget);
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &end, m_mTarget);
      gtk_text_buffer_apply_tag(m_pBuffer, m_tWrong, &start, &end);
    } else {
      GtkTextIter it,it2;
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &it, m_mEntered);
      gtk_text_buffer_get_iter_at_mark(m_pBuffer, &it2, m_mTarget);
      DASHER_ASSERT(gtk_text_iter_get_offset(&it) == gtk_text_iter_get_offset(&it2));
      if (iPrev < lastCorrectSym()) {
        //correct text added
        DASHER_ASSERT(iPrev == lastCorrectSym()-1);
        gtk_text_iter_forward_chars(&it2, 1);
        gtk_text_buffer_apply_tag(m_pBuffer, m_tEntered, &it, &it2);
      } else {
        //correct text erased!
        DASHER_ASSERT(iPrev == lastCorrectSym()+1);
        gtk_text_iter_backward_chars(&it2, 1);
        gtk_text_buffer_remove_tag(m_pBuffer, m_tEntered, &it2, &it);
      }
      gtk_text_buffer_move_mark(m_pBuffer, m_mEntered, &it2);
      gtk_text_buffer_move_mark(m_pBuffer, m_mTarget, &it2);
    } 
  }
  void DrawText(CDasherView *pView) {}
private:
  GtkTextBuffer *m_pBuffer;
  GtkTextTag *m_tEntered, *m_tWrong;
  GtkTextMark *m_mEntered; //just after what's been correctly entered
  GtkTextMark *m_mTarget; //after any "wrong" text, before target; if no wrong chars, ==m_entered.
};

CGameModule *CDasherControl::CreateGameModule() {
  if (GtkTextBuffer *buf=gtk_dasher_control_game_text_buffer(m_pDasherControl))
    return new GtkGameModule(this, this, GetView(), m_pDasherModel, buf);
  return CDashIntfScreenMsgs::CreateGameModule();
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

int CDasherControl::TimerEvent() {
  int x, y;
  GdkWindow *default_root_window = gdk_get_default_root_window();
  GdkWindow *window = gtk_widget_get_window(m_pCanvas);

  GdkDeviceManager *device_manager =
    gdk_display_get_device_manager(gdk_window_get_display(window));
  GdkDevice *pointer = gdk_device_manager_get_client_pointer(device_manager);

  gdk_window_get_device_position(window, pointer, &x, &y, NULL);
  m_pMouseInput->SetCoordinates(x, y);

  gdk_window_get_device_position(default_root_window, pointer, &x, &y, NULL);

  int iRootWidth;
  int iRootHeight;

  iRootWidth  = gdk_window_get_width (default_root_window);
  iRootHeight = gdk_window_get_height(default_root_window);

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

      gdk_window_get_frame_extents(window, &sWindowRect);

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
      gdk_window_get_device_position(NULL, pointer, &iMouseX, &iMouseY, NULL);

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
  return FALSE;
}

void CDasherControl::Done() {
  CDasherInterfaceBase::Done();
  g_signal_emit_by_name(GTK_WIDGET(m_pDasherControl), "dasher_stop");
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
//   g_signal_emit_by_name(GTK_WIDGET(m_pCanvas), "focus_in_event", GTK_WIDGET(m_pCanvas), focusEvent, NULL, &returnType);

  // No - don't take the focus - give it to the text area instead
  
  //GDK uses button 1=left, 2=middle, 3=right. We want 100, 102, 101
  int button = event->button;
  if (button&2) button^=1;
  if(event->type == GDK_BUTTON_PRESS)
    KeyDown(get_time(), button+99 );
  else if(event->type == GDK_BUTTON_RELEASE)
    KeyUp(get_time(), button+99);

  return false;
}

gint CDasherControl::KeyReleaseEvent(GdkEventKey *event) {
  // TODO: This is seriously flawed - the semantics of of X11 Keyboard
  // events mean the there's no guarantee that key up/down events will
  // be received in pairs.

//  if((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R)) {
//     if(event->state & GDK_CONTROL_MASK)
//       SetLongParameter(LP_BOOSTFACTOR, 25);
//     else
//       SetLongParameter(LP_BOOSTFACTOR, 100);
//  }
//  else if((event->keyval == GDK_Control_L) || (event->keyval == GDK_Control_R)) {
//     if(event->state & GDK_SHIFT_MASK)
//       SetLongParameter(LP_BOOSTFACTOR, 175);
//     else
//       SetLongParameter(LP_BOOSTFACTOR, 100);
//  }
//  else {
//     if(m_pKeyboardHelper) {
//       int iKeyVal(m_pKeyboardHelper->ConvertKeycode(event->keyval));
      
//       if(iKeyVal != -1)
// 	KeyUp(get_time(), iKeyVal);
//     }
//  }

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

extern "C" gint canvas_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data) {
  return ((CDasherControl*)data)->ExposeEvent();
}
