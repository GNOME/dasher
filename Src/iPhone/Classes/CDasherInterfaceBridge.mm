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
#import "GameModule.h"
#import <iostream>
#import <fcntl.h>

#import <sys/stat.h>

using namespace std;

class IPhoneGameModule : public CGameModule {
public:
  IPhoneGameModule(Dasher::CSettingsUser *pCreateFrom, CDasherInterfaceBridge *pIntf, CDasherView *pView, CDasherModel *pModel, UIWebView *pWebView)
  : CGameModule(pCreateFrom,pIntf,pView,pModel), m_pWebView(pWebView), cachedStr(nil) {
  };
  ~IPhoneGameModule() {
  }
protected:
  virtual void HandleEvent(const CEditEvent *pEvt) {
    CGameModule::HandleEvent(pEvt);
    if (cachedStr) {
      [cachedStr release];
      cachedStr=nil;
    }
  }
  virtual void DrawText(CDasherView *pView) {
    if (cachedStr) return; //unchanged since last time!
    const vector<symbol> &target(targetSyms());
    string html;
    
    if (lastCorrectSym()>-1) {
      html="<span style=\"color:#0f0\">";
      for (int i=0; i<=lastCorrectSym(); i++)
        html+=m_pAlph->GetText(target[i]);
      html+="</span>";
    }
    //any wrongly entered text in red with strikethrough
    html+="<span style=\"color:#f00; text-decoration:line-through;\">"+m_strWrong+"</span><span id=\"here\">";
    //target still in black...with div id "here" which will be scrolled to the center
    for (int i=lastCorrectSym()+1; i<target.size(); i++)
      html+=m_pAlph->GetText(target[i]);
    html+="</span>";
    cachedStr = [NSStringFromStdString(html) retain];
    [m_pWebView loadHTMLString:cachedStr baseURL:[NSURL URLWithString:@"http://localhost/"]];
  }
private:
  NSString *cachedStr;
  UIWebView *m_pWebView;
};

CDasherInterfaceBridge::CDasherInterfaceBridge(DasherAppDelegate *aDasherApp) : CDashIntfScreenMsgs(new COSXSettingsStore()), dasherApp(aDasherApp) {
}

void CDasherInterfaceBridge::CreateModules() {
	//create the default set...a good idea?!?!

  RegisterModule(m_pUndoubledTouch = new UndoubledTouch());
	RegisterModule(m_pMouseDevice = 
				new CIPhoneMouseInput(this));
	RegisterModule(m_pTiltDevice = 
				new CIPhoneTiltInput());
  SetDefaultInputDevice(m_pMouseDevice);
                 
  RegisterModule(new CButtonMode(this, this, true, 9, "Menu Mode"));
  RegisterModule(new CButtonMode(this, this, false, 8, "Direct Mode"));
  RegisterModule(new CTwoButtonDynamicFilter(this, this));
  RegisterModule(new CTwoPushDynamicFilter(this, this));
  
	RegisterModule(m_pTiltFilter =
				   new CIPhoneTiltFilter(this, this, 16, m_pMouseDevice));
	RegisterModule(m_pTouchFilter = 
				   new CIPhoneTouchFilter(this, this, 17, m_pUndoubledTouch, m_pTiltDevice));
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
  CDasherInterfaceBase::Realize(get_time());
  HandleEvent(SP_ALPHABET_ID); //calls dasherApp::SetAlphabet
}

void CDasherInterfaceBridge::SetupPaths() {
  NSString *systemDir = [NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] bundlePath]];
  NSString *userDir = [NSString stringWithFormat:@"%@/", [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]];

  SetStringParameter(SP_SYSTEM_LOC, StdStringFromNSString(systemDir));
  SetStringParameter(SP_USER_LOC, StdStringFromNSString(userDir));
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

void CDasherInterfaceBridge::HandleEvent(int iParameter) {
  
  // don't need to do anything because the PreferencesController is observing changes to the 
  // user defaults controller which is observing the user defaults and will be notified when
  // the parameter is actually written by COSXSettingsStore.
  //NSLog(@"CParameterNotificationEvent, m_iParameter: %d", parameterEvent->m_iParameter);
  if (iParameter == LP_MAX_BITRATE || iParameter == LP_BOOSTFACTOR)
    [dasherApp notifySpeedChange];
  else if (iParameter == SP_ALPHABET_ID)
    [dasherApp setAlphabet:GetActiveAlphabet()];
  else if (iParameter == BP_GAME_MODE)
    [dasherApp refreshToolbar];
  CDasherInterfaceBase::HandleEvent(iParameter);
}

void CDasherInterfaceBridge::editOutput(const string &strText, CDasherNode *pNode) {
//NSLog(@"ExternalEventHandler edit insert");
  [dasherApp outputCallback:NSStringFromStdString(strText)];
  CDasherInterfaceBase::editOutput(strText,pNode);
}

void CDasherInterfaceBridge::editDelete(const string &strText, CDasherNode *pNode) {
// NSLog(@"ExternalEventHandler edit delete");
  [dasherApp deleteCallback:NSStringFromStdString(strText)];
  CDasherInterfaceBase::editDelete(strText, pNode);
}

void CDasherInterfaceBridge::editConvert(CDasherNode *pSource) {
  NSLog(@"ExternalEventHandler edit convert");
  CDasherInterfaceBase::editConvert(pSource);
}

void CDasherInterfaceBridge::editProtect(CDasherNode *pSource) {
  NSLog(@"ExternalEventHandler edit protect");
  CDasherInterfaceBase::editProtect(pSource);
}

void CDasherInterfaceBridge::Message(const string &strMessage, bool bInterrupt) {
  if (bInterrupt)
    CDashIntfScreenMsgs::Message(strMessage,true);
  else
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

CGameModule *CDasherInterfaceBridge::CreateGameModule(CDasherView *pView, CDasherModel *pModel) {
  return new IPhoneGameModule(this, this, pView, pModel, [dasherApp getWebView]);  
}
