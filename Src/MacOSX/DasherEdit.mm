//
//  DasherEdit.mm
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherEdit.h"
#import "UnicharGenerator.h"
#import "../Common/Common.h"
#import <Carbon/Carbon.h>
#import "AlphInfo.h"
#import "DasherUtil.h"
#import "DasherInterfaceBase.h"

using namespace Dasher;

@interface DirectEdit ()
- (void)sendString:(NSString *)aString;
@end

@implementation DirectEdit


-(id)initWithIntf:(CDasherInterfaceBase *)_pIntf AppWatcher:(AppWatcher *)_appWatcher {
  if (self = [super init]) {
    allTextEntered = [[NSMutableString alloc] initWithCapacity:1024];
    pIntf = _pIntf;
    appWatcher = _appWatcher;
  }
  return self;
}

- (void)sendString:(NSString *)aString {
    if (int pid = [appWatcher targetAppPid]) {
      [[UnicharGenerator sharedInstance] postKeyboardEventsToPID: pid unicharString: aString];
    }
}

- (void)outputCallback:(NSString *)aString {
  [self sendString:aString];
  [allTextEntered appendString:aString];
}

- (void)deleteCallback:(NSString *)s {
  // just send the app a number of backspace characters equal to [s length]
  int len = [s length];
  
  for (int i = 0; i < len; i++) {
    [self sendString:@"\b"];
  }
  [allTextEntered deleteCharactersInRange:NSMakeRange([allTextEntered length]-len, len)];
}

-(NSString *)textAtOffset:(unsigned int)iOffset Length:(unsigned int)iLength {
  //This does not hold if using control mode since it gets the offsets wrong...
  //DASHER_ASSERT(iOffset+iLength <= [allTextEntered length]);
  //Instead we shall handle all the out-of-bounds cases ourselves
  // (substringWithRange is not at all forgiving!)
  if (iOffset >= [allTextEntered length]) return @"";
  if (iOffset+iLength > [allTextEntered length])
    iLength = [allTextEntered length]-iOffset;
  return [allTextEntered substringWithRange:NSMakeRange(iOffset,iLength)];
}

-(NSString *)allContext {
  return allTextEntered;
}

-(void)clearContext {
  [allTextEntered setString:@""];
}

-(unsigned int)currentCursorPos {
  return [allTextEntered length];
}

-(unsigned int)ctrlMove:(CControlManager::EditDistance)dist forwards:(BOOL)bForwards {
#ifdef DEBUG
std::cout << "Call to edit move, doing nothing" << std::endl;
#endif
  return [allTextEntered length];
}

-(unsigned int)ctrlDelete:(CControlManager::EditDistance)dist forwards:(BOOL)bForwards {
  if (!bForwards) {
    const unsigned int iCurrentOffset([allTextEntered length]);
    unsigned int numChars;
    if (dist==CControlManager::EDIT_CHAR)
      numChars=1;
    else if (dist==CControlManager::EDIT_FILE)
      numChars = iCurrentOffset;
    else {
      const CAlphInfo *pAlph(pIntf->GetActiveAlphabet());
      const string &target(pAlph->GetText(dist==CControlManager::EDIT_WORD ? pAlph->GetSpaceSymbol() : pAlph->GetParagraphSymbol()));
      NSRange range = [allTextEntered rangeOfString:NSStringFromStdString(target) options:NSBackwardsSearch];
      numChars = (range.length==0) ? iCurrentOffset : iCurrentOffset-range.location; //0=> not found, so go to beginning
    }
    for (unsigned int i=0; i<numChars; i++)
      [self sendString:@"\b"];
  }
  return [allTextEntered length];
}

@end
