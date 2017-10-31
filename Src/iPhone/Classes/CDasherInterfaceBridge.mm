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
#import "EAGLView.h"
#import "GameModule.h"
#import "../Common/Globber.h"
#import <iostream>
#import <fcntl.h>

#import <sys/stat.h>

//declare some "friend" methods
@interface DasherAppDelegate ()
-(EAGLView *)glView;
-(UIWebView *)getWebView;
@end;

using namespace Dasher;
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

CDasherInterfaceBridge::CDasherInterfaceBridge(DasherAppDelegate *aDasherApp) : CDashIntfScreenMsgs(new COSXSettingsStore(), &m_fileUtils),
dasherApp(aDasherApp),
userPath([[NSString stringWithFormat:@"%@/", [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]] retain]) {
}

void CDasherInterfaceBridge::CreateModules() {
	//don't create the default set...just the ones accessible from the GUI.
  RegisterModule(m_pUndoubledTouch = new UndoubledTouch([dasherApp glView]));
	RegisterModule(m_pMouseDevice = 
				new CIPhoneMouseInput([dasherApp glView]));
	RegisterModule(m_pTiltDevice = 
				new CIPhoneTiltInput());
  RegisterModule(m_pTwoFingerDevice=new CIPhoneTwoFingerInput([dasherApp glView]));
  SetDefaultInputDevice(m_pMouseDevice);
                 
  RegisterModule(new CButtonMode(this, this, true, 9, "Menu Mode"));
  RegisterModule(new CButtonMode(this, this, false, 8, "Direct Mode"));
  RegisterModule(new CTwoButtonDynamicFilter(this, this, m_pFramerate));
  RegisterModule(new CTwoPushDynamicFilter(this, this, m_pFramerate));
  
	RegisterModule(new CIPhoneTiltFilter(this, this, m_pFramerate, 16, m_pMouseDevice));
  //Touch filter is stylus filter with optional Tilt X....
  CIPhoneTouchFilter *pTouchFilter = 
          new CIPhoneTouchFilter(this, this, m_pFramerate, 17, m_pUndoubledTouch, m_pTiltDevice);
	RegisterModule(pTouchFilter);
	SetDefaultInputMethod(pTouchFilter);
  
  RegisterModule(new CIPhoneTwoFingerFilter(this,this, m_pFramerate, 18));
}
	
CDasherInterfaceBridge::~CDasherInterfaceBridge() {
  delete m_pMouseDevice;
	delete m_pTiltDevice;
  delete m_pTwoFingerDevice;
  delete m_pUndoubledTouch;
  //(ACL)registered input filters should be automatically free'd by the module mgr?
  [userPath release];
}

void CDasherInterfaceBridge::SetTiltAxes(Vec3 main, float off, Vec3 slow, float off2)
{
	m_pTiltDevice->SetAxes(main, off, slow, off2);
}

void CDasherInterfaceBridge::Realize() {
  CDashIntfScreenMsgs::Realize(get_time());
  
  [dasherApp setAlphabet:GetActiveAlphabet()];
  //don't call HandleEvent, would call superclass and reconstruct the NCManager!
  //TODO maybe better to make ChangeAlphabet virtual and override that?  
    dispatch_async(dispatch_get_main_queue(), ^{
    
  [dasherApp glView].animating=YES;
    });
}

