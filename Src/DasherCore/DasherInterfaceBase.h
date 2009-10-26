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
#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "AutoSpeedControl.h"
#include "ColourIO.h"
#include "InputFilter.h"
#include "ModuleManager.h"

#include <map>
#include <algorithm>

namespace Dasher {
  class CDasherScreen;
  class CDasherView;
  class CDasherInput;
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


struct Dasher::SLockData {
  std::string strDisplay;
  int iPercent;
};


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

  const CAlphIO::AlphInfo & GetInfo(const std::string & AlphID);

  /// \todo Document this

  void SetInfo(const CAlphIO::AlphInfo & NewInfo);

  /// \todo Document this

  void DeleteAlphabet(const std::string & AlphID);

  /// Get a pointer to the current alphabet object

  CAlphabet *GetAlphabet() {
    return m_Alphabet;
  }

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



  /// @name Starting and stopping
  /// Methods used to instruct dynamic motion of Dasher to start or stop
  /// @{ 

  /// Resets the Dasher model. Doesn't actually unpause Dasher.
  /// \deprecated Use InvalidateContext() instead

  //  void Start();

  /// Pause Dasher
  /// \todo Parameters are ignored (?) - remove from definition.

  void PauseAt(int MouseX, int MouseY); // are required to make

  /// Unpause Dasher
  /// \param Time Time in ms, used to keep a constant frame rate

  void Unpause(unsigned long Time);     // Dasher run at the

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

  /// Set the context in which Dasher makes predictions
  /// \param strNewContext The new context (UTF-8)

  void SetContext(std::string strNewContext);

  /// New control mechanisms:

  void SetBuffer(int iOffset);

  void UnsetBuffer(); 

  void SetOffset(int iOffset);

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
    PauseAt(0,0);
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

  void SetControlOffset(int iOffset);

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
  CAlphabet *m_Alphabet;
  CDasherModel *m_pDasherModel;
  CDasherScreen *m_DasherScreen;
  CDasherView *m_pDasherView;
  CDasherInput *m_pInput;
  CInputFilter* m_pInputFilter;
  CModuleManager m_oModuleManager;
  CAlphIO *m_AlphIO;
  CColourIO *m_ColourIO;
  CNodeCreationManager *m_pNCManager;
  CUserLogBase *m_pUserLog; 
  /// @}

  std::string strTrainfileBuffer;
  std::string strCurrentContext;

  std::string m_strContext;

  /// @name State variables
  /// Represent the current overall state of the core
  /// @{
  //  bool m_bGlobalLock; // The big lock
  bool m_bRedrawScheduled;
  EState m_iCurrentState;
  bool m_bOldVisible;

  std::map<int, SLockData> m_mapCurrentLocks;
  int m_iNextLockID;

  /// @}

  bool m_bLastChanged;
};
/// @}

#endif /* #ifndef __DasherInterfaceBase_h__ */
