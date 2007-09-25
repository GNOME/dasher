//
//  PreferencesController.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "COSXSettingsStore.h"

#define ALTER_FONT_DASHER 1
#define ALTER_FONT_EDIT 2

@class DasherApp;

@interface PreferencesController : NSObject
{
  DasherApp *dasherApp;
  
  IBOutlet NSPanel *panel;
  int fontToAlter;
  
}

+ (id)preferencesController;
- (id)init;
- (void)makeKeyAndOrderFront:(id)sender;
//- (void)displayFontPanel;
//- (IBAction)changeFont:(id)sender;
//- (void)changeDasherFont:(id)sender;


@end

