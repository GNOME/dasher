//
//  DasherEdit.mm
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherEdit.h"
#import "PreferencesController.h"
#import "UnicharGenerator.h"
#import "Chatter.h"

#import <Carbon/Carbon.h>

@implementation DasherEdit


- init
{
  if (self = [super init]) 
    {
      allTextEntered = [[NSMutableString alloc] initWithCapacity:1024];
    }
  
  return self;
}

- (void)sendString:(NSString *)aString toTargetApp:(AXUIElementRef)aTargetApp {
  if (aTargetApp != NULL) {
    [[UnicharGenerator sharedInstance] postKeyboardEventsToUIElementRef:aTargetApp unicharString:aString];
  }
}

- (void)outputCallback:(NSString *)aString targetApp:(AXUIElementRef)aTargetApp
{
  dasherIsModifyingText = YES;
  [self sendString:aString toTargetApp:aTargetApp];
  dasherIsModifyingText = NO;
  [allTextEntered appendString:aString];
  [[Chatter sharedInstance] addToBufferedText:aString];
}

- (void)deleteCallback:(NSString *)s targetApp:(AXUIElementRef)aTargetApp
{
  // just send the app a number of backspace characters equal to [s length]
  int len = [s length];
  int i;
  
  dasherIsModifyingText = YES;
  for (i = 0; i < len; i++)
    {
    [self sendString:@"\b" toTargetApp:aTargetApp];
    }
  dasherIsModifyingText = NO;
  [allTextEntered deleteCharactersInRange:NSMakeRange([allTextEntered length]-len, len)];
  [[Chatter sharedInstance] removeFromBufferedText:s];
}

-(NSString *)textAtOffset:(int)iOffset Length:(int)iLength {
  return [allTextEntered substringWithRange:NSMakeRange(iOffset,iLength)];
}


@end
