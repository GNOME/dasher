//
//  DasherEdit.h
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <AppKit/AppKit.h>

@interface DasherEdit : NSObject 
{
  BOOL dasherIsModifyingText;
  NSMutableString *allTextEntered;
}

- init;
- (void)sendString:(NSString *)aString toTargetApp:(AXUIElementRef)aTargetApp;
- (void)outputCallback:(NSString *)aString targetApp:(AXUIElementRef)aTargetApp;
- (void)deleteCallback:(NSString *)s targetApp:(AXUIElementRef)aTargetApp;
- (NSString *)textAtOffset:(int)iOffset Length:(int)iLength;
- (NSString *)allContext;
- (void)clearContext;

@end
