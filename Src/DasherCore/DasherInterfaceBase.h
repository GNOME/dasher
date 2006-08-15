// DasherInterfaceBase.h
//
// Copyright (c) 2002 Iain Murray

#ifndef __DasherInterfaceBase_h__
#define __DasherInterfaceBase_h__

// TODO - there is a list of things to be configurable in my notes
// Check that everything that is not self-contained within the GUI is covered.

#include "../Common/NoClones.h"
#include "../Common/ModuleSettings.h"
#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "CustomColours.h"
#include "ColourIO.h"
#include "ModuleManager.h"

#include "AutoSpeedControl.h"

#include "InputFilter.h"

namespace Dasher {
  class CDasherScreen;
  class CDasherView;
  class CDasherInput;
  class CDasherModel;
  class CEventHandler;
  class CEvent;
}

class Dasher::CDasherScreen;
class Dasher::CDasherView;
class Dasher::CDasherInput;
class Dasher::CDasherModel;
class Dasher::CEventHandler;
class Dasher::CEvent;
class CSettingsStore;
class CUserLog;
class CDasherButtons;

#include <map>
#include <algorithm>

namespace Dasher {
  class CDasherInterfaceBase;
}
/// The central class in the core of Dasher. Ties together the rest of
/// the platform independent stuff and provides a single interface for
/// the UI to use.

class Dasher::CDasherInterfaceBase:private NoClones
{
public:
  CDasherInterfaceBase();
  virtual ~ CDasherInterfaceBase();

  ///
  /// Allocate resources, create alphabets etc. This is a separate
  /// routine to the constructor to give us a chance to set up
  /// parameters before things are created.
  ///

  void Realize();


  ///
  /// Return a pointer to the current EventHandler (the one
  /// which the CSettingsStore is using to notify parameter
  /// changes)
  ///

  virtual CEventHandler *GetEventHandler() {
    return m_pEventHandler;
  };

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

  void PreSetNotify(int iParameter, const std::string &sValue);

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
  
  void ResetParameter(int iParmater);

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

  // Widget Interface
  // -----------------------------------------------------

  /// Resets the Dasher model. Doesn't actually unpause Dasher.
  /// \deprecated Use InvalidateContext() instead

  void Start();

  /// Draw a new Dasher frame, regardless of whether we're paused etc.
  /// \param iTime Current time in ms.
  /// \todo See comments in cpp file for some functionality which needs to be re-implemented

  void NewFrame(unsigned long iTime);

  /// Pause Dasher
  /// \todo Parameters are ignored (?) - remove from definition.

  void PauseAt(int MouseX, int MouseY); // are required to make

  /// Halt Dasher. This simply freezes Dasher but does not emit a stop event, so does not result in speech etc.

  void Halt();

  /// Unpause Dasher
  /// \param Time Time in ms, used to keep a constant frame rate

  void Unpause(unsigned long Time);     // Dasher run at the

  /// Force a redraw of the Dasher display
  /// \todo I don't really see the need to call this externally. In
  /// Linux drawing is always done to an offscreen buffer, so it's not
  /// needed during canvas exposure events.


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
  /// \param Filename File to load.

  int TrainFile(std::string Filename, int iTotalBytes, int iOffset); // all training data must be in UTF-8.

  /// \todo Document this

  void GetFontSizes(std::vector < int >*FontSizes) const;

  /// Get the current rate of text entry.
  /// \retval The rate in characters per minute.

  double GetCurCPM();           // App may want to display characters per minute

  /// Get current refresh rate.
  /// \retval The rate in frames per second

  double GetCurFPS();           // or frames per second.

  // Customize alphabet

  /// \todo Document this

  void GetAlphabets(std::vector < std::string > *AlphabetList);

  /// \todo Document this

  const CAlphIO::AlphInfo & GetInfo(const std::string & AlphID);

  /// \todo Document this

  void SetInfo(const CAlphIO::AlphInfo & NewInfo);

  /// \todo Document this

  void DeleteAlphabet(const std::string & AlphID);

  /// \todo Document this

  void GetColours(std::vector < std::string > *ColourList);

  void GetPermittedValues(int iParameter, std::vector<std::string> &vList);

  /// Get the current autocalibration offset
  /// \retval The offset.

  int GetAutoOffset();

  /// \todo Document this

  //  void Render();

  /// Provide a new CDasherInput input device object.

  void CreateInput();

  /// Set the context in which Dasher makes predictions
  /// \param strNewContext The new context (UTF-8)

  void SetContext(std::string strNewContext);
  void InvalidateContext(bool bForceStart);

  /// Get the total number of nats (base-e bits) entered.
  /// \retval The current total
  /// \todo Obsolete since new logging code?

  double GetNats() const;

  /// Reset the count of nats entered.
  /// \todo Obsolete since new logging code?

  void ResetNats();

  /// Get a pointer to the current alphabet object

  CAlphabet *GetAlphabet() {
    return m_Alphabet;
  }

  // Control mode stuff
  
  void RegisterNode( int iID, const std::string &strLabel, int iColour );
  void ConnectNode(int iChild, int iParent, int iAfter);
  void DisconnectNode(int iChild, int iParent);

  /// Gets a pointer to the object doing user logging
  CUserLog*       GetUserLogPtr();
  
  virtual void WriteTrainFile(const std::string &strNewText) {
  };

  void KeyDown(int iTime, int iId);
  void KeyUp(int iTime, int iId);

  // Module management functions
  void RegisterFactory(CModuleFactory *pFactory);

  void CreateFactories();

  void StartShutdown();

  bool GetModuleSettings(const std::string &strName, SModuleSettings **pSettings, int *iCount);

  void AddGameModeString(const std::string &strText) {
    m_deGameModeStrings.push_back(strText);
    PauseAt(0,0);
    CreateDasherModel();
    Start();
  };

  void ScheduleRedraw() {
    m_bRedrawScheduled = true;
  };
  
protected:
  void WriteTrainFileFull();
  void WriteTrainFilePartial();

  // Various 'child' components
  CAlphabet *m_Alphabet;
  CCustomColours *m_pColours;
  CDasherModel *m_pDasherModel;
  CDasherScreen *m_DasherScreen;
  CDasherView *m_pDasherView;
  CDasherInput *m_pInput;
  CAlphIO *m_AlphIO;
  CColourIO *m_ColourIO;

  std::string strTrainfileBuffer;
  std::string strCurrentContext;

  CEventHandler *m_pEventHandler;
  CSettingsStore *m_pSettingsStore;
  CUserLog *m_pUserLog;               // Pointer to the object that handles logging user activity
  CInputFilter* m_pInputFilter;
  CModuleManager m_oModuleManager;
  
  bool m_bGlobalLock; // The big lock
  bool m_bShutdownLock;
  
  // TODO: Make private?
  CDasherModule *GetModule(long long int iID);
  CDasherModule *GetModuleByName(const std::string &strName);

  bool m_bRedrawScheduled;

 private:
  // To be implemented by child class
  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList) = 0;
  virtual void ScanColourFiles(std::vector<std::string> &vFileList) = 0;
  virtual void SetupPaths() = 0;
  virtual void SetupUI() = 0;
  virtual void CreateSettingsStore() = 0;
  virtual int GetFileSize(const std::string &strFileName) = 0;

  void CreateInputFilter();
  void CreateDasherModel();
  void ChangeAlphabet();
  void ChangeColours();
  void ChangeView();

  void Redraw(bool bRedrawNodes);                // correct speed.


  std::deque<std::string> m_deGameModeStrings;
};

#endif /* #ifndef __DasherInterfaceBase_h__ */
