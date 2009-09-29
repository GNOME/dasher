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

//private methods...
@interface MiscSettings ()
- (void)budgetSlid:(id)sender;
- (void)speedSlid:(id)sender;
- (void)marginSlid:(id)sender;
@end

@implementation MiscSettings

- (id)init {
	if (self = [super init]) {
		self.title = @"Misc";
		self.tabBarItem.image = [UIImage imageNamed:@"misc.png"];
	}
	return self;
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
	CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
	UIView *view = [[[UIView alloc] init] autorelease];
	view.backgroundColor = [UIColor whiteColor];
	
	budgetLbl = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 30.0, 300.0, 20.0)] autorelease];
	UISlider *budget = [[[UISlider alloc] initWithFrame:CGRectMake(10.0, 50.0, 300.0, 20.0)] autorelease];
	budget.minimumValue = 100; budget.maximumValue = 4000; budget.value = intf->GetLongParameter(LP_NODE_BUDGET);
	[budget addTarget:self action:@selector(budgetSlid:) forControlEvents:UIControlEventValueChanged];
	[self budgetSlid:budget];
	[view addSubview:budgetLbl]; [view addSubview:budget];
	
	speedLbl = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 100.0, 300.0, 20.0)] autorelease];
	UISlider *speed = [[[UISlider alloc] initWithFrame:CGRectMake(10.0, 120.0, 300.0, 20.0)] autorelease];
	speed.minimumValue = 0.1; speed.maximumValue = 12.0; speed.value = intf->GetLongParameter(LP_MAX_BITRATE)/100.0;
	[speed addTarget:self action:@selector(speedSlid:) forControlEvents:UIControlEventValueChanged];
	[self speedSlid:speed];
	[view addSubview:speedLbl]; [view addSubview:speed];

	marginLbl = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 170.0, 300.0, 20.0)] autorelease];
  UISlider *margin = [[[UISlider alloc] initWithFrame:CGRectMake(10.0, 190.0, 300.0, 20.0)] autorelease];
  margin.minimumValue = 100.0; margin.maximumValue = 900.0; margin.value = intf->GetLongParameter(LP_MARGIN_WIDTH);
  [margin addTarget:self action:@selector(marginSlid:) forControlEvents:UIControlEventValueChanged];
  [self marginSlid:margin];
  [view addSubview:marginLbl]; [view addSubview:margin];
  
	UILabel *ascLbl = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 232.0, 190.0, 20.0)] autorelease];
	ascLbl.text = @"Auto Speed Control";
	UISwitch *asc = [[[UISwitch alloc] initWithFrame:CGRectMake(210.0, 232.0, 100.0, 20.0)] autorelease];
	[view addSubview:ascLbl]; [view addSubview:asc];
	asc.on = intf->GetBoolParameter(BP_AUTO_SPEEDCONTROL);
	[asc addTarget:self action:@selector(autoSpeedCon:) forControlEvents:UIControlEventValueChanged];
  
  UILabel *nonLinLbl = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 274.0, 190.0, 20.0)] autorelease];
  nonLinLbl.text = @"Compress top/bottom";
  UISwitch *nonLin = [[[UISwitch alloc] initWithFrame:CGRectMake(210.0, 274.0, 100.0, 20.0)] autorelease];
  [view addSubview:nonLinLbl]; [view addSubview:nonLin];
  nonLin.on = intf->GetBoolParameter(BP_NONLINEAR_Y);
  [nonLin addTarget:self action:@selector(nonLinear:) forControlEvents:UIControlEventValueChanged];

  UILabel *dblLabel = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 316.0, 190.0, 20.0)] autorelease];
  dblLabel.text = @"Touch on left half";
  UISwitch *dbl = [[[UISwitch alloc] initWithFrame:CGRectMake(210.0, 316.0, 100.0, 20.0)] autorelease];
  [view addSubview:dblLabel]; [view addSubview:dbl];
  dbl.on = intf->GetBoolParameter(BP_DOUBLE_X);
  [dbl addTarget:self action:@selector(doubleX:) forControlEvents:UIControlEventValueChanged];
  
  self.view = view;
}

- (void)autoSpeedCon:(id)sender {
  [DasherAppDelegate theApp].dasherInterface->SetBoolParameter(BP_AUTO_SPEEDCONTROL, ((UISwitch *)sender).on);
}

- (void)nonLinear:(id)sender {
  [DasherAppDelegate theApp].dasherInterface->SetBoolParameter(BP_NONLINEAR_Y, ((UISwitch *)sender).on);
}

- (void)doubleX:(id)sender {
  [DasherAppDelegate theApp].dasherInterface->SetBoolParameter(BP_DOUBLE_X, ((UISwitch *)sender).on);
}

- (void)budgetSlid:(id)sender {
	int v = ((UISlider *)sender).value;
	[DasherAppDelegate theApp].dasherInterface->SetLongParameter(LP_NODE_BUDGET, v);
	budgetLbl.text = [NSString stringWithFormat:@"Display detail: %i", v];
}

- (void)speedSlid:(id)sender {
	float v = ((UISlider *)sender).value;
	[DasherAppDelegate theApp].dasherInterface->SetLongParameter(LP_MAX_BITRATE, 100*v);
	speedLbl.text = [NSString stringWithFormat:@"Max Bitrate: %1.2f", v];
}

- (void)marginSlid:(id)sender {
  int v = ((UISlider *)sender).value;
  [DasherAppDelegate theApp].dasherInterface->SetLongParameter(LP_MARGIN_WIDTH, v);
  marginLbl.text = [NSString stringWithFormat:@"Margin Width: %i", v];
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
