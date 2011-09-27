/*
 *  CDasherInterfaceBridge.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 18/3/2009.
 *
 */

#import "../DasherCore/DashIntfScreenMsgs.h"
#import "IPhoneInputs.h"
#import "IPhoneFilters.h"
#import "DefaultFilter.h"
#import "StylusFilter.h"
#import "Vec3.h"

@class DasherAppDelegate;

///
/// \brief C++ core of the Dasher UI component.
///
/// Class representing the Dasher UI component (ie the canvas and speed slider).
/// Implements the necessary abstract methods by bridging them into Objective C
/// and sending messages onto the DasherAppDelegate.
///
/// We inherit from CDashIntfScreenMsgs but use it for displaying modal messages
/// only (over the canvas). TODO: we could possibly do better by displaying a modal
/// View, however we would need some way for users not capable of touching the screen
/// (who might be using Dasher via tilt control & start-on-circle) to clear the display!
class CDasherInterfaceBridge : public Dasher::CDashIntfScreenMsgs {
  
public:
  
  ///
  /// \param pVBox GTK VBox to populate with the DasherControl
  /// component widgets. This needs to be created externally by the
  /// GObject wrapper and passed to the C++ class rather than being
  /// created internally.  
  /// \param pDasherControl Pointer to the GObject wrapper. This is
  /// needed so that we can emit signals from the GObject.
  ///
  
  CDasherInterfaceBridge(DasherAppDelegate *aDasherApp);
  ~CDasherInterfaceBridge();
  
  ///Calls superclass Realize with system time, starts timer, etc.
  /// - i.e. everything ve need to make ourselves operational (except ChangeScreen)
  void Realize();
  
  //redefine to make public....
  void NewFrame(unsigned long iTime, bool bForceRedraw);

  void SetTiltAxes(Vec3 main, float off, Vec3 slow, float off2);
  virtual void WriteTrainFile(const std::string &filename,const std::string &strNewText);
  virtual int GetFileSize(const std::string &strFileName);
  bool SupportsClipboard() {return true;}
  void CopyToClipboard(const std::string &strText);
  bool SupportsSpeech();
  void Speak(const std::string &strText, bool bInterrupt);
  void ClearAllContext();
  std::string GetAllContext();
  std::string GetContext(unsigned int iStart, unsigned int iLength);
  unsigned int ctrlMove(bool bForwards, Dasher::CControlManager::EditDistance dist);
  unsigned int ctrlDelete(bool bForwards, Dasher::CControlManager::EditDistance dist);
  void SetLockStatus(const string &strText, int iPercent);
  void editOutput(const string &strText, Dasher::CDasherNode *pNode);
  void editDelete(const string &strText, Dasher::CDasherNode *pNode);
  void editConvert(Dasher::CDasherNode *pNode);
  void editProtect(Dasher::CDasherNode *pNode);
  ///Override for asynchronous messages only...TODO?
  void Message(const string &strText, bool bInterrupt);
  Dasher::CGameModule *CreateGameModule();
  
  void HandleEvent(int iParameter);
  void EnterGameMode(Dasher::CGameModule *pGameModule);
  void LeaveGameMode();
private:
  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void CreateModules();
  virtual void SetupUI();
  
  ///
  /// Pass events coming from the core to the appropriate handler.
  ///
  
  DasherAppDelegate *dasherApp;   // objc counterpart
  	
  Dasher::CIPhoneMouseInput *m_pMouseDevice;
  Dasher::CIPhoneTiltInput *m_pTiltDevice;
  Dasher::UndoubledTouch *m_pUndoubledTouch;
  Dasher::CIPhoneTwoFingerInput *m_pTwoFingerDevice;
};
