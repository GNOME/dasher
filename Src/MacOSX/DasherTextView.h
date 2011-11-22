//
//  DasherTextView.h
//  Dasher
//
//  Created by Alan Lawrence on 19/05/2011.
//  Copyright 2011 Cavendish Laboratory. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DasherApp.h"
#import "DasherEdit.h"

///Implements DasherEdit protocol by making necessary changes to itself
/// (as it _is_ the textbox). However NOTE, bit of a hack, it checks for
/// game mode and disables most functions while game mode is on, as the
/// COSXGameModule will be mutating the text storage instead.
#ifdef MAC_OS_X_VERSION_10_6
@interface DasherTextView : NSTextView<DasherEdit,NSTextViewDelegate> {
#else
@interface DasherTextView : NSTextView<DasherEdit> {
#endif  
  IBOutlet DasherApp *dasherApp;
  BOOL suppressCursorEvents;
}

@end
