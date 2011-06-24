// DasherInterfaceBase.h
//
// Copyright (c) 2008 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __DasherInterfaceBase_h__
#define __DasherInterfaceBase_h__

///
/// \mainpage
///
/// This is the Dasher source code documentation. Please try to keep
/// it up to date!
///

// TODO - there is a list of things to be configurable in my notes
// Check that everything that is not self-contained within the GUI is covered.

#include "Messages.h"
#include "../Common/NoClones.h"
#include "../Common/ModuleSettings.h"
#include "ActionButton.h"
#include "Alphabet/AlphIO.h"
#include "AutoSpeedControl.h"
#include "ColourIO.h"
#include "InputFilter.h"
#include "ModuleManager.h"
#include "ControlManager.h"

#include <set>
#include <algorithm>

namespace Dasher {
  class CDasherScreen;
  class CDasherView;
  class CDasherInput;
  class CInputFilter;
  class CDasherModel;
  class CSettingsStore;
  class CGameModule;
  class CDasherInterfaceBase;
}

class CUserLogBase;
class CNodeCreationManager;

/// \defgroup Core Core Dasher classes
/// @{


/// The central class in the core of Dasher. Ties together the rest of
/// the platform independent stuff and provides a single interface for
/// the UI to use. Note: CMessageDisplay unimplemented; platforms should
/// provide their own methods using appropriate GUI components, or subclass
/// CDashIntfScreenMsgs instead.
class Dasher::CDasherInterfaceBase : public CMessageDisplay, public Observable<const CEditEvent *>, protected Observer<int>, protected CSettingsUser, private NoClones {
public:
  ///Create a new interface by providing the only-and-only settings store that will be used throughout.
  CDasherInterfaceBase(CSettingsStore *pSettingsStore);
  virtual ~CDasherInterfaceBase();

  /// @name Access to internal member classes
  /// Access various classes contained within the interface. These
  /// should be considered dangerous and use minimised. Eventually to
  /// be replaced by properly encapsulated equivalents.
  /// @{

  CUserLogBase* GetUserLogPtr();

  // @}

  ///
  /// @name Parameter manipulation
  /// Members for manipulating the parameters of the core Dasher object.
  ///

  //@{

  ///
  /// Reset a parameter to the default value
  ///

  void ResetParameter(int iParmater);

  ///
  /// Obtain the permitted values for a string parameter - used to
  /// geneate preferences dialogues etc.
  ///

  void GetPermittedValues(int iParameter, std::vector<std::string> &vList);

  ///
  /// Get a list of settings which apply to a particular module
  ///

  bool GetModuleSettings(const std::string &strName, SModuleSettings **pSettings, int *iCount);

  //@}

  /// Called when a parameter changes - but *after* components have been notified.
  /// Subsumes previous Interface level event handler, for example, responsible for
  /// restarting the Dasher model whenever parameter changes make it
  /// invalid. Subclasses should override to forward events to SettingsUI, editbox,
  /// etc., as appropriate, but should _call_through_to_superclass_method_ first.
  /// \param iParameter The parameter that's just changed.
  /// \todo Should be protected (??)

  virtual void HandleEvent(int iParameter);
  
  void PreSetNotify(int iParameter, const std::string &sValue);

  ///Locks/unlocks Dasher. The default here stores the lock message and percentage
  /// in m_strLockMessage, such that NewFrame renders this instead of the canvas
  /// if we are locked. Subclasses may override to implement better (GUI)
  /// notifications/dialogues, but should call through to this method to ensure
  /// isLocked() returns the correct value.
  /// Note that we do not support multiple/concurrent locks; each call to SetLockStatus
  /// overrides any/all previous ones.
  /// \param strText text of message to display, excluding %age, _if_ locked;
  ///  ignored, if unlocked.
  /// \param iPercent -1 unlocks Dasher; anything else locks it, and indicates
  ///  %progress.
  virtual void SetLockStatus(const std::string &strText, int iPercent);

