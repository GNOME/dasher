/*
 *  CDasherInterfaceBridge.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 18/3/2009.
 *
 */

#import "CDasherInterfaceBridge.h"
#import "COSXSettingsStore.h"
#import "ModuleManager.h"
#import "DasherUtil.h"
#import "DasherAppDelegate.h"
#import "Event.h"
#import "CalibrationController.h"
#import "ControlManager.h"
#import "../Common/Common.h"
#import "ButtonMode.h"
#import "TwoButtonDynamicFilter.h"
#import "TwoPushDynamicFilter.h"
#import <iostream>

#import <fcntl.h>

#import <sys/stat.h>

using namespace std;

CDasherInterfaceBridge::CDasherInterfaceBridge(DasherAppDelegate *aDasherApp) : dasherApp(aDasherApp) {
}

void CDasherInterfaceBridge::CreateModules() {
	//create the default set...a good idea?!?!

  RegisterModule(m_pUndoubledTouch = new UndoubledTouch(m_pEventHandler, m_pSettingsStore));
	RegisterModule(m_pMouseDevice = 
				new CIPhoneMouseInput(m_pEventHandler, m_pSettingsStore));
	RegisterModule(m_pTiltDevice = 
				new CIPhoneTiltInput(m_pEventHandler, m_pSettingsStore));
  SetDefaultInputDevice(m_pMouseDevice);
                 
  RegisterModule(new CButtonMode(m_pEventHandler, m_pSettingsStore, this, true, 9, "Menu Mode"));
  RegisterModule(new CButtonMode(m_pEventHandler, m_pSettingsStore, this, false, 8, "Direct Mode"));
  RegisterModule(new CTwoButtonDynamicFilter(m_pEventHandler, m_pSettingsStore, this));
  RegisterModule(new CTwoPushDynamicFilter(m_pEventHandler, m_pSettingsStore, this));
  
	RegisterModule(m_pTiltFilter =
				   new CIPhoneTiltFilter(m_pEventHandler, m_pSettingsStore, this, 16, m_pMouseDevice));
	RegisterModule(m_pTouchFilter = 
				   new CIPhoneTouchFilter(m_pEventHandler, m_pSettingsStore, this, 17, m_pUndoubledTouch, m_pTiltDevice));
	SetDefaultInputMethod(m_pTouchFilter);
}
	
CDasherInterfaceBridge::~CDasherInterfaceBridge() {
  delete m_pMouseDevice;
	delete m_pTiltDevice;
  delete m_pUndoubledTouch;
  //(ACL)registered input filters should be automatically free'd by the module mgr?
}

void CDasherInterfaceBridge::SetTiltAxes(Vec3 main, float off, Vec3 slow, float off2)
{
	m_pTiltDevice->SetAxes(main, off, slow, off2);
}

void CDasherInterfaceBridge::SetupUI() {
  NSLog(@"CDasherInterfaceBridge::SetupUI");
}

void CDasherInterfaceBridge::Realize() {  
  CDasherInterfaceBase::Realize();
  ExternalEventHandler(&CParameterNotificationEvent(SP_ALPHABET_ID)); //calls dasherApp::SetAlphabet
  CDasherInterfaceBase::OnUIRealised();
}

void CDasherInterfaceBridge::SetupPaths() {
  NSString *systemDir = [NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] bundlePath]];
  NSString *userDir = [NSString stringWithFormat:@"%@/", [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]];

  m_pSettingsStore->SetStringParameter(SP_SYSTEM_LOC, StdStringFromNSString(systemDir));
  m_pSettingsStore->SetStringParameter(SP_USER_LOC, StdStringFromNSString(userDir));
}

void CDasherInterfaceBridge::CreateSettingsStore() {
  m_pSettingsStore = new COSXSettingsStore(m_pEventHandler);
}

