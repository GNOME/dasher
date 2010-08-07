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

#include "../Common/NoClones.h"
#include "../Common/ModuleSettings.h"
#include "ActionButton.h"
#include "Alphabet/AlphIO.h"
#include "AutoSpeedControl.h"
#include "ColourIO.h"
#include "InputFilter.h"
#include "ModuleManager.h"

#include <set>
#include <algorithm>

namespace Dasher {
  class CDasherScreen;
  class CDasherView;
  class CDasherInput;
  class CInputFilter;
  class CDasherModel;
  class CEventHandler;
  class CEvent;
  
  class CDasherInterfaceBase;
}

class CSettingsStore;
class CUserLogBase;
class CNodeCreationManager;

/// \defgroup Core Core Dasher classes
/// @{


/// The central class in the core of Dasher. Ties together the rest of
/// the platform independent stuff and provides a single interface for
/// the UI to use.
class Dasher::CDasherInterfaceBase:private NoClones
{
public:
  CDasherInterfaceBase();
  virtual ~CDasherInterfaceBase();

  /// @name Access to internal member classes
  /// Access various classes contained within the interface. These
  /// should be considered dangerous and use minimised. Eventually to
  /// be replaced by properly encapsulated equivalents.
  /// @{

  ///
  /// Return a pointer to the current EventHandler (the one
  /// which the CSettingsStore is using to notify parameter
  /// changes)
  ///

  virtual CEventHandler *GetEventHandler() {
    return m_pEventHandler;
  };

  ///
  /// \deprecated In situ alphabet editing is no longer supported
  /// \todo Document this
  ///

  const CAlphInfo *GetInfo(const std::string & AlphID);

  /// \todo Document this

  void SetInfo(const CAlphInfo *NewInfo);

  /// \todo Document this

  void DeleteAlphabet(const std::string & AlphID);

  /// Gets a pointer to the object doing user logging

  CUserLogBase* GetUserLogPtr();

  // @}

  ///
  /// @name Parameter manipulation
  /// Members for manipulating the parameters of the core Dasher object.
  ///

  //@{

  ///
  /// Set a boolean parameter.
  /// \param iParameter The parameter to set.
  /// \param bValue The new value.
  ///

  void SetBoolParameter(int iParameter, bool bValue);

  ///
  /// Set a long integer parameter.
  /// \param iParameter The parameter to set.
  /// \param lValue The new value.
  ///

  void SetLongParameter(int iParameter, long lValue);

  ///
  /// Set a string parameter.
  /// \param iParameter The parameter to set.
  /// \param sValue The new value.
  ///

  void SetStringParameter(int iParameter, const std::string & sValue);

  /// Get a boolean parameter
  /// \param iParameter The parameter to get.
  /// \retval The current value.

  bool GetBoolParameter(int iParameter);

  /// Get a long integer parameter
  /// \param iParameter The parameter to get.
  /// \retval The current value.

  long GetLongParameter(int iParameter);

  /// Get a string parameter
  /// \param iParameter The parameter to get.
  /// \retval The current value.

  std::string GetStringParameter(int iParameter);

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

  /// Forward events to listeners in the SettingsUI and Editbox.
  /// \param pEvent The event to forward.
  /// \todo Should be protected.

  virtual void ExternalEventHandler(Dasher::CEvent * pEvent) {};

  /// Interface level event handler. For example, responsible for
  /// restarting the Dasher model whenever parameter changes make it
  /// invalid.
  /// \param pEvent The event.
  /// \todo Should be protected.

  void InterfaceEventHandler(Dasher::CEvent * pEvent);


  void PreSetNotify(int iParameter, const std::string &sValue);


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
  void Unpause(unsigned long Time);

  ///Whether any actions are currently setup to occur when Dasher 'stop's.
  /// Default is to return TRUE iff we support speech and BP_SPEAK_ON_STOP is set,
  /// and/or if we support clipboard and BP_COPY_ALL_ON_STOP is set; subclasses may
  /// override if they have additional on-stop actions.
  virtual bool hasStopTriggers();
  /// @}