  /// Tells us whether Dasher is locked (i.e. for training).
  /// TODO This just replaces the old BP_TRAINING; however, I'd think that _if_ we
  /// do actually need a global function to tell whether Dasher's locked, it probably
  /// needs to be threadsafe, which neither this nor BP_TRAINING is (I don't think!)...
  inline bool isLocked() {return !m_strLockMessage.empty();}

  ///Does this subclass support speech (i.e. the speak(string) method?)
  /// Default is just to return false.
  virtual bool SupportsSpeech() {return false;}
  ///Does this subclass support clipboard copying (i.e. the copyToClipboard(string) method?)
  /// Default is just to return false.
  virtual bool SupportsClipboard() {return false;}

  ///Subclasses supporting speech should override to speak the supplied text
  /// (Default implementation does nothing)
  virtual void Speak(const std::string &text, bool bInterrupt) {}

  ///Subclasses supporting clipboard operations should override to copy
  /// the specified text to the clipboard. (Default implementation does nothing).
  virtual void CopyToClipboard(const std::string &text) {}

  ///Called to execute a control-mode "move" command.
  ///\param bForwards true to move forwards (right), false for backwards
  ///\param dist how far to move: character, word, line, file. (Usually defined
  /// by OS, e.g. for non-european languages)
  ///\return the offset, into the edit buffer of the cursor *after* the move.
  virtual unsigned int ctrlMove(bool bForwards, CControlManager::EditDistance dist)=0;

  ///Called to execute a control-mode "delete" command.
  ///\param bForwards true to delete forwards (right), false for backwards
  ///\param dist how much to delete: character, word, line, file. (Usually defined
  /// by OS, e.g. for non-european languages)
  ///\return the offset, into the edit buffer, of the cursor *after* the delete
  /// (for forwards deletion, this will be the same as the offset *before*)
  virtual unsigned int ctrlDelete(bool bForwards, CControlManager::EditDistance dist)=0;

  virtual void editOutput(const std::string &strText, CDasherNode *pCause);
  virtual void editDelete(const std::string &strText, CDasherNode *pCause);
  virtual void editConvert(CDasherNode *pCause);
  virtual void editProtect(CDasherNode *pCause);

  class TextAction {
  public:
    TextAction(CDasherInterfaceBase *pMgr);
    void executeOnAll();
    void executeOnNew();
    void executeLast();
    void NotifyOffset(int iOffset);
    virtual ~TextAction();
  protected:
    virtual void operator()(const std::string &strText)=0;
    CDasherInterfaceBase *m_pIntf;
  private:
    int m_iStartOffset;
    std::string strLast;
  };


  /// @name Starting and stopping
  /// Methods used to instruct dynamic motion of Dasher to start or stop
  /// @{

  /// Stop Dasher - Sets BP_DASHER_PAUSED and executes any on-stop actions
  ///  (speech, clipboard - subclasses may override to do more).
  /// (But does nothing if BP_DASHER_PAUSED is not set)
  virtual void Stop();

  /// Unpause Dasher. Clears BP_DASHER_PAUSED.
  /// (But does nothing if BP_DASHER_PAUSED is currently set).
  /// \param Time Time in ms, used to keep a constant frame rate
  virtual void Unpause(unsigned long Time);

  ///Whether any actions are currently setup to occur when Dasher 'stop's.
  /// Default is to return TRUE iff we support speech and BP_SPEAK_ON_STOP is set,
  /// and/or if we support clipboard and BP_COPY_ALL_ON_STOP is set; subclasses may
  /// override if they have additional on-stop actions.
  virtual bool hasStopTriggers();
  /// @}

  ///
  /// Append text to the user training file - used to store state between sessions
  /// \param filename name of training file, without path (e.g. "training_english_GB.txt")
  /// \param strNewText text to append
  ///

  virtual void WriteTrainFile(const std::string &filename, const std::string &strNewText) {
  };

  // App Interface
  // -----------------------------------------------------