void CDasherInterfaceBridge::ScanAlphabetFiles(std::vector<std::string> &vFileList) {
  
  NSDirectoryEnumerator *dirEnum;
  NSString *file;

  dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:NSStringFromStdString(GetStringParameter(SP_SYSTEM_LOC))];
  while (file = [dirEnum nextObject]) {
    if ([file hasSuffix:@".xml"] && [file hasPrefix:@"alphabet"]) {
      vFileList.push_back(StdStringFromNSString(file));
    }
  }  
  
  dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:NSStringFromStdString(GetStringParameter(SP_USER_LOC))];
  while (file = [dirEnum nextObject]) {
    if ([file hasSuffix:@".xml"] && [file hasPrefix:@"alphabet"]) {
      vFileList.push_back(StdStringFromNSString(file));
    }
  }  
}

void CDasherInterfaceBridge::ScanColourFiles(std::vector<std::string> &vFileList) {
  NSDirectoryEnumerator *dirEnum;
  NSString *file;
  
  dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:NSStringFromStdString(GetStringParameter(SP_SYSTEM_LOC))];
  while (file = [dirEnum nextObject]) {
    if ([file hasSuffix:@".xml"] && [file hasPrefix:@"colour"]) {
      vFileList.push_back(StdStringFromNSString(file));
    }
  }  
  
  dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:NSStringFromStdString(GetStringParameter(SP_USER_LOC))];
  while (file = [dirEnum nextObject]) {
    if ([file hasSuffix:@".xml"] && [file hasPrefix:@"colour"]) {
      vFileList.push_back(StdStringFromNSString(file));
    }
  }  
}


void CDasherInterfaceBridge::NewFrame(unsigned long iTime, bool bForceRedraw) {
  CDasherInterfaceBase::NewFrame(iTime, bForceRedraw);
}

void CDasherInterfaceBridge::StartTimer() {
  [dasherApp startTimer];
}

void CDasherInterfaceBridge::ShutdownTimer() {
  [dasherApp shutdownTimer];
}
 
void CDasherInterfaceBridge::GameMessageOut(int message, const void* messagedata) {
  NSLog(@"GameMessageOut");
}

void CDasherInterfaceBridge::ExternalEventHandler(Dasher::CEvent *pEvent) {
  
  switch (pEvent->m_iEventType) {
    case EV_PARAM_NOTIFY:
      // don't need to do anything because the PreferencesController is observing changes to the 
      // user defaults controller which is observing the user defaults and will be notified when
      // the parameter is actually written by COSXSettingsStore.
//      CParameterNotificationEvent *parameterEvent(static_cast < CParameterNotificationEvent * >(pEvent));
//      NSLog(@"CParameterNotificationEvent, m_iParameter: %d", parameterEvent->m_iParameter);
	  {
		Dasher::CParameterNotificationEvent *pEvt(static_cast<Dasher::CParameterNotificationEvent *>(pEvent));
		if (pEvt->m_iParameter == LP_MAX_BITRATE || pEvt->m_iParameter == LP_BOOSTFACTOR)
			[dasherApp notifySpeedChange];
    else if (pEvt->m_iParameter == SP_ALPHABET_ID)
      [dasherApp setAlphabet:GetActiveAlphabet()];
    }
      break;
    case EV_EDIT:
	  {
//      NSLog(@"ExternalEventHandler, m_iEventType = EV_EDIT");
		  CEditEvent *editEvent((CEditEvent *)pEvent);
      switch (editEvent->m_iEditType) {
        case 1:
          //NSLog(@"ExternalEventHandler edit insert");
          [dasherApp outputCallback:NSStringFromStdString(editEvent->m_sText)];
          break;
        case 2:
          //NSLog(@"ExternalEventHandler edit delete");
          [dasherApp deleteCallback:NSStringFromStdString(editEvent->m_sText)];
          break;
        case 10:
          NSLog(@"ExternalEventHandler edit convert");
          break;
        case 11:
          NSLog(@"ExternalEventHandler edit protect");
          break;
        default:
          break;
      }
	  }
        break;
   case EV_SCREEN_GEOM:
     //no need to do anything
     break;
    default:
      NSLog(@"ExternalEventHandler, UNKNOWN m_iEventType = %d", pEvent->m_iEventType);
      break;
  }
  
}

void CDasherInterfaceBridge::Message(const string &strMessage) {
  [dasherApp displayMessage:NSStringFromStdString(strMessage)];
}

