//
//  Chatter.h
//  Dasher
//
//  Created by dougie on 11/10/2004.
//  Copyright 2004 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Chatter : NSObject 
{
  NSMutableString *_bufferedText;
  NSCharacterSet *_speechTrigger;
  NSSpeechSynthesizer *_speechSynthesizer;
  int _speechLag;
}

- (int)speechLag;
- (void)setSpeechLag:(int)newSpeechLag;

+ (id)sharedInstance;

- (void)speakIfPossible;
- (void)speakString:(NSString *)aString;

- (void)addToBufferedText:(NSString *)aString;
- (void)removeFromBufferedText:(NSString *)aString;
- (void)clearBuffer;

- (NSMutableString *)bufferedText;
- (void)setBufferedText:(NSMutableString *)newBufferedText;
- (NSCharacterSet *)speechTrigger;
- (void)setSpeechTrigger:(NSCharacterSet *)newSpeechTrigger;
- (NSSpeechSynthesizer *)speechSynthesizer;
- (void)setSpeechSynthesizer:(NSSpeechSynthesizer *)newSpeechSynthesizer;


@end