  // std::map<int, std::string>& GetAlphabets(); // map<key, value> int is a UID string can change. Store UID in preferences. Display string to user.
  // std::vector<std::string>& GetAlphabets();
  // std::vector<std::string>& GetLangModels();
  // std::vector<std::string>& GetViews();

  /// Supply a new CDasherScreen object onto which to render. Note this should
  /// only be called (a) at startup, and (b) when the new screen is _significantly_
  /// different from the old, rather than just a window resize: specifically, this means
  /// the tree of nodes will be rebuilt with new Labels for the new screen; and in the future,
  /// maybe also if things like colour depth, alpha transparency support, etc., change.
  /// If the existing rendering setup should just scale to the new screen dimensions,
  /// call ScreenResized() instead (we expect this to be the case most/all of the time,
  /// and this method subsumes a call to ScreenResized.) Note, at startup, ChangeScreen
  /// and Realize may occur in either order; if ChangeScreen comes after, Resize will create a 
  /// tree with null Labels, which will have to be rebuilt in the call to ChangeScreen.
  /// \param NewScreen Pointer to the new CDasherScreen.
  virtual void ChangeScreen(CDasherScreen * NewScreen);
  
  ///Call when the screen dimensions have been changed, to recalculate scaling factors etc.
  /// \param pScreen the screen whose dimensions have changed. TODO we expect this to be
  /// the same one-and-only screen that we are using anyway, so remove parameter?
  void ScreenResized(CDasherScreen *pScreen);

  /// Train Dasher from a file
  /// All traing data must be in UTF-8
  /// \param Filename File to load.
  /// \param iTotalBytes documentme
  /// \param iOffset Document me
  //  int TrainFile(std::string Filename, int iTotalBytes, int iOffset);

  /// New control mechanisms:

  ///Equivalent to SetOffset(iOffset, true)
  void SetBuffer(int iOffset) {SetOffset(iOffset, true);}

  /// Tells the model to rebuild itself with the
  /// cursor at the specified offset (position within textbox/buffer).
  /// @param bForce true meaning the entire context may have changed,
  /// false if we've just moved around within it.
  void SetOffset(int iOffset, bool bForce=false);

  /// @name Status reporting
  /// Get information about the runtime status of Dasher which might
  /// be of interest for debugging purposes etc.
  /// @{

  /// Get the current rate of text entry.
  /// \retval The rate in characters per minute.
  /// TODO: Check that this is still used

  double GetCurCPM();

  /// Get current refresh rate.
  /// \retval The rate in frames per second
  /// TODO: Check that this is still used

  double GetCurFPS();

  /// Get the total number of nats (base-e bits) entered.
  /// \retval The current total
  /// \todo Obsolete since new logging code?

  double GetNats() const;

  /// Reset the count of nats entered.
  /// \todo Obsolete since new logging code?

  void ResetNats();

  /// @}

  /// @name Action buttons
  /// TODO, how much of this was ever implemented???
  /// @{
  /// ACL in answer to previous: not much, not sure whether it's worth keeping anything;
  /// but for now, I'm preserving this as overridable by subclasses which do want to handle
  /// commands, so they can make it do something. The default implementation does nothing.
  virtual void ExecuteCommand(const std::string &strName) {};

  void AddActionButton(const std::string &strName);

  /// @}

  /// @name User input
  /// Deals with forwarding user input to the core
  /// @{

  void KeyDown(int iTime, int iId, bool bPos = false, int iX = 0, int iY = 0);

  void KeyUp(int iTime, int iId, bool bPos = false, int iX = 0, int iY = 0);

  void HandleClickUp(int iTime, int iX, int iY);

  void HandleClickDown(int iTime, int iX, int iY);

  /// @}

  // Module management functions
  CDasherModule *RegisterModule(CDasherModule *pModule);
  CDasherModule *GetModule(ModuleID_t iID);
  CDasherModule *GetModuleByName(const std::string &strName);
  CDasherInput *GetActiveInputDevice() {return m_pInput;}
  CInputFilter *GetActiveInputMethod() {return m_pInputFilter;}
  const CAlphInfo *GetActiveAlphabet();
  void SetDefaultInputDevice(CDasherInput *);
  void SetDefaultInputMethod(CInputFilter *);

