//
//  KeyboardEvent.m
//  TestAcc
//
//  Created by dougie on 20/09/2004.
//  Copyright 2004 __MyCompanyName__. All rights reserved.
//

#import "KeyboardEvent.h"
#import <Carbon/Carbon.h>

static UInt32 modifierKeyMask[] = {shiftKey, optionKey, controlKey};
static CGKeyCode modifierKeyCode[] = {56, 58, 59};  // ASSUMING: THESE ARE THE SAME FOR ALL KB'S!!!
static int modifierKeyMaskLen = sizeof(modifierKeyMask) / sizeof(UInt32);

@implementation KeyboardEvent


- (void)postToUIElementRef:(AXUIElementRef)aUIElementRef charCode:(CGCharCode)aCharCode keyCode:(CGKeyCode)aKeyCode keyDown:(Boolean)isKeyDown
{
  AXError error;
  
//  NSLog(@"posting: charCode:%d keyCode:%d %@", aCharCode, aKeyCode, isKeyDown ? @"down" : @"u
    error = AXUIElementPostKeyboardEvent(aUIElementRef, (CGCharCode)aCharCode, (CGKeyCode)aKeyCode, NSKeyDown);
}

- (void)postToPID:(int)pid keyCode:(CGKeyCode)aKeyCode modifierKeyCode:(CGKeyCode)modifyCode keyDown:(BOOL)isKeyDown
{
    if (isKeyDown) {
        CGEventRef keyDownEvent = CGEventCreateKeyboardEvent(NULL, aKeyCode, true);
        
        // Shift
        if (modifyCode == 56) {
            CGEventSetFlags(keyDownEvent, kCGEventFlagMaskShift);
        }
        // Option (Alt)
        else if (modifyCode == 58) {
            CGEventSetFlags(keyDownEvent, kCGEventFlagMaskAlternate);
        }
        // Control
        else if (modifyCode == 59) {
            CGEventSetFlags(keyDownEvent, kCGEventFlagMaskControl);
        }
        CGEventPostToPid(pid, keyDownEvent);
        CFRelease(keyDownEvent);
    } else {
        CGEventRef keyUpEvent = CGEventCreateKeyboardEvent(NULL, aKeyCode, false);
        // Shift
        if (modifyCode == 56) {
            CGEventSetFlags(keyUpEvent, kCGEventFlagMaskShift);
        }
        // Option (Al
        else if (modifyCode == 58) {
            CGEventSetFlags(keyUpEvent, kCGEventFlagMaskAlternate);
        }
        // Control
        else if (modifyCode == 59) {
            CGEventSetFlags(keyUpEvent, kCGEventFlagMaskControl);
        }
        CGEventPostToPid(pid, keyUpEvent);
        CFRelease(keyUpEvent);
    }
}

- (void)postToUIElementRef:(AXUIElementRef)aUIElementRef
{
  int i, j;
  
  for (i = 0; i < _eventCount; i++)
   {
    for (j = 0; j < modifierKeyMaskLen; j++)
     {
      if (_modifierStates[i] & modifierKeyMask[j])
       {
        [self postToUIElementRef:aUIElementRef charCode:0 keyCode:modifierKeyCode[j] keyDown:true];
       }
     }
    
    [self postToUIElementRef:aUIElementRef charCode:0 keyCode:_keyCodes[i] keyDown:true];
    [self postToUIElementRef:aUIElementRef charCode:0 keyCode:_keyCodes[i] keyDown:false];
    
    for (j = modifierKeyMaskLen - 1; j >= 0; --j)
     {
      if (_modifierStates[i] & modifierKeyMask[j])
       {
        [self postToUIElementRef:aUIElementRef charCode:0 keyCode:modifierKeyCode[j] keyDown:false];
       }
     }    
   }
}

- (void)postToPID:(int)pid {
    int i, j;
    for (i = 0; i < _eventCount; i++) {
        BOOL hasModifyKey = false;
        CGKeyCode keycode = _keyCodes[i];
        for (j = 0; j < modifierKeyMaskLen; j++) {
            if (_modifierStates[i] & modifierKeyMask[j]) {
                hasModifyKey = true;
                [self postToPID:pid keyCode: keycode modifierKeyCode: modifierKeyCode[j] keyDown:true];
                [self postToPID:pid keyCode: keycode modifierKeyCode: modifierKeyCode[j]  keyDown:false];
            }
        }
        
        if (!hasModifyKey) {
            [self postToPID:pid keyCode: keycode modifierKeyCode: 0 keyDown:true];
            [self postToPID:pid keyCode: keycode modifierKeyCode: 0 keyDown:false];
        }
    }
}

+ (id)keyboardEventWithUnicharString:(NSString *)aUnicharSring keyCodes:(UInt16 *)aKeyCodeList modifierStates:(UInt32 *)aModiferStateList count:(int)aCount
{
  return [[[self alloc] initWithUnicharString:aUnicharSring keyCodes:aKeyCodeList modifierStates:aModiferStateList count:aCount] autorelease];
}


- (id)initWithUnicharString:(NSString *)aUnicharString keyCodes:(UInt16 *)aKeyCodeList modifierStates:(UInt32 *)aModiferStateList count:(int)aCount 
{
  if (self = [super init]) 
   {
    [self setUnicharString:aUnicharString];
    [self setKeyCodes:aKeyCodeList count:aCount];
    [self setModifierStates:aModiferStateList count:aCount];
    [self setEventCount:aCount];
   }
  
  return self;
}

- (NSString *)description
{
  return [NSString stringWithFormat:@"%@: %@", [self unicharString], [self keyCodes]];
}

- (NSString *)unicharString {
  return _unicharString;
}

- (void)setUnicharString:(NSString *)newUnicharString {
  if (_unicharString != newUnicharString)
   {
    [_unicharString release];
    _unicharString = [newUnicharString retain];
   }
}

- (UInt16 *)keyCodes
{
  return _keyCodes;
}

- (void)setKeyCodes:(UInt16 *)aKeyCodeList count:(int)aCount
{
  if (_keyCodes)
   {
    free(_keyCodes);
    _keyCodes = NULL;
   }
  
  if (aKeyCodeList && aCount)
   {
    _keyCodes = malloc(sizeof(UInt16) * aCount);
    if (_keyCodes)
     {
      memcpy(_keyCodes, aKeyCodeList, sizeof(UInt16) * aCount);
     }
   }
}

- (UInt32 *)modifierStates
{
  return _modifierStates;
}

- (void)setModifierStates:(UInt32 *)aModifierStateList count:(int)aCount
{
  if (_modifierStates)
   {
    free(_modifierStates);
    _modifierStates = NULL;
   }
  
  if (aModifierStateList && aCount)
   {
    _modifierStates = malloc(sizeof(UInt32) * aCount);
    if (_modifierStates)
     {
      memcpy(_modifierStates, aModifierStateList, sizeof(UInt32) * aCount);
     }
   }
}

- (int)eventCount
{
  return _eventCount;
}

- (void)setEventCount:(int)aCount
{
  _eventCount = aCount;
}



- (void)dealloc {
  [_unicharString release];
  [self setKeyCodes:NULL count:0];  
  [self setModifierStates:NULL count:0];
  
  [super dealloc];
}



@end
