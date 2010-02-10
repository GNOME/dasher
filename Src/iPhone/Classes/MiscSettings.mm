//
//  MiscSettings.m
//  Dasher
//
//  Created by Alan Lawrence on 20/07/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "MiscSettings.h"
#import "DasherAppDelegate.h"
#import "CDasherInterfaceBridge.h"
#import "Parameters.h"

static SModuleSettings _settings[] = { //note iStep and string description are ignored
  {LP_NODE_BUDGET, T_LONG, 400, 10000, 1, 0, ""}, //hopefully appropriate for an iPhone 3GS?
  {LP_MARGIN_WIDTH, T_LONG, 100, 900, 1, 0, ""},
  {BP_AUTO_SPEEDCONTROL, T_BOOL, -1, -1, -1, -1, ""},
  {BP_NONLINEAR_Y, T_BOOL, -1, -1, -1, -1, ""},
  {BP_DOUBLE_X, T_BOOL, -1, -1, -1, -1, ""},
};
static int _count = sizeof(_settings) / sizeof(_settings[0]);

@implementation MiscSettings

- (id)init {  
	if (self = [super initWithTitle:@"Misc" Settings:_settings Count:_count]) {
		self.tabBarItem.image = [UIImage imageNamed:@"misc.png"];
	}
	return self;
}

@end
