//
//  DasherEdit.h
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

// acts as the delegate and controller of the TextView in the dasher window

@interface DasherEdit : NSObject {
  int flushCount;
  IBOutlet NSTextView *dasherTextUI;

  BOOL dasherIsModifyingText;
}

@end
