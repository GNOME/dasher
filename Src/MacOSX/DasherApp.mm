//
//  DasherApp.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import "DasherApp.h"
#import "DasherUtil.h"
#import "PreferencesController.h"
#import "DasherController.h"
#import "DasherEdit.h"

/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 18 April 2003
 */

#import <Cocoa/Cocoa.h>


@implementation DasherApp

- (id)init
{
  if (self = [super init])
    {
    [PreferencesController preferencesController];
    [[DasherController dasherController] makeKeyAndOrderFront:self];
    [DasherEdit dasherEdit];
    }

  return self;
}

- (IBAction)importTrainingText:(id)sender
{
  NSOpenPanel *op = [NSOpenPanel openPanel];

  int returnCode = [op runModalForDirectory:nil file:nil types:nil];

  if (returnCode == NSOKButton)
    {
    importTrainingFile([op filename]);
    }
}

- (IBAction)showPreferences:(id)sender
{
  [[PreferencesController preferencesController] makeKeyAndOrderFront:sender];
}

- (IBAction)showDasherPanel:(id)sender
{
  [[DasherController dasherController] makeKeyAndOrderFront:sender];
}


@end
