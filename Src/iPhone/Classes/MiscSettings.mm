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
  {LP_DASHER_FONTSIZE, T_LONG, 1, 3, 1, 1, ""},
  {LP_SHAPE_TYPE, T_LONG, 0, 5, 1, -1, ""},
  {LP_OUTLINE_WIDTH, T_LONG, -5, 5, 1, -1, ""},
  {BP_AUTO_SPEEDCONTROL, T_BOOL, -1, -1, -1, -1, ""},
  {LP_NONLINEAR_X, T_LONG, 0, 10, 1, -1, ""},
  {BP_DOUBLE_X, T_BOOL, -1, -1, -1, -1, ""},
};

static int _count = sizeof(_settings) / sizeof(_settings[0]);

static SModuleSettings _controlSettings[] = {
  {BP_CONTROL_MODE, T_BOOL, -1, -1, -1, -1, ""},
  {BP_CONTROL_MODE_HAS_COPY, T_BOOL, -1, -1, -1, -1, ""},
  {BP_CONTROL_MODE_HAS_SPEECH, T_BOOL, -1, -1, -1, -1, ""},
  {BP_CONTROL_MODE_HAS_HALT, T_BOOL, -1, -1, -1, -1, ""},
  {BP_CONTROL_MODE_HAS_EDIT, T_BOOL, -1, -1, -1, -1, ""},
  {BP_COPY_ALL_ON_STOP, T_BOOL, -1, -1, -1, -1, ""},
  {BP_SPEAK_ALL_ON_STOP, T_BOOL, -1, -1, -1, -1, ""},
};

static int _controlCount = sizeof(_controlSettings) / sizeof(_controlSettings[0]);

@implementation MiscSettings

- (id)init {  
	if (self = [super initWithTitle:@"Misc" Settings:_settings Count:_count]) {
		self.tabBarItem.image = [UIImage imageNamed:@"misc.png"];
    if (![self.view isKindOfClass:[UIScrollView class]]) {
      [super dealloc];
      return nil;
    }
    UIScrollView *view = (UIScrollView *)self.view;
    CGSize oldSize = [view contentSize];
    [view setContentSize:CGSizeMake(oldSize.width, oldSize.height + 70)];
    UIButton *btn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    btn.frame = CGRectMake(40.0, oldSize.height+20.0, oldSize.width-80.0, 30.0);
    [btn setTitle:@"Control Mode..." forState:UIControlStateNormal];
    [btn addTarget:self action:@selector(control) forControlEvents:UIControlEventTouchUpInside];
    [view addSubview:btn];
	}
	return self;
}

- (void)control {
  ParametersController *control = [[[ParametersController alloc] initWithTitle:@"Control Mode" Settings:_controlSettings Count:_controlCount] autorelease];
  [control setTarget:self Selector:@selector(controlDone)];
  [self.navigationController pushViewController:control animated:YES];
}

-(void)controlDone {
  [self.navigationController popViewControllerAnimated:YES];
}

@end
