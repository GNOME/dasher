// DasherInterfaceBase.h
//
// Copyright (c) 2002 Iain Murray

#ifndef __DasherInterfaceBase_h__
#define __DasherInterfaceBase_h__

// TODO - there is a list of things to be configurable in my notes
// Check that everything that is not self-contained within the GUI is covered.

#include "../Common/NoClones.h"

#include "DasherWidgetInterface.h"
#include "DasherAppInterface.h"
#include "DasherSettingsInterface.h"

#include "DasherScreen.h"
#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "CustomColours.h"
#include "ColourIO.h"
#include "DashEdit.h"
#include "DasherView.h"
#include "DasherInput.h"

#include "EventHandler.h"
#include "Event.h"

#include <map>
#include <iostream>

namespace Dasher {
  class CDasherInterfaceBase;
}
/// The central class in the core of Dasher. Ties together the rest of
/// the platform independent stuff and provides a single interface for
/// the UI to use.

class Dasher::CDasherInterfaceBase:private NoClones, public CDasherWidgetInterface, public CDasherAppInterface, public CDasherSettingsInterface
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
  /// Create a settings store - to be implemented by the
  /// platform-dependent derived class
  ///

  virtual CSettingsStore *CreateSettingsStore() = 0;

  ///
  /// Set a boolean parameter.
  /// \param iParameter The parameter to set.
  /// \param bValue The new value.
  ///

  void SetBoolParameter(int iParameter, bool bValue) {
    m_pSettingsStore->SetBoolParameter(iParameter, bValue);
  };

  ///
  /// Set a long integer parameter.
  /// \param iParameter The parameter to set.
  /// \param lValue The new value.
  ///

  void SetLongParameter(int iParameter, long lValue) {
    m_pSettingsStore->SetLongParameter(iParameter, lValue);
  };

  ///
  /// Set a string parameter.
  /// \param iParameter The parameter to set.
  /// \param sValue The new value.
  ///

  void SetStringParameter(int iParameter, const std::string & sValue) {
    m_pSettingsStore->SetStringParameter(iParameter, sValue);
  };

  /// Get a boolean parameter
  /// \param iParameter The parameter to get.
  /// \retval The current value.

  bool GetBoolParameter(int iParameter) {
    return m_pSettingsStore->GetBoolParameter(iParameter);
  }

  /// Get a long integer parameter
  /// \param iParameter The parameter to get.
  /// \retval The current value.

  long GetLongParameter(int iParameter) {
    return m_pSettingsStore->GetLongParameter(iParameter);
  }

  /// Get a string parameter
  /// \param iParameter The parameter to get.
  /// \retval The current value.

  std::string GetStringParameter(int iParameter) {
    return m_pSettingsStore->GetStringParameter(iParameter);
  }

  /// Forward events to listeners in the SettingsUI and Editbox.
  /// \param pEvent The event to forward.
  /// \todo Should be protected.

  void ExternalEventHandler(Dasher::CEvent * pEvent);

  /// Interface level event handler. For example, responsible for
  /// restarting the Dasher model whenever parameter changes make it
  /// invalid.
  /// \param pEvent The event.
  /// \todo Should be protected.

  void InterfaceEventHandler(Dasher::CEvent * pEvent);

  /// Tell the core which CDasherSettingsInterface should be used
  ///
  /// Provide a pointer to an instance of CDasherSettingsInterface in 
  /// order to allow for platform dependent configuration of certain 
  /// options
  /// \param SettingsUI Pointer to the CDasherSettingsInterface.

  void SetSettingsUI(CDasherSettingsInterface * SettingsUI);

  /// Set the path for user specific configuration and files
  /// \param UserLocation The new path.
  /// \todo This is unlikely to change, so should probably be supplied directly to the constructor.

  void SetUserLocation(std::string UserLocation);

  /// Set the path for system-wide configuration and files
  /// \param SystemLocation The new path.
  /// \todo This is unlikely to change, so should probably be supplied directly to the constructor.

  void SetSystemLocation(std::string SystemLocation);

  /// Add an alphabet filename
  /// \param Filename The filename to add

  void AddAlphabetFilename(std::string Filename);

  /// Add a colour filename
  /// \param Filename The filename to add

  void AddColourFilename(std::string Filename);

  // Widget Interface
  // -----------------------------------------------------

  /// Resets the Dasher model. Doesn't actually unpause Dasher.

  void Start();

  /// Draw a new Dasher frame, regardless of whether we're paused etc.
  /// \param iTime Current time in ms.
  /// \todo See comments in cpp file for some functionality which needs to be re-implemented

  void NewFrame(unsigned long iTime);

  /// Prompts Dasher to draw a new frame
  /// \param Time The current time (in ms). Used to ensure a constant frame rate.
  /// \todo MouseX and MouseY are currently ignored - remove from the definition.
  /// \deprecated Use NewFrame instead.

  void TapOn(int MouseX, int MouseY, unsigned long Time);       // Times in milliseconds

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

  void Redraw();                // correct speed.

  /// Request a full redraw at the next timer event.

  void RequestFullRedraw();

  /// \todo Sort out difference between Redraw functions

  void Redraw(int iMouseX, int iMouseY);

  /// Draw the boxes for start on mouse position
  /// \todo Shouldn't be called externally

  void DrawMousePos(int MouseX, int MouseY, int iWhichBox);

  /// \todo Document this

  void GoTo(int MouseX, int MouseY);

  /// \todo Document this

  void DrawGoTo(int MouseX, int MouseY);

  /// Force an update of the CScreen object without changing the pointer
  /// \todo When is this needed?

  void ChangeScreen();          // The widgets need to tell the engine when they have been

  /// Force an update of the CEditbox object without changing the pointer
  /// \todo When is this needed?

  void ChangeEdit();            // affected by external interaction

  /// Returns the number of symbols in the alphabet
  /// \retval the number of symbols.

  unsigned int GetNumberSymbols();      // These are needed so widgets know

  /// Get the display string for a symbol
  /// \param Symbol The symbol ID to look up.
  /// \retval UTF-8 string to be displayed

  const std::string & GetDisplayText(symbol Symbol);    // how to render the alphabet. All

  /// Get the edit string for a symbol
  /// \param Symbol The symbol ID to look up.
  /// \retval UTF-8 string to be displayed

  const std::string & GetEditText(symbol Symbol);       // strings are encoded in UTF-8

  /// \todo Document this

  int GetTextColour(symbol Symbol);     // the foreground colour of the text

  /// \todo Document this

  Opts::ScreenOrientations GetAlphabetOrientation();

  /// \todo Document this

  Opts::AlphabetTypes GetAlphabetType();

  /// \todo Document this

  const std::string GetTrainFile();

  // App Interface
  // -----------------------------------------------------

  // std::map<int, std::string>& GetAlphabets(); // map<key, value> int is a UID string can change. Store UID in preferences. Display string to user.
  // std::vector<std::string>& GetAlphabets();
  // std::vector<std::string>& GetLangModels();
  // std::vector<std::string>& GetViews();

  /// Supply a new CDasherScreen object to do the rendering.
  /// \param NewScreen Pointer to the new CDasherScreen.

  void ChangeScreen(CDasherScreen * NewScreen); // We may change the widgets Dasher uses

  /// Supply a new CDashEditbox to receive output events
  /// \param NewEdit Pointer to the new CDashEditbox.

  void ChangeEdit(CDashEditbox * NewEdit);      // at run time.

  /// Train Dasher using a UTF-8 string
  /// \param TrainString The training string.
  /// \param IsMore Whether to keep state (affects how incomplete UTF-8 characters are handled at the end)

  void Train(std::string * TrainString, bool IsMore);   // Training by string segments or file

  /// Train Dasher from a file
  /// \param Filename File to load.

  void TrainFile(std::string Filename); // all training data must be in UTF-8.

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

  /// \todo Document this

  void AddControlTree(ControlTree * controltree);

  /////////////////////////////////////////////////////////////////////////////
  // Settings Interface (CDasherSettingsInterface) - options saved between sessions
  /////////////////////////////////////////////////////////////////////////////

  /// \deprecated Use parameter interface instead

  virtual void MouseposStart(bool Value);

  /// \deprecated Use parameter interface instead

  void ChangeAlphabet(const std::string & NewAlphabetID);

  /// \deprecated Use parameter interface instead

  std::string GetCurrentAlphabet();

  /// \deprecated Use parameter interface instead

  void ChangeColours(const std::string & NewColourID);

  /// \deprecated Use parameter interface instead

  std::string GetCurrentColours();

  /// \deprecated Use parameter interface instead

  void ChangeMaxBitRate(double NewMaxBitRate);

  // DJW - nasty thing about this is - we dont necessarily want the LM to rebuild every
  // time a parameter is change - e.g. if we change 2 or 3 params in a row ???

  /// \deprecated Use parameter interface instead

  void ChangeLanguageModel(int NewLanguageModelID);

  /// \deprecated Use parameter interface instead

  void ChangeView(unsigned int NewViewID);

  /// \deprecated Use parameter interface instead

  void ChangeOrientation(Opts::ScreenOrientations Orientation);

  /// \deprecated Use parameter interface instead

  void SetFileEncoding(Opts::FileEncodingFormats Encoding);

  /// \deprecated Not part of Dasher control

  void ShowToolbar(bool Value);

  /// \deprecated Not part of Dasher control

  void ShowToolbarText(bool Value);

  /// \deprecated Not part of Dasher control

  void ShowToolbarLargeIcons(bool Value);

  /// \deprecated Use parameter interface instead

  void ShowSpeedSlider(bool Value);

  /// \deprecated Not part of Dasher control

  void FixLayout(bool Value);

  /// \deprecated Not part of Dasher control

  void TimeStampNewFiles(bool Value);

  /// \deprecated Not part of Dasher control

  void CopyAllOnStop(bool Value);

  /// \deprecated Use parameter interface instead

  void DrawMouse(bool Value);

  /// \deprecated Use parameter interface instead

  void DrawMouseLine(bool Value);

  ///
  /// \deprecated Use parameter interface instead

  void StartOnSpace(bool Value);

  /// \deprecated Use parameter interface instead

  void StartOnLeft(bool Value);

  /// \deprecated Document this

  void KeyControl(bool Value);

  /// \deprecated Not part of Dasher control

  void WindowPause(bool Value);

  /// \deprecated Use parameter interface instead

  void ControlMode(bool Value);

  /// \deprecated Use parameter interface instead

  void ColourMode(bool Value);

  /// \deprecated Not part of Dasher control

  void KeyboardMode(bool Value);

  /// \deprecated Not part of Dasher control

  void Speech(bool Value);

  /// \deprecated Use parameter interface instead

  void OutlineBoxes(bool Value);

  /// \deprecated Use parameter interface instead

  void PaletteChange(bool Value);

  /// \deprecated Not part of Dasher control

  void SetScreenSize(long Width, long Height);

  /// \deprecated Not part of Dasher control

  void SetEditHeight(long Value);

  /// \deprecated Not part of Dasher control

  void SetEditFont(std::string Name, long Size);

  /// \deprecated Use parameter interface instead

  void SetDasherFont(std::string Name);

  /// \deprecated Use parameter interface instead

  void SetDasherFontSize(FontSize fontsize);

  /// \deprecated Use parameter interface instead

  void SetDasherDimensions(bool Value);

  /// \deprecated Use parameter interface instead

  void SetDasherEyetracker(bool Value);

  /// \deprecated Use parameter interface instead

  void SetUniform(int Value);

  /// \deprecated Use parameter interface instead

  void SetYScale(int Value);

  /// \deprecated Use parameter interface instead

  void SetMousePosDist(int Value);

  /// \deprecated Use parameter interface instead

  void SetTruncation(int Value);

  /// \deprecated Use parameter interface instead

  void SetTruncationType(int Value);

  // Which mouse position box should the View draw?
  // 0 - no box, 1 - upper box, 2 - lower box

  /// \deprecated Use parameter interface instead

  void SetDrawMousePosBox(int iWhich);

  /// Get the current autocalibration offset
  /// \retval The offset.

  int GetAutoOffset();

  /// \todo Document this

  void Render();

  /// Provide a new CDasherInput input device object.
  /// \param _pInput Pointer to the new object

  void SetInput(CDasherInput * _pInput);

  /// Set the context in which Dasher makes predictions
  /// \param strNewContext The new context (UTF-8)

  void SetContext(std::string strNewContext) {
    if(m_pDasherModel != NULL)
      m_pDasherModel->SetContext(strNewContext);
  }

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

  /// Get a pointer to the CDashEditbox

  CDashEditbox *GetEditbox() {
    return m_DashEditbox;
  }

private:
  CAlphabet * m_Alphabet;
  CCustomColours *m_pColours;
  CDasherModel *m_pDasherModel;
  CDashEditbox *m_DashEditbox;
  CDasherScreen *m_DasherScreen;
  CDasherView *m_pDasherView;

  CDasherInput *m_pInput;

  CDasherSettingsInterface *m_SettingsUI;
  CAlphIO *m_AlphIO;
  CAlphIO::AlphInfo m_AlphInfo;
  CColourIO *m_ColourIO;
  CColourIO::ColourInfo m_ColourInfo;

  std::vector < std::string > m_AlphabetFilenames;
  std::vector < std::string > m_ColourFilenames;

  static const std::string EmptyString;

  void CreateDasherModel();

protected:
  CEventHandler * m_pEventHandler;
  CSettingsStore *m_pSettingsStore;
};

#endif /* #ifndef __DasherInterfaceBase_h__ */
