//
//  UnicharGenerator.m
//  TestAcc
//
//  Created by dougie on 20/09/2004.
//  Copyright 2004 __MyCompanyName__. All rights reserved.
//

#import "UnicharGenerator.h"
#import "KeyboardEvent.h"
#import "LowLevelKeyboardHandling.h"

static UnicharGenerator *_sharedInstance = NULL;

@implementation UnicharGenerator

+ (id)unicharGenerator
{
  return [[[self alloc] init] autorelease];
}

- (id)init
{
  if (self = [super init])
   {
    [self setUnicharLookup:[NSMutableDictionary dictionary]];
   }
  
  return self;
}

+ (id)sharedInstance
{
  if (!_sharedInstance)
   {
    _sharedInstance = [[self unicharGenerator] retain];
   }
  
  return _sharedInstance;
}

- (void)postKeyboardEventsToUIElementRef:(AXUIElementRef)aUIElementRef unicharString:(NSString *)aUnicharString
{
  if (keyboardMappingHasChanged())
   {
    [self populateUnicharLookup];
   }
  
  [[[self unicharLookup] objectForKey:aUnicharString] postToUIElementRef:aUIElementRef];
}

- (void)postKeyboardEventsToPID:(int)pid unicharString:(NSString *)aUnicharString
{
  if (keyboardMappingHasChanged())
  {
    [self populateUnicharLookup];
  }
  
  NSLog(@"aUnicharString: %@", aUnicharString);
  if (aUnicharString && aUnicharString.length > 0) {
    
    // For most symbols, we look up the keyboard event corresponding to the unicode and send that to the target process.
    // This generally works well across all alphabets, regardless of the computer's keyboard layout.
    // However, for certain symbols listed here, this method produces incorrect results, e.g. two characters instead of one. For these,
    // we use a keydown/keyup approach instead.
    NSArray *symbols = [NSArray arrayWithObjects: @"€", @"›", @"‹", @"’", @"·", @"”", @"«", @"»", @"„", @"‚", @"Ç", @"Í", @"Ï", @"Î", @"Ó", @"Ò", @"Æ", @"Ø", @"Å", @"¿", @"°", @"—", nil];
    
    NSMutableDictionary *dict = [self unicharLookup];
    KeyboardEvent *event = [dict objectForKey:aUnicharString];

    if (event && ![symbols containsObject: aUnicharString]) {
      [event postToPID: pid];
    } else {
      
      // If we can't find keyboard event for unichar, we will convert unicode using compsedRange and send it to the active process with keydown & keyup.
      NSRange composedRange = [aUnicharString rangeOfComposedCharacterSequencesForRange:NSMakeRange(0, aUnicharString.length)];
      unichar *characters = malloc(sizeof(unichar) * composedRange.length);
      [aUnicharString getCharacters:characters range:composedRange];
  
      CGEventRef keyDownEvent = CGEventCreateKeyboardEvent( nil, 0, true );
      CGEventKeyboardSetUnicodeString(keyDownEvent, composedRange.length, characters);
      CGEventPostToPid(pid, keyDownEvent);
      CFRelease(keyDownEvent);
      CGEventRef keyUpEvent = CGEventCreateKeyboardEvent( nil, 0, false);
      CGEventKeyboardSetUnicodeString(keyUpEvent, composedRange.length, characters);
      CGEventPostToPid(pid, keyUpEvent);
      CFRelease(keyUpEvent);
    }
  }
}

- (void)populateUnicharLookup
{
  UInt16 keyCodes[10];
  UInt32 modifierStates[10];
  
  [self setUnicharLookup:[NSMutableDictionary dictionary]];
  
  // TODO: is there a better way to do this (like with a configuration file?)
  // need to make sure newline is keyCode 36 -- is it 36 on EVERY keyboard????
  // 36 generates unichar 0x0d, whereas \n is 0x0a!!
  // I notice in AlphIO.cpp there's some ifdefing around windows handling for this, maybe
  // all that should be moved to platform specific configuration files????
  keyCodes[0] = 36;
  modifierStates[0] = 0;
  [[self unicharLookup] setObject:[KeyboardEvent keyboardEventWithUnicharString:@"\n" keyCodes:keyCodes modifierStates:modifierStates count:1] forKey:@"\n"];

  [self fillWithDeadKeyState:0 keyCodes:keyCodes modifierStates:modifierStates count:0];
}

- (void)fillWithDeadKeyState:(UInt32)aDeadKeyState keyCodes:(UInt16 *)aKeyCodeList modifierStates:(UInt32 *)aModiferStateList count:(int)anEventCount
{
  UInt16 keyCode;
  UInt32 modifierStateIndex = 0;
//  static UInt32 allModifierStates[] = {0, shiftKey, optionKey, controlKey, optionKey | shiftKey};
// for our purposes, we don't need to generate control characters
  static UInt32 allModifierStates[] = {0, shiftKey, optionKey, optionKey | shiftKey};
  
  for (keyCode = 0; keyCode < 128; keyCode++)
   {
    for (modifierStateIndex = 0; modifierStateIndex < sizeof(allModifierStates)/sizeof(UInt32); modifierStateIndex++)
     {
      UInt32 newDeadKeyState = aDeadKeyState;
      
#define MAX_UNICHARS 20
      UniChar uniChars[MAX_UNICHARS];
      int len;

      len = TkMacOSXKeycodeToUnicode(uniChars, MAX_UNICHARS, kEventRawKeyDown, keyCode, allModifierStates[modifierStateIndex], &newDeadKeyState);
      
//      NSLog(@"%@ kc = %d, mod = %x, dead in = %d dead out = %d, len = %d, uni = %C (%x)", [@"-----" substringToIndex:anEventCount], keyCode, allModifierStates[modifierStateIndex], aDeadKeyState, newDeadKeyState, len, len > 0 ? uniChars[0] : 32, uniChars[0]);
      
      if (len <= 1)
       {
        aKeyCodeList[anEventCount] = keyCode;
        aModiferStateList[anEventCount] = allModifierStates[modifierStateIndex];
        
        // TODO: the event count < 1 is a hack to stop it wandering down opt-x-opt-y-opt-z alleys
        // whats the RIGHT way to do it???
        if (newDeadKeyState != 0 && newDeadKeyState != aDeadKeyState && anEventCount < 1)
         {
          [self fillWithDeadKeyState:newDeadKeyState keyCodes:aKeyCodeList modifierStates:aModiferStateList count:anEventCount + 1];
         }
        else
         {
          NSString *unicharString = [NSString stringWithCharacters:uniChars length:1];
          if ([[self unicharLookup] objectForKey:unicharString] == nil)
           {
            [[self unicharLookup] setObject:[KeyboardEvent keyboardEventWithUnicharString:unicharString keyCodes:aKeyCodeList modifierStates:aModiferStateList count:anEventCount + 1] forKey:unicharString];
           }
         }
       }
     }
   }
}



- (NSMutableDictionary *)unicharLookup
{
  return _unicharLookup;
}

- (void)setUnicharLookup:(NSMutableDictionary *)newUnicharLookup
{
  if (_unicharLookup != newUnicharLookup)
   {
    [_unicharLookup release];
    _unicharLookup = [newUnicharLookup retain];
   }
}

- (KeyboardLayoutRef)currentKeyboardLayoutRef
{
  return _currentKeyboardLayoutRef;
}

- (void)setCurrentKeyboardLayoutRef:(KeyboardLayoutRef)newKeyboardLayoutRef
{
  _currentKeyboardLayoutRef = newKeyboardLayoutRef;
}

- (void)dealloc
{
  [_unicharLookup release];
  [super dealloc];
}

@end


