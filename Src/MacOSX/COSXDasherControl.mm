/*
 *  COSXDasherControl.cpp
 *  Dasher
 *
 *  Created by Doug Dickinson on 17/11/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import "COSXDasherControl.h"
#import "COSXSettingsStore.h"
#import "ModuleManager.h"
#import "COSXMouseInput.h"
#import "DasherUtil.h"
#import "DasherApp.h"
#import "DasherEdit.h"
#import "Event.h"
#import "../Common/Common.h"

#import <iostream>

#import <fcntl.h>

#import <sys/stat.h>

using namespace std;


COSXDasherControl::COSXDasherControl(DasherApp *aDasherApp) {
  
  dasherApp = aDasherApp;
  
  dasherEdit = [[DasherEdit alloc] init];

  
}

void COSXDasherControl::CreateModules() {
  CDasherInterfaceBase::CreateModules();
  // Create locally cached copies of the mouse input objects, as we
  // need to pass coordinates to them from the timer callback
  RegisterModule(m_pMouseInput = new COSXMouseInput(m_pEventHandler, m_pSettingsStore));
  SetDefaultInputDevice(m_pMouseInput);
  RegisterModule(m_p1DMouseInput = new COSX1DMouseInput(m_pEventHandler, m_pSettingsStore));
}
  
COSXDasherControl::~COSXDasherControl() {
  if(m_pMouseInput) {
    m_pMouseInput = NULL;
  }
  
  if(m_p1DMouseInput) {
    m_p1DMouseInput = NULL;
  }
}

void COSXDasherControl::SetupUI() {
  NSLog(@"COSXDasherControl::SetupUI");
}

// TODO: hack to get around things being used before they were created.  is there a better way?
// TODO: maybe this should go in setupui?
void COSXDasherControl::Realize2() {
  CDasherInterfaceBase::Realize();
  OnUIRealised();
}

void COSXDasherControl::SetupPaths() {

  NSString *systemDir = [NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] resourcePath]];
  NSString *userDir = [NSString stringWithFormat:@"%@/Library/Application Support/Dasher/", NSHomeDirectory()];
  
  // if the userDir doesn't exist, create it, ready to receive stuff
  if (![[NSFileManager defaultManager] fileExistsAtPath:userDir isDirectory:NULL]) {
    (void)[[NSFileManager defaultManager] createDirectoryAtPath:userDir attributes:nil];
  }
    
    // system resources are inside the .app, under the Resources directory
  m_pSettingsStore->SetStringParameter(SP_SYSTEM_LOC, StdStringFromNSString(systemDir));
  m_pSettingsStore->SetStringParameter(SP_USER_LOC, StdStringFromNSString(userDir));
}

void COSXDasherControl::CreateSettingsStore() {
  m_pSettingsStore = new COSXSettingsStore(m_pEventHandler);
}


void COSXDasherControl::ScanAlphabetFiles(std::vector<std::string> &vFileList) {
  
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

void COSXDasherControl::ScanColourFiles(std::vector<std::string> &vFileList) {
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


void COSXDasherControl::goddamn(unsigned long iTime, bool bForceRedraw) {
  NewFrame(iTime, bForceRedraw);
}

void COSXDasherControl::StartTimer() {
  [dasherApp startTimer];
}

void COSXDasherControl::ShutdownTimer() {
  [dasherApp shutdownTimer];
}

void COSXDasherControl::TimerFired(NSPoint p) {
  m_pMouseInput->SetCoordinates(p.x, p.y);
  m_p1DMouseInput->SetCoordinates(p.x, p.y);
  
  [[dasherApp dasherView] redisplay];
}  
 
void COSXDasherControl::GameMessageOut(int message, const void* messagedata) {
  NSLog(@"GameMessageOut");
}

void COSXDasherControl::ExternalEventHandler(Dasher::CEvent *pEvent) {
  
  switch (pEvent->m_iEventType) {
    case EV_PARAM_NOTIFY:
      // don't need to do anything because the PreferencesController is observing changes to the 
      // user defaults controller which is observing the user defaults and will be notified when
      // the parameter is actually written by COSXSettingsStore.
//      CParameterNotificationEvent *parameterEvent(static_cast < CParameterNotificationEvent * >(pEvent));
//      NSLog(@"CParameterNotificationEvent, m_iParameter: %d", parameterEvent->m_iParameter);
      break;
    case EV_EDIT: {
//      NSLog(@"ExternalEventHandler, m_iEventType = EV_EDIT");
      CEditEvent *editEvent(static_cast < CEditEvent * >(pEvent));
      switch (editEvent->m_iEditType) {
        case 1:
          //NSLog(@"ExternalEventHandler edit insert");
          [dasherEdit outputCallback:NSStringFromStdString(editEvent->m_sText) targetApp:[dasherApp targetAppUIElementRef]];
          break;
        case 2:
         // NSLog(@"ExternalEventHandler edit delete");
          [dasherEdit deleteCallback:NSStringFromStdString(editEvent->m_sText) targetApp:[dasherApp targetAppUIElementRef]];
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
      break;
    }
    case EV_LOCK:
//      CLockEvent *lockEvent(static_cast < CLockEvent * >(pEvent));
//      NSLog(@"ExternalEventHandler, m_iEventType = EV_LOCK, mess: %@, bLock = %d, pct = %d", NSStringFromStdString(lockEvent->m_strMessage), lockEvent->m_bLock, lockEvent->m_iPercent);
      break;
    case EV_MESSAGE: {
      CMessageEvent *messageEvent(static_cast < CMessageEvent * >(pEvent));
      NSLog(@"ExternalEventHandler, m_iEventType = EV_MESSAGE, mess: %@, id = %d, type = %d", NSStringFromStdString(messageEvent->m_strMessage), messageEvent->m_iID, messageEvent->m_iType);
      break;
    }
    case EV_SCREEN_GEOM:
      //no need to do anything, so avoid log message
      break;
    default:
      NSLog(@"ExternalEventHandler, UNKNOWN m_iEventType = %d", pEvent->m_iEventType);
      break;
  }
  
}

unsigned int COSXDasherControl::ctrlMove(bool bForwards, CControlManager::EditDistance dist) {
#ifdef DEBUG
  std::cout << "Call to edit move, doing nothing" << std::endl;
#endif
  return [[dasherEdit allContext] length];
}

unsigned int COSXDasherControl::ctrlDelete(bool bForwards, CControlManager::EditDistance dist) {
  if (!bForwards) {
    const unsigned int iCurrentOffset([[dasherEdit allContext] length]);
    int iStartOffset;
    if (dist==CControlManager::EDIT_CHAR)
      iStartOffset=(iCurrentOffset==0) ? 0 : iCurrentOffset-1;
    else if (dist==CControlManager::EDIT_FILE)
      iStartOffset=0;
    else {
      const CAlphInfo *pAlph(GetInfo(GetStringParameter(SP_ALPHABET_ID)));
      const string &target(pAlph->GetText(dist==CControlManager::EDIT_WORD ? pAlph->GetSpaceSymbol() : pAlph->GetParagraphSymbol()));
      NSRange range = [[dasherEdit allContext] rangeOfString:NSStringFromStdString(target) options:NSBackwardsSearch];
      iStartOffset = (range.length==0) ? 0 : range.location; //0=> not found, so go to beginning
    }
    DASHER_ASSERT(iStartOffset<=[[dasherEdit allContext] length]);
    [dasherEdit deleteCallback:[[dasherEdit allContext] substringFromIndex:iStartOffset] targetApp:[dasherApp targetAppUIElementRef]];
  }
  return [[dasherEdit allContext] length];
}

int COSXDasherControl::GetFileSize(const std::string &strFileName) {
  struct stat sStatInfo;
  
  if(!stat(strFileName.c_str(), &sStatInfo))
    return sStatInfo.st_size;
  else
    return 0;
}

void COSXDasherControl::Train(NSString *fileName) {
  std::string f = StdStringFromNSString(fileName);
  NSLog(@"Read train file: %s", f.c_str());
  CDasherInterfaceBase::ImportTrainingText(f);
}

void COSXDasherControl::WriteTrainFile(const std::string &filename, const std::string &strNewText) {
  if(strNewText.length() == 0)
    return;
  
  std::string strFilename(GetStringParameter(SP_USER_LOC) + filename);
  
  NSLog(@"Write train file: %s", strFilename.c_str());
  
  int fd=open(strFilename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,strNewText.c_str(),strNewText.length());
  close(fd);
}

NSDictionary *COSXDasherControl::ParameterDictionary() {
  COSXSettingsStore *ss(static_cast < COSXSettingsStore * >(m_pSettingsStore));
  return ss->ParameterDictionary();
}

id COSXDasherControl::GetParameter(NSString *aKey) {
  
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

void COSXDasherControl::SetParameter(NSString *aKey, id aValue) {
  
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
}

bool COSXDasherControl::SupportsSpeech() {
  return [dasherApp supportsSpeech];
}

void COSXDasherControl::Speak(const std::string &strText, bool bInterrupt) {
  [dasherApp speak:NSStringFromStdString(strText) interrupt:bInterrupt];
}

void COSXDasherControl::CopyToClipboard(const std::string &strText) {
  [dasherApp copyToClipboard:NSStringFromStdString(strText)];
}

std::string COSXDasherControl::GetContext(unsigned int iOffset, unsigned int iLength) {
  return StdStringFromNSString([dasherEdit textAtOffset:iOffset Length:iLength]);
}

std::string COSXDasherControl::GetAllContext() {
  return StdStringFromNSString([dasherEdit allContext]);
}

void COSXDasherControl::ClearAllContext() {
  [dasherEdit clearContext];
  CDasherInterfaceBase::ClearAllContext();
}


