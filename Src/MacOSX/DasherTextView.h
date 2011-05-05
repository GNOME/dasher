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

@interface DasherTextView : NSTextView<DasherEdit> {
  IBOutlet DasherApp *dasherApp;
  BOOL suppressCursorEvents;
}

@end