  void StartShutdown();

  void ScheduleRedraw() {
    m_bRedrawScheduled = true;
  };

  ///Subclasses should return the contents of (the specified subrange of) the edit buffer
  virtual std::string GetContext(unsigned int iStart, unsigned int iLength)=0;

  ///Clears all written text from edit buffer and rebuilds the model. The default
  /// implementation does this using the control mode editDelete mechanism
  /// (one call forward, one back), followed by a call to SetBuffer(0). Subclasses
  /// may (optionally) override with more efficient / easier implementations, but
  /// should make the same call to SetBuffer.
  virtual void ClearAllContext();
  virtual std::string GetAllContext()=0;

  /// Set a key value pair by name - designed to allow operation from
  /// the command line.  Returns 0 on success, an error string on failure.
  ///
  const char* ClSet(const std::string &strKey, const std::string &strValue);

  void ImportTrainingText(const std::string &strPath);

  /// Flush the/all currently-written text to the user's training file(s).
  /// Just calls through to WriteTrainFileFull(this) on the AlphabetManager;
  /// public so e.g. iPhone can flush the buffer when app is backgrounded.
  void WriteTrainFileFull();

  ///
  /// Obtain the size in bytes of a file - the way to do this is
  /// dependent on the OS (TODO: Check this - any posix on Windows?)
  ///
  virtual int GetFileSize(const std::string &strFileName) = 0;

protected:

  /// @name Startup
  /// Interaction with the derived class during core startup
  /// @{

  ///
  /// Finish initializing the DasherInterface; we can't do everything in the constructor,
  /// because some initialization depends on virtual methods provided by subclasses.
  /// Both Realize and ChangeScreen must be called after construction before other functions
  /// will work, but they can be called in either order (as the SettingsStore is passed into
  /// the c'tor).
  /// \param ulTime timestamp, much as per NewFrame, used for initializing the RNG (i.e. srand).
  /// (Is that too hacky?)
  ///
  void Realize(unsigned long ulTime);

  ///
  /// Creates a default set of modules. Override in subclasses to create any
  /// extra/different modules specific to the platform (eg input device drivers)
  ///
  virtual void CreateModules();

  /// @}

  ///Creates the game module - called on demand, i.e. (only) when game mode is started
  /// by setting BP_GAME_MODE. Subclasses must implement to return a concrete subclass
  /// of GameModule, perhaps by using platform-specific widgets (e.g. the edit box?)
  
  virtual CGameModule *CreateGameModule(CDasherView *pView, CDasherModel *pModel)=0;

  /// Draw a new Dasher frame, regardless of whether we're paused etc.
  /// \param iTime Current time in ms.
  /// \param bForceRedraw Passing in true is equivalent to calling ScheduleRedraw() first,
  /// and forces the nodes/canvas to be re-rendered (even if we haven't moved).
  void NewFrame(unsigned long iTime, bool bForceRedraw);

  ///Renders the current state of the nodes (optionally), decorations, etc. (Does not move around the nodes.)
  /// \param ulTime Time of rendering, for time-dependent decorations (e.g. messages)
  /// \param bRedrawNodes whether to re-render the nodes (expensive!)
  /// \param policy if redrawing nodes, use this to expand/collapse nodes, and set m_bLastChanged if any were.
  bool Redraw(unsigned long ulTime, bool bRedrawNodes, CExpansionPolicy &policy);

  ///Called at the end of each frame, after lock-message / nodes+decorations have been rendered.
  /// Default does nothing, but subclasses can override if they need to do anything else.
  /// \return true if anything has been rendered to the Screen such that it needs to be blitted
  /// (i.e. Display() called) - the default just returns false.
  virtual bool FinishRender(unsigned long ulTime) {return false;}

  /// @}

  CDasherScreen *m_DasherScreen;

