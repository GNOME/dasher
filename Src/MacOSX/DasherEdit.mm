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
  
  [[Chatter sharedInstance] removeFromBufferedText:s];
}


- (NSString *)getNewContextCallback:(int)maxChars
{
#if 0
  NSString *result = nil;
  
  this needs redoing to handle the new setup of typing into other apps
  
  NSString *s = [[self currentTextUI] string];
  NSRange r = [[self currentTextUI] selectedRange];
  unsigned int location = 0;
  unsigned int length = maxChars;
  
  if ((int)r.location < maxChars) {
    location = 0;
    length = r.location;
  } else {
    location = r.location - maxChars;
    length = maxChars;
  }
  
  r = NSMakeRange(location, length);
  
  result = r.length <= 0 ? @"" : [s substringWithRange:r];
#endif
  
  [[Chatter sharedInstance] clearBuffer];
  
//  return result;
  return @"abcde";
}


@end