/*
void CDasherInterfaceBridge::ScanFiles(AbstractParser *parser, const std::string &strPattern) {

  string strPath(StdStringFromNSString([[NSBundle mainBundle] bundlePath])+"/"+strPattern);
  const char *sys[2];
  sys[0] = strPath.c_str();
  sys[1] = NULL;
  
  const char *user[2]; user[1] = NULL;
  if ([[NSFileManager defaultManager] fileExistsAtPath:userPath isDirectory:NULL]) {
    user[0] = (StdStringFromNSString(userPath)+strPattern).c_str();
  } else {
    // userDir doesn't exist => create it, ready to receive stuff
    (void)[[NSFileManager defaultManager] createDirectoryAtPath:userPath withIntermediateDirectories:YES attributes:nil error:nil];
    user[0] = 0;
  }
  globScan(parser, user, sys);

}
*/
/*void CDasherInterfaceBridge::ScanForFiles(AbstractFileParser *parser, const std::string &strName) {
  NSFileManager *mgr = [NSFileManager defaultManager];
  NSArray *names = [[mgr enumeratorAtPath:systemPath] allObjects];
  if ([names containsObject:NSStringFromStdString(strName)])
      parser->ParseFile(StdStringFromNSString(systemPath)+strName,false);
  
  names = [[mgr enumeratorAtPath:userPath] allObjects];
  if ([names containsObject:NSStringFromStdString(strName)])
    parser->ParseFile(StdStringFromNSString(userPath)+strName,true);
}*/

void CDasherInterfaceBridge::NewFrame(unsigned long iTime, bool bForceRedraw) {
  CDashIntfScreenMsgs::NewFrame(iTime, bForceRedraw);
}

void CDasherInterfaceBridge::HandleEvent(int iParameter) {
  
  // don't need to do anything because the PreferencesController is observing changes to the 
  // user defaults controller which is observing the user defaults and will be notified when
  // the parameter is actually written by COSXSettingsStore.
  //NSLog(@"CParameterNotificationEvent, m_iParameter: %d", parameterEvent->m_iParameter);
  if (iParameter == LP_MAX_BITRATE)
    [dasherApp notifySpeedChange];
  else if (iParameter == SP_ALPHABET_ID)
    [dasherApp setAlphabet:GetActiveAlphabet()];
  CDashIntfScreenMsgs::HandleEvent(iParameter);
}

void CDasherInterfaceBridge::EnterGameMode(CGameModule *pGameModule) {
  CDashIntfScreenMsgs::EnterGameMode(pGameModule);
  [dasherApp refreshToolbar];
}

void CDasherInterfaceBridge::LeaveGameMode() {
  CDashIntfScreenMsgs::LeaveGameMode();
  [dasherApp refreshToolbar];
}

void CDasherInterfaceBridge::editOutput(const string &strText, CDasherNode *pNode) {
//NSLog(@"ExternalEventHandler edit insert");
  [dasherApp outputCallback:NSStringFromStdString(strText)];
  CDashIntfScreenMsgs::editOutput(strText,pNode);
}

void CDasherInterfaceBridge::editDelete(const string &strText, CDasherNode *pNode) {
// NSLog(@"ExternalEventHandler edit delete");
  [dasherApp deleteCallback:NSStringFromStdString(strText)];
  CDashIntfScreenMsgs::editDelete(strText, pNode);
}

void CDasherInterfaceBridge::editConvert(CDasherNode *pSource) {
  NSLog(@"ExternalEventHandler edit convert");
  CDashIntfScreenMsgs::editConvert(pSource);
}

void CDasherInterfaceBridge::editProtect(CDasherNode *pSource) {
  NSLog(@"ExternalEventHandler edit protect");
  CDashIntfScreenMsgs::editProtect(pSource);
}

void CDasherInterfaceBridge::Message(const string &strMessage, bool bInterrupt) {
  NSLog(@"Message: %s", strMessage.c_str());
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
  CDashIntfScreenMsgs::SetLockStatus(strText, iPercent);
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

/// Subclasses should return the length of whole text. In letters, not bytes.
int CDasherInterfaceBridge::GetAllContextLenght() {
  return (int)[[dasherApp allText] length];
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
    
  std::string strFilename(StdStringFromNSString((const NSString*)userPath) + filename);
  
  NSLog(@"Write train file: %s", strFilename.c_str());
  
  int fd=open(strFilename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,strNewText.c_str(),strNewText.length());
  close(fd);
}

CGameModule *CDasherInterfaceBridge::CreateGameModule() {
  return new IPhoneGameModule(this, this, GetView(), m_pDasherModel, [dasherApp getWebView]);  
}