void CDasherInterfaceBridge::SetLockStatus(const string &strText, int iPercent) {
  NSString *dispMsg = nil;
  if (iPercent != -1) {
    dispMsg = NSStringFromStdString(strText);
    if (iPercent) dispMsg = [NSString stringWithFormat:@"%@ (%i%%)", dispMsg, iPercent];
  }
  [dasherApp setLockText:dispMsg];
  //Call superclass too. Probably unnecessary, as no frames'll be rendered..!
  CDasherInterfaceBase::SetLockStatus(strText, iPercent);
}

void CDasherInterfaceBridge::CopyToClipboard(const std::string &strText) {
  [dasherApp copy:NSStringFromStdString(strText)];
}

bool CDasherInterfaceBridge::SupportsSpeech() {
  return [dasherApp supportsSpeech];
}

void CDasherInterfaceBridge::Speak(const std::string &strText, bool bInterrupt) {
  [dasherApp speak:NSStringFromStdString(strText) interrupt:bInterrupt];  
}

void CDasherInterfaceBridge::ClearAllContext() {
  [dasherApp clearText];
}

string CDasherInterfaceBridge::GetAllContext() {
  return StdStringFromNSString([dasherApp allText]);
}

string CDasherInterfaceBridge::GetContext(unsigned int iOffset, unsigned int iLength) {
  return StdStringFromNSString([dasherApp textAtOffset:iOffset Length:iLength]);
}

unsigned int CDasherInterfaceBridge::ctrlMove(bool bForwards, CControlManager::EditDistance dist) {
  return [dasherApp move:dist forwards:bForwards];
}

unsigned int CDasherInterfaceBridge::ctrlDelete(bool bForwards, CControlManager::EditDistance dist) {
  return [dasherApp del:dist forwards:bForwards];
}

int CDasherInterfaceBridge::GetFileSize(const std::string &strFileName) {
  int ret=0;
  if (FILE *file = fopen(strFileName.c_str(), "r")) { //returns non-null for success
    if (!fseek(file, 0, SEEK_END)) //returns non-null as error code
      ret = ftell(file);
    fclose(file);
  }
  return ret;
}

void CDasherInterfaceBridge::WriteTrainFile(const std::string &filename,const std::string &strNewText) {
  if(strNewText.length() == 0)
    return;
  
  std::string strFilename(GetStringParameter(SP_USER_LOC) + filename);
  
  NSLog(@"Write train file: %s", strFilename.c_str());
  
  int fd=open(strFilename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,strNewText.c_str(),strNewText.length());
  close(fd);
}

/*NSDictionary *CDasherInterfaceBridge::ParameterDictionary() {
  COSXSettingsStore *ss(static_cast < COSXSettingsStore * >(m_pSettingsStore));
  return ss->ParameterDictionary();
}

id CDasherInterfaceBridge::GetParameter(NSString *aKey) {
  
  COSXSettingsStore *ss(static_cast < COSXSettingsStore * >(m_pSettingsStore));
  int pIndex = ss->GetParameterIndex(StdStringFromNSString(aKey));

  switch (ss->GetParameterType(pIndex)) {
    case ParamBool:
      return [NSNumber numberWithBool:GetBoolParameter(pIndex)];
      break;
    case ParamLong:
      return [NSNumber numberWithLong:GetLongParameter(pIndex)];
      break;
    case ParamString:
      return NSStringFromStdString(GetStringParameter(pIndex));
      break;
    default:
      break;
  }
  
  return nil;
}

void CDasherInterfaceBridge::SetParameter(NSString *aKey, id aValue) {
  
  COSXSettingsStore *ss(static_cast < COSXSettingsStore * >(m_pSettingsStore));
  int pIndex = ss->GetParameterIndex(StdStringFromNSString(aKey));
  
  switch (ss->GetParameterType(pIndex)) {
    case ParamBool:
      SetBoolParameter(pIndex, [aValue boolValue]);
      break;
    case ParamLong:
      SetLongParameter(pIndex, [aValue longValue]);
      break;
    case ParamString:
      SetStringParameter(pIndex, StdStringFromNSString(aValue));
      break;
    default:
      break;
  }
}*/
