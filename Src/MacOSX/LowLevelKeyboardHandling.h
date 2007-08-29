/*
 *  LowLevelKeyboardHandling.h
 *  TestAcc
 *
 * See LowLevelKeyboardHandling.c for origin and copyright information.
 *
 */


#import <Carbon/Carbon.h>

bool keyboardMappingHasChanged(void);

int
TkMacOSXKeycodeToUnicode(
                         UniChar * uniChars, int maxChars,
                         EventKind eKind,
                         UInt32 keycode, UInt32 modifiers,
                         UInt32 * deadKeyStatePtr);