  // App Interface
  // -----------------------------------------------------

  // std::map<int, std::string>& GetAlphabets(); // map<key, value> int is a UID string can change. Store UID in preferences. Display string to user.
  // std::vector<std::string>& GetAlphabets();
  // std::vector<std::string>& GetLangModels();
  // std::vector<std::string>& GetViews();

  /// Supply a new CDasherScreen object to do the rendering.
  /// \param NewScreen Pointer to the new CDasherScreen.

  void ChangeScreen(CDasherScreen * NewScreen); // We may change the widgets Dasher uses

  /// Train Dasher from a file
  /// All traing data must be in UTF-8
  /// \param Filename File to load.
  /// \param iTotalBytes documentme
  /// \param iOffset Document me
  //  int TrainFile(std::string Filename, int iTotalBytes, int iOffset);

  /// Part of the mechanism by which the DasherCore part gets the surrounding
  /// (preceding) characters from its context (i.e. the text edit box): an
  /// EV_EDIT_CONTEXT_REQUEST event specifying the context required (i.e.
  /// offset and length) is broadcast from the core, and picked up
  /// by platform-dependent code, which must then call SetContext with the
  /// requested text. TODO this is a ghastly mechanism: the
  /// EditContextRequest event goes to every component, and any number (>=0)
  /// of components (anywhere!) could call SetContext (at any time!), rather
  /// than just exactly one, _only_ in response to such an event...suggest
  /// sthg like "virtual std::string getContext(int off, int len)=0;" ???
  /// \param strNewContext The requested part of the context (UTF-8)

  void SetContext(std::string strNewContext);

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

  double GetFramerate();

  /// @}

  /// @name Control hierarchy and action buttons
  /// Manipulate the hierarchy of commands presented in control mode etc
  /// @{

  void RegisterNode( int iID, const std::string &strLabel, int iColour );

  void ConnectNode(int iChild, int iParent, int iAfter);

  void DisconnectNode(int iChild, int iParent);

  void ExecuteCommand(const std::string &strName);

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
  CDasherModule *GetDefaultInputDevice();
  CDasherModule *GetDefaultInputMethod();
  void SetDefaultInputDevice(CDasherModule *);
  void SetDefaultInputMethod(CDasherModule *);

  void StartShutdown();

  void AddGameModeString(const std::string &strText) {
    m_deGameModeStrings.push_back(strText);
    Stop();
    //    CreateDasherModel();
    CreateNCManager();
    //    Start();
  };

  void GameMessageIn(int message, void* messagedata);

  virtual void GameMessageOut(int message, const void* messagedata) {}

  void ScheduleRedraw() { 
    m_bRedrawScheduled = true; 
  }; 

  std::string GetContext(int iStart, int iLength);
  
  ///Subclasses should override to clear text edit box, etc., etc., but then
  /// call this (superclass) implementation as well to rebuild the model...
  virtual void ClearAllContext();
  virtual std::string GetAllContext()=0;

  /// Set a key value pair by name - designed to allow operation from
  /// the command line.  Returns 0 on success, an error string on failure. 
  ///
  const char* ClSet(const std::string &strKey, const std::string &strValue);

  void ImportTrainingText(const std::string &strPath);
  
protected:

  /// @name Startup
  /// Interaction with the derived class during core startup
  /// @{

  ///
  /// Allocate resources, create alphabets etc. This is a separate
  /// routine to the constructor to give us a chance to set up
  /// parameters before things are created.
  ///

  void Realize();

  ///
  /// Notify the core that the UI has been realised. At this point drawing etc. is expected to work
  ///

  void OnUIRealised();
	
  ///
  /// Creates a default set of modules. Override in subclasses to create any
  /// extra/different modules specific to the platform (eg input device drivers)
  ///	
  virtual void CreateModules();