  /// Asynchronous (non-modal) messages to be displayed to the user, longest-ago
  /// at the front, along with the timestamp of the frame at which each was first
  /// displayed to the user - 0 if not yet displayed.
  std::deque<pair<CDasherScreen::Label*, unsigned long> > m_dqAsyncMessages;
  
  /// Modal messages being or waiting to be displayed to the user, longest-ago
  /// at the front, along with the timestamp when each was first displayed to the
  /// user (0 if not yet displayed).
  std::deque<pair<CDasherScreen::Label*, unsigned long> > m_dqModalMessages;
  
 private:
  
  ///We keep a reference to the (currently unique/global) SettingsStore with which
  /// this interface was created, as ClSet and ResetParameter need to access it.
  /// (TODO _could_ move these into CSettingsUser, but that seems uglier given so few clients?)
  CSettingsStore * const m_pSettingsStore;

  //The default expansion policy to use - an amortized policy depending on the LP_NODE_BUDGET parameter.
  CExpansionPolicy *m_defaultPolicy;

  /// @name Platform dependent utility functions
  /// These functions provide various platform dependent functions
  /// required by the core. A derived class is created for each
  /// supported platform which implements these.
  // @{

  ///
  /// Initialise the SP_SYSTEM_LOC and SP_USER_LOC paths - the exact
  /// method of doing this will be OS dependent
  ///

  virtual void SetupPaths() = 0;

  ///
  /// Produce a list of filenames for alphabet files
  ///

  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList) = 0;

  ///
  /// Produce a list of filenames for colour files
  ///

  virtual void ScanColourFiles(std::vector<std::string> &vFileList) = 0;

  ///
  /// Set up the platform dependent UI for the widget (not the wider
  /// app). Note that the constructor of the derived class will
  /// probably want to return details of what was created - this will
  /// have to happen separately, but we'll need to be careful with the
  /// semantics.
  ///

  virtual void SetupUI() = 0;

  /// @}


  /// Provide a new CDasherInput input device object.

  void CreateInput();

  void CreateInputFilter();

  void CreateModel(int iOffset);
  void CreateNCManager();

  void ChangeAlphabet();
  void ChangeColours();
  void ChangeView();

  //Compute the screen orientation to use - i.e. combining the user's
  // preference with the alphabet.
  Opts::ScreenOrientations ComputeOrientation();

  void SetupActionButtons();
  void DestroyActionButtons();
  void PositionActionButtons();
  bool DrawActionButtons();

  std::vector<CActionButton *> m_vLeftButtons;
  std::vector<CActionButton *> m_vRightButtons;


  class WordSpeaker : public TransientObserver<const CEditEvent *> {
  public:
    WordSpeaker(CDasherInterfaceBase *pIntf);
    void HandleEvent(const CEditEvent *);
  private:
    ///builds up the word currently being entered
    std::string m_strCurrentWord;
  } *m_pWordSpeaker;
  
  /// @name Child components
  /// Various objects which are 'owned' by the core.
  /// @{
  CDasherModel *m_pDasherModel;
  CDasherView *m_pDasherView;
  CDasherInput *m_pInput;
  CInputFilter* m_pInputFilter;
  CModuleManager m_oModuleManager;
  CAlphIO *m_AlphIO;
  CColourIO *m_ColourIO;
  CNodeCreationManager *m_pNCManager;
  CUserLogBase *m_pUserLog;

  // the game mode module - only
  // initialized if game mode is enabled
  CGameModule *m_pGameModule;
  /// @}

  ///If non-empty, Dasher is locked, and this is the message that should be displayed.
  std::string m_strLockMessage;
  /// (Cache) renderable version of previous; created only to render
  /// (so may still be NULL even if locked)
  CDasherScreen::Label *m_pLockLabel;

  /// @name State variables
  /// Represent the current overall state of the core
  /// @{
  bool m_bRedrawScheduled;
  bool m_bOldVisible;

  /// @}

  std::set<TextAction *> m_vTextActions;
};
/// @}

#endif /* #ifndef __DasherInterfaceBase_h__ */
