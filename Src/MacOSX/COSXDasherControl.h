/*
 *  COSXDasherControl.h
 *  Dasher
 *
 *  Created by Doug Dickinson on 17/11/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#import "../DasherCore/SocketInput.h"

// #import "../DasherCore/DasherSettingsInterface.h"
#import "../DasherCore/DasherInterfaceBase.h"
#import "COSXMouseInput.h"

@class DasherApp;
@class DasherEdit;

///
/// \brief C++ core of the Dasher GTK UI component.
///
/// Class representing the Dasher UI component (ie the canvas and speed slider)
///

class COSXDasherControl : public CDasherInterfaceBase {
  
public:
  
  ///
  /// \param pVBox GTK VBox to populate with the DasherControl
  /// component widgets. This needs to be created externally by the
  /// GObject wrapper and passed to the C++ class rather than being
  /// created internally.  
  /// \param pDasherControl Pointer to the GObject wrapper. This is
  /// needed so that we can emit signals from the GObject.
  ///
  
  COSXDasherControl(DasherApp *aDasherApp);
  ~COSXDasherControl();
  
  void Realize2();
  void TimerFired(NSPoint p);
  void Train(NSString *fileName);
  id GetParameter(NSString *aKey);
  void SetParameter(NSString *aKey, id aValue);
  NSDictionary *ParameterDictionary();
  void goddamn(unsigned long iTime, bool bForceRedraw);
  virtual void WriteTrainFile(const std::string &filename, const std::string &strNewText);
  std::string GetAllContext();
  void ClearAllContext();
  std::string GetContext(unsigned int iOffset, unsigned int iLength);
private:
  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void CreateModules();
  virtual void SetupUI();
  virtual void CreateSettingsStore();
  virtual int GetFileSize(const std::string &strFileName);
  virtual void StartTimer();
  virtual void ShutdownTimer();
  virtual bool SupportsSpeech();
  virtual void Speak(const std::string &strText, bool bInterrupt);
  virtual bool SupportsClipboard() {return true;}
  virtual void CopyToClipboard(const std::string &strText);
  ///Control-mode editing commands not currently supported on MacOSX,
  /// so just returns the current offset unchanged.
  ///Could try to send arrow keys via LowLevelKeyboardHandling,
  /// but at best we'd lose our context.
  ///Really this will have to wait for either (a)reinstating an in-Dasher
  /// text edit box, or (b) implementing the MacOSX input method API.
  unsigned int ctrlMove(bool bForwards, CControlManager::EditDistance dist);

  ///Control-mode editing commands not currently supported on MacOSX;
  /// forward deletion returns current offset and does nothing,
  /// backwards deletion we look for space/paragraph characters
  /// and send corresponding # of (char-)deletes.
  ///Could try to send complex keypresses (option-delete=word, line/file...
  /// = select then delete?) via LowLevelKeyboardHandling, but at best we'd
  /// lose our context.
  ///Really this will have to wait for either (a)reinstating an in-Dasher
  /// text edit box, or (b) implementing the MacOSX input method API.
  unsigned int ctrlDelete(bool bForwards, CControlManager::EditDistance dist);
  ///
  /// Pass events coming from the core to the appropriate handler.
  ///
  
  void ExternalEventHandler(Dasher::CEvent *pEvent);
  void GameMessageOut(int message, const void* messagedata);
  
  DasherApp *dasherApp;   // objc counterpart
  DasherEdit *dasherEdit;  // for outputting into other apps
  
  COSXMouseInput *m_pMouseInput;
  COSX1DMouseInput *m_p1DMouseInput;

  
};
