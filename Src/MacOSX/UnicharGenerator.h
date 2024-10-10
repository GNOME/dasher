//
//  UnicharGenerator.h
//  TestAcc
//
//  Created by dougie on 20/09/2004.
//  Copyright 2004 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

@interface UnicharGenerator : NSObject
{
  NSMutableDictionary *_unicharLookup;
  KeyboardLayoutRef _currentKeyboardLayoutRef;
}

+ (id)sharedInstance;

- (void)postKeyboardEventsToUIElementRef:(AXUIElementRef)aUIElementRef unicharString:(NSString *)aUnicharString;
- (void)postKeyboardEventsToPID:(int)pid unicharString:(NSString *)aUnicharString;
- (void)fillWithDeadKeyState:(UInt32)aDeadKeyState keyCodes:(UInt16 *)aKeyCodeList modifierStates:(UInt32 *)aModiferStateList count:(int)anEventCount;

- (void)populateUnicharLookup;

- (NSMutableDictionary *)unicharLookup;
- (void)setUnicharLookup:(NSMutableDictionary *)newUnicharLookup;

- (KeyboardLayoutRef)currentKeyboardLayoutRef;
- (void)setCurrentKeyboardLayoutRef:(KeyboardLayoutRef)newKeyboardLayoutRef;


@end
