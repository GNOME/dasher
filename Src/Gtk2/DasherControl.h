#ifndef __dashercontrol_h__
#define __dashercontrol_h__

#include "PangoCache.h"
#include "Canvas.h"
#include "../DasherCore/SocketInput.h"
#include "mouse_input.h"

#include "GtkDasherControl.h"
#include "KeyboardHelper.h"

#include "../DasherCore/DasherSettingsInterface.h"
#include "../DasherCore/DashEdit.h"
#include "../DasherCore/DasherInterfaceBase.h"
#include "../DasherCore/GnomeSettingsStore.h"
#include "../DasherCore/UserLog.h"

///
/// \brief C++ core of the Dasher GTK UI component.
///
/// Class representing the Dasher UI component (ie the canvas and speed slider)
/// \todo It would really be more sensible for CDasherControl to inheret from CDasherInterface rather than contain a pointer to it
///

class CDasherControl : public CDasherInterfaceBase {

public:

  ///
  /// \param pVBox GTK VBox to populate with the DasherControl
  /// component widgets. This needs to be created externally by the
  /// GObject wrapper and passed to the C++ class rather than being
  /// created internally.  
  /// \param pDasherControl Pointer to the GObject wrapper. This is
  /// needed so that we can emit signals from the GObject.
  ///

  CDasherControl(GtkVBox * pVbox, GtkDasherControl * pDasherControl);
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

  void RealizeCanvas();

  ///
  /// Called periodically by a timer. Used to prompt the interface to perform a redraw
  /// \todo There's rather a lot which happens in this
  /// function. Ideally it should just be a simple call to the core
  /// which then figures out whether we're paused or not etc.
  ///

  int TimerEvent();

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
  /// Key press event on the canvas or the speed slider. This is
  /// currently a single handler for the two objects, but it may be
  /// sensible to at least partially split it in the future.
  /// \todo There's quite a bit of functionality which used to be here in 3.2
  /// but which has now been removed. Some of this is obsolete button Dasher
  /// code, but some of it performs useful functions such as recalibration,
  /// so this needs to be re-implemented.
  ///
  
 gint KeyReleaseEvent(GdkEventKey * event);
 gint KeyPressEvent(GdkEventKey * event);


  /// 
  /// \todo Pointless one-line function, bring in to scan_alphabet_files.
  ///

  int alphabet_filter(const gchar * filename, GPatternSpec * alphabet);

  ///
  /// \todo Pointless one-line function, bring in to scan_colour_files.
  ///

  int colour_filter(const gchar * filename, GPatternSpec * colourglob);

  ///
  /// Return an array of allowed values for a string parameter.
  /// \param iParameter The parameter to query.
  /// \return A GArray of gchar* pointers to strings containing permitted values
  ///

  GArray *GetAllowedValues(int iParameter);

  /// 
  /// Load a file and use to train the language model
  /// \param filename The file to load.
  ///

  void Train(const gchar * filename) {
    // TODO: Shouldn't call this directly - need a wrapper function which stats for filesize etc.
    // Better - allow user to pass a vector of filenames
    TrainFile(filename,0,0);
  };

  ///
  /// Called by UI needs to signal a new user trial is starting.
  ///
  void UserLogNewTrial();

  void WriteTrainFile(const std::string &strNewText);

  void AlphabetComboChanged();
  void PopulateAlphabetCombol();

private:
  //  virtual void CreateSettingsStore();
  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void SetupUI();
  virtual void CreateSettingsStore();
  virtual void HandleEvent(CEvent * pEvent);
  virtual int GetFileSize(const std::string &strFileName);

  ///
  /// Pass events coming from the core to the appropriate handler.
  ///

  void ExternalEventHandler(Dasher::CEvent *pEvent);

  ///
  /// Notification from CDasherInterface that a parameter has changed
  /// \param iParameter The parameter which has changed
  ///

  void HandleParameterNotification(int iParameter);

  GtkWidget *m_pVBox;
  GtkWidget *m_pCanvas;
  GtkWidget *m_pSpeedHScale;
  GtkWidget *m_pSpeedFrame;

  GtkWidget *m_pStatusBar;

  GtkWidget *m_pSpin;
  GtkWidget *m_pCombo;
  GtkWidget *m_pStatusLabel;


  ///
  /// Abstracted input device object for mouse input.
  ///

  CDasherMouseInput *m_pMouseInput;

  ///
  /// Abstracted input device object for TCP/IP socket input.
  ///

  CSocketInput *m_pSocketInput;

  ///
  /// Cache of Pango layouts
  ///

  CPangoCache *m_pPangoCache;

  ///
  /// The CCanvas object
  ///

  CCanvas *m_pScreen;

  ///
  /// The GObject which is wrapping this class
  ///

  GtkDasherControl *m_pDasherControl;

  ///
  /// Keyboard helper class
  ///

  CKeyboardHelper *m_pKeyboardHelper;

  int m_iComboCount;

};

#endif
