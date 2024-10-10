//
//  KeyboardEvent.h
//  TestAcc
//
//  Created by dougie on 20/09/2004.
//  Copyright 2004 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface KeyboardEvent : NSObject 
{
  NSString *_unicharString;
  UInt16 *_keyCodes;
  UInt32 *_modifierStates;
  int _eventCount;
}

- (void)postToUIElementRef:(AXUIElementRef)aUIElementRef charCode:(CGCharCode)aCharCode keyCode:(CGKeyCode)aKeyCode keyDown:(Boolean)isKeyDown;
- (void)postToPID:(int)pid keyCode:(CGKeyCode)aKeyCode modifierKeyCode:(CGKeyCode)modifyCode keyDown:(BOOL)isKeyDown;
- (void)postToUIElementRef:(AXUIElementRef)aUIElementRef;
- (void)postToPID: (int)pid;
+ (id)keyboardEventWithUnicharString:(NSString *)aUnicharSring keyCodes:(UInt16 *)aKeyCodeList modifierStates:(UInt32 *)aModiferStateList count:(int)aCount;

- (id)initWithUnicharString:(NSString *)aUnicharSring keyCodes:(UInt16 *)aKeyCodeList modifierStates:(UInt32 *)aModiferStateList count:(int)aCount;

- (UInt16 *)keyCodes;
- (void)setKeyCodes:(UInt16 *)aKeyCodeList count:(int)aCount;

- (UInt32 *)modifierStates;
- (void)setModifierStates:(UInt32 *)aModifierStateList count:(int)aCount;

- (int)eventCount;
- (void)setEventCount:(int)aCount;

- (NSString *)unicharString;
- (void)setUnicharString:(NSString *)newUnichar;


@end
