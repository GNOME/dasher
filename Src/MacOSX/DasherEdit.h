//
//  DasherEdit.h
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <AppKit/AppKit.h>
#import "AppWatcher.h"
#import "ControlManager.h"

@protocol DasherEdit

- (void)outputCallback:(NSString *)aString;
- (void)deleteCallback:(NSString *)s;
- (unsigned int)currentCursorPos;
- (NSString *)textAtOffset:(unsigned int)iOffset Length:(unsigned int)iLength;
- (NSString *)allContext;
- (void)clearContext;
-(unsigned int)ctrlMove:(Dasher::CControlManager::EditDistance)dist forwards:(BOOL)bForwards;
-(unsigned int)ctrlDelete:(Dasher::CControlManager::EditDistance)dist forwards:(BOOL)bForwards;
@end

///Editing functions by sending keystrokes to named app. No picking up context:
/// stores text entered in Dasher and can delete (only) that; control-mode editing
/// functions not currently supported except for backwards deletion (we look for
/// space/paragraph characters and send corresponding # of (char-)deletes.)
/// so just return the current offset unchanged.
///Could try to send arrow keys or complex keypresses (option-delete=word,
/// line/file... = select then delete?) via LowLevelKeyboardHandling,
/// but at best we'd lose our context.
///Really this will have to wait for implementing the MacOSX input method API.
@interface DirectEdit : NSObject<DasherEdit> {
  NSMutableString *allTextEntered;
  AppWatcher *appWatcher;
  Dasher::CDasherInterfaceBase *pIntf;
}
-(id)initWithIntf:(Dasher::CDasherInterfaceBase *)_pIntf AppWatcher:(AppWatcher *)_appWatcher;
@end