  /// @}



  /// Draw a new Dasher frame, regardless of whether we're paused etc.
  /// \param iTime Current time in ms.
  /// \param bForceRedraw 
  /// \todo See comments in cpp file for some functionality which needs to be re-implemented
  void NewFrame(unsigned long iTime, bool bForceRedraw);


  enum ETransition {
    TR_MODEL_INIT = 0,
    TR_UI_INIT,
    TR_LOCK,
    TR_UNLOCK,
    TR_SHUTDOWN,
    TR_NUM
  };

  enum EState {
    ST_START = 0,
    ST_MODEL,
    ST_UI,
    ST_NORMAL,
    ST_LOCKED,
    ST_SHUTDOWN,
    ST_NUM,
    ST_FORBIDDEN,
    ST_DELAY
  };

  /// @name State machine functions
  /// ...
  /// @{

  void ChangeState(ETransition iTransition);

  /// @}

  CEventHandler *m_pEventHandler;
  CSettingsStore *m_pSettingsStore;

 private:

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

  ///
  /// Create settings store object, which will be platform dependent
  /// TODO: Can this not be done just by selecting which settings
  /// store implementation to instantiate?
  ///

  virtual void CreateSettingsStore() = 0;

  ///
  /// Obtain the size in bytes of a file - the way to do this is
  /// dependent on the OS (TODO: Check this - any posix on Windows?)
  ///

  virtual int GetFileSize(const std::string &strFileName) = 0;

  ///
  /// Start the callback timer
  ///

  virtual void StartTimer() = 0;
  
  ///
  /// Shutdown the callback timer (permenantly - this is called once
  /// Dasher is committed to closing).
  ///

  virtual void ShutdownTimer() = 0;

  ///
  /// Append text to the training file - used to store state between
  /// sessions
  /// @todo Pass file path to the function rather than having implementations work it out themselves
  ///
  
  virtual void WriteTrainFile(const std::string &strNewText) {
  };

  /// @}


  /// Provide a new CDasherInput input device object.

  void CreateInput();
	
  void CreateInputFilter();

  void CreateModel(int iOffset);
  void CreateNCManager();

  void ChangeAlphabet();
  void ChangeColours();
  void ChangeView();
  void Redraw(bool bRedrawNodes, CExpansionPolicy &policy);
  void SetupActionButtons();
  void DestroyActionButtons();
  void PositionActionButtons();
  bool DrawActionButtons();

  void WriteTrainFileFull();
  void WriteTrainFilePartial();
  
  std::deque<std::string> m_deGameModeStrings;

  std::vector<CActionButton *> m_vLeftButtons;
  std::vector<CActionButton *> m_vRightButtons;


  /// @name Child components
  /// Various objects which are 'owned' by the core. 
  /// @{
  CDasherModel *m_pDasherModel;
  CDasherScreen *m_DasherScreen;
  CDasherView *m_pDasherView;
  CDasherInput *m_pInput;
  CInputFilter* m_pInputFilter;
  CModuleManager m_oModuleManager;
  CAlphInfo::AlphIO *m_AlphIO;
  CColourIO *m_ColourIO;
  CNodeCreationManager *m_pNCManager;
  CUserLogBase *m_pUserLog; 
  /// @}

  std::string strTrainfileBuffer;
  std::string strCurrentContext;
  
  ///builds up the word currently being entered for speech.
  std::string m_strCurrentWord;

  std::string m_strContext;

  /// @name State variables
  /// Represent the current overall state of the core
  /// @{
  //  bool m_bGlobalLock; // The big lock
  bool m_bRedrawScheduled;
  EState m_iCurrentState;
  bool m_bOldVisible;

  /// @}

  bool m_bLastChanged;
  
  std::set<TextAction *> m_vTextActions;
};
/// @}

#endif /* #ifndef __DasherInterfaceBase_h__ */
