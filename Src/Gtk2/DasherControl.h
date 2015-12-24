#ifndef __dashercontrol_h__
#define __dashercontrol_h__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Canvas.h"
#include "../DasherCore/SocketInput.h"

#ifdef JOYSTICK
#include "joystick_input.h"
#endif

#ifdef TILT
#include "tilt_input.h"
#endif

#ifdef WITH_SPEECH
#include "Speech.h"
#endif

#include "mouse_input.h"

#include "GtkDasherControl.h"
//#include "KeyboardHelper.h"

//#include "../DasherCore/DasherSettingsInterface.h"
#include "../DasherCore/DashIntfScreenMsgs.h"
#include "../DasherCore/UserLog.h"
#include "FileUtils.h"

///
/// \brief C++ core of the Dasher GTK UI component.
///
/// Class representing the Dasher UI component (ie the canvas and speed slider)
///

class CDasherControl : public CDashIntfScreenMsgs {

public:

  ///
  /// \param pVBox GTK VBox to populate with the DasherControl
  /// component widgets. This needs to be created externally by the
  /// GObject wrapper and passed to the C++ class rather than being
  /// created internally.
  /// \param pDasherControl Pointer to the GObject wrapper. This is
  /// needed so that we can emit signals from the GObject.
  ///
  // The CDasherControl object takes ownership of 'settings'.
  CDasherControl(GtkVBox * pVbox, GtkDasherControl * pDasherControl,
                 CSettingsStore* settings);
  ~CDasherControl();

  // Event handlers
  // FIXME - we should probably pass all parameters to these from the "C" callbacks
  void SetFocus();


  ///
  /// GTK Signal handler for the canvas getting the focus (which it gives away to the edit box)
  ///

  bool FocusEvent(GtkWidget *pWidget, GdkEventFocus *pEvent);


  ///
  /// Called when the canvas gets realized (ie when internal resources have been allocated), so we can finalise setup.
  ///

  void RealizeCanvas(GtkWidget *pWidget);

  ///
  /// Called periodically by a timer. Used to prompt the interface to perform a redraw
  /// \todo There's rather a lot which happens in this
  /// function. Ideally it should just be a simple call to the core
  /// which then figures out whether we're paused or not etc.
  ///

  int TimerEvent();
  int LongTimerEvent();


  ///
  /// Mouse button pressed on the canvas
  ///

  gboolean ButtonPressEvent(GdkEventButton * event);

  ///
  /// Called when the canvas has been resized, prompts the (re)creation of the CCanvas object.
  ///

  int CanvasConfigureEvent();

  ///
  /// Speed slider has been moved.
  ///

  void SliderEvent();

  ///
  /// Called when the canvas GTK widget is destroyed, so we can free any references to it.
  ///

  void CanvasDestroyEvent();

  ///
  /// Key press event on the canvas
  ///

  gint KeyReleaseEvent(GdkEventKey * event);
  gint KeyPressEvent(GdkEventKey * event);

  ///
  /// Return an array of allowed values for a string parameter.
  /// \param iParameter The parameter to query.
  /// \return A GArray of gchar* pointers to strings containing permitted values
  ///

  GArray *GetAllowedValues(int iParameter);

  ///
  /// Called by UI needs to signal a new user trial is starting.
  ///
  void UserLogNewTrial();

  void ExternalKeyDown(int iKeyVal);
  void ExternalKeyUp(int iKeyVal);

  gboolean ExposeEvent();

  ///Override to broadcast dasher_stop signal...
  void Done() override;

  void ClearAllContext() override ;
  std::string GetAllContext() override;
  int GetAllContextLenght() override;
  std::string GetTextAroundCursor(CControlManager::EditDistance dist) override;
  std::string GetContext(unsigned int iStart, unsigned int iLength) override;

  bool SupportsClipboard() override;
  void CopyToClipboard(const std::string &strText) override;

  unsigned int ctrlMove(bool bForwards, CControlManager::EditDistance dist) override;
  unsigned int ctrlDelete(bool bForwards, CControlManager::EditDistance dist) override;
#ifdef WITH_SPEECH
  ///override default non-implementation if compiling with speech...
  bool SupportsSpeech() override;
  void Speak(const std::string &strText, bool bInterrupt) override;
#endif

  ///
  /// Pass events coming from the core to the appropriate handler.
  ///
  void HandleEvent(int iParameter) override;

  ///Override to emit Gtk2 signal
  void editOutput(const std::string &strText, CDasherNode *pNode) override;
  void editDelete(const std::string &strText, CDasherNode *pNode) override;
  void editConvert(CDasherNode *pNode) override;
  void editProtect(CDasherNode *pNode) override;

  ///Override to emit Gtk2 signal
  void SetLockStatus(const string &strText, int iPercent) override;

  CGameModule *CreateGameModule() override;
private:
  virtual void CreateModules() override;

  GtkWidget *m_pVBox;
  GtkWidget *m_pCanvas;

  ///
  /// Abstracted input device object for mouse input.
  ///

  CDasherMouseInput *m_pMouseInput;
  CDasher1DMouseInput *m_p1DMouseInput;

  ///
  /// The CCanvas object
  ///

  CCanvas *m_pScreen;

  ///
  /// The GObject which is wrapping this class
  ///

  GtkDasherControl *m_pDasherControl;

  //full path of user data directory, including trailing /
  const char *m_user_data_dir;

  ///
  /// Keyboard helper class
  ///

  //  CKeyboardHelper *m_pKeyboardHelper;

  //Cache the clipboard object...
  GtkClipboard *pClipboard;
  FileUtils file_utils_;
#ifdef WITH_SPEECH
  CSpeech m_Speech;
#endif
};

#endif
