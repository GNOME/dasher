//
//  DasherController.mm
//  Dasher
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import "DasherController.h"
#import "DasherView.h"
#import "PreferencesController.h"

static DasherController *dasherController = nil;

@implementation DasherController

+ (id)dasherController
{
  if (dasherController == nil)
    {
    dasherController = [[self alloc] init];  // retain to use as singleton
    [NSBundle loadNibNamed:@"DasherPanel" owner:self];
    }

  return dasherController;
}

- (void)awakeFromNib
{
  [self setPanelAlphaValue:[[NSUserDefaults standardUserDefaults] floatForKey:DASHER_PANEL_OPACITY]];
}

- (void)makeKeyAndOrderFront:(id)sender
{
  if (panelUI == nil)
    {
    [NSBundle loadNibNamed:@"DasherPanel" owner:self];
    }

  [panelUI makeKeyAndOrderFront:self];
}

- (void)setPanelAlphaValue:(float)anAlphaValue
{
//  [panelUI setOpaque:NO];
  [panelUI setAlphaValue:anAlphaValue];
}

@end
