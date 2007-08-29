//
//  Chatter.m
//  Dasher
//
//  Created by dougie on 11/10/2004.
//  Copyright 2004 __MyCompanyName__. All rights reserved.
//

#import "Chatter.h"

static Chatter *sharedInstance = nil;

@implementation Chatter

+ (id)sharedInstance
{
    if (sharedInstance == nil)
      {
      sharedInstance = [[self alloc] init];
      }
  
  return sharedInstance;
}

- (id)init
{
  if (self = [super init])
    {
    [self setSpeechTrigger:[[NSCharacterSet letterCharacterSet] invertedSet]];
    [self setBufferedText:[NSMutableString stringWithCapacity:30]];
    [self setSpeechLag:2];
    [self setSpeechSynthesizer:[[[NSSpeechSynthesizer alloc] initWithVoice:[NSSpeechSynthesizer defaultVoice]] autorelease]];
    }
  
  return self;
}

- (void)clearBuffer
{
  [[self bufferedText] setString:@""];
}

- (void)addToBufferedText:(NSString *)aString
{
  [[self bufferedText] appendString:aString];
  [self speakIfPossible];
}

- (void)speakString:(NSString *)aString
{
  [[self speechSynthesizer] startSpeakingString:aString];
}

/* 
speech lag is in number of characters, which isn't very good.  It should be in seconds, and that would mean setting up a timer, or it could work off the main timer?  or should it be both seconds and characters whichever comes first?
*/
- (int)speechLag
{
  return _speechLag;
}

- (void)setSpeechLag:(int)newSpeechLag
{
  _speechLag = newSpeechLag;
}

- (void)speakIfPossible
{
  // don't want to tread all over the currently speaking word.  We'll come back on the next char and pick it up then.
  if ([[self speechSynthesizer] isSpeaking])
    {
    return;
    }

  NSRange triggerRange = [[self bufferedText] rangeOfCharacterFromSet:[self speechTrigger]];
  if (triggerRange.location == NSNotFound || triggerRange.location + [self speechLag] > [[self bufferedText] length])
    {
    return;
    }
  
  [self speakString:[[self bufferedText] substringToIndex:triggerRange.location]];
  [[self bufferedText] setString:[[self bufferedText] substringFromIndex:triggerRange.location + triggerRange.length]];
}

- (void)removeFromBufferedText:(NSString *)aString
{
  int wholeLen = [[self bufferedText] length];
  int delLen = [aString length];
  
  NSRange delRange = NSMakeRange(wholeLen - delLen, delLen);
  
  if (delLen > wholeLen)
    {
    delRange.location = 0;
    delRange.length = wholeLen;
    }
  
  [[self bufferedText] deleteCharactersInRange:delRange];
}


- (NSMutableString *)bufferedText {
  return [[_bufferedText retain] autorelease];
}

- (void)setBufferedText:(NSMutableString *)newBufferedText {
  if (_bufferedText != newBufferedText) {
    [_bufferedText release];
    _bufferedText = [newBufferedText retain];
  }
}

- (NSCharacterSet *)speechTrigger {
  return [[_speechTrigger retain] autorelease];
}

- (void)setSpeechTrigger:(NSCharacterSet *)newSpeechTrigger {
  if (_speechTrigger != newSpeechTrigger) {
    [_speechTrigger release];
    _speechTrigger = [newSpeechTrigger retain];
  }
}

- (NSSpeechSynthesizer *)speechSynthesizer {
  return [[_speechSynthesizer retain] autorelease];
}

- (void)setSpeechSynthesizer:(NSSpeechSynthesizer *)newSpeechSynthesizer {
  if (_speechSynthesizer != newSpeechSynthesizer) {
    [_speechSynthesizer release];
    _speechSynthesizer = [newSpeechSynthesizer retain];
  }
}


- (void)dealloc
{
  [_speechTrigger release];
  [_bufferedText release];
  [_speechSynthesizer release];
  
  [super dealloc];
}
@end
