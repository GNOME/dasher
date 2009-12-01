/*
 *  CDasherInterfaceBridge.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 18/3/2009.
 *
 */

#import "../DasherCore/DasherInterfaceBase.h"
#import "IPhoneInputs.h"
#import "IPhoneFilters.h"
#import "DefaultFilter.h"
#import "PlainDragFilter.h"
#import "StylusFilter.h"
#import "Vec3.h"

@class DasherAppDelegate;

///
/// \brief C++ core of the Dasher UI component.
///
/// Class representing the Dasher UI component (ie the canvas and speed slider).
/// Implements the necessary abstract methods by bridging them into Objective C
/// and sending messages onto the DasherAppDelegate.

class CDasherInterfaceBridge : public CDasherInterfaceBase {
  
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
  
  void ChangeScreen(CDasherScreen *NewScreen);
  
  //redefinitions to make public....
  void OnUIRealised();
  void NewFrame(unsigned long iTime, bool bForceRedraw);
  
  void NotifyTouch(screenint x, screenint y);
  void SetTiltAxes(Vec3 main, float off, Vec3 slow, float off2);
private:
  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void CreateModules();
  virtual void SetupUI();
  virtual void CreateSettingsStore();
  virtual int GetFileSize(const std::string &strFileName);
  virtual void WriteTrainFile(const std::string &strNewText);
  virtual void StartTimer();
  virtual void ShutdownTimer();
  
  ///
  /// Pass events coming from the core to the appropriate handler.
  ///
  
  void ExternalEventHandler(Dasher::CEvent *pEvent);
  void GameMessageOut(int message, const void* messagedata);
  
  DasherAppDelegate *dasherApp;   // objc counterpart
  
  CPlainDragFilter *m_pPlainDragFilter;
  CIPhone1DFilter *m_pOneDFilter;
  CIPhonePolarFilter *m_pPolarFilter;
	
  CMixedInput *m_pMixDevice, *m_pReverseMix;
  CIPhoneMouseInput *m_pMouseDevice;
  CIPhoneTiltInput *m_pTiltDevice;
	
};
