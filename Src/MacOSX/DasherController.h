//
//  DasherController.h
//  Dasher
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class DasherView;

@interface DasherController : NSObject {
  IBOutlet NSPanel *panelUI;
  IBOutlet DasherView *dasherViewUI;
}

+ (id)dasherController;

- (void)makeKeyAndOrderFront:(id)sender;
- (void)setPanelAlphaValue:(float)anAlphaValue;

@end
