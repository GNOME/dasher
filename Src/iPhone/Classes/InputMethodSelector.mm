//
//  InputMethodSelector.mm
//  Dasher
//
//  Created by Alan Lawrence on 20/04/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "InputMethodSelector.h"
#import "DasherUtil.h"
#import "Parameters.h"
#import "Common.h"
#import "DasherAppDelegate.h"
#import "CalibrationController.h"
#import "ParametersController.h"

typedef struct __FILTER_DESC__ {
	NSString *title;
	NSString *subTitle;
	const char *deviceName;
	const char *filterName;
} SFilterDesc;

SFilterDesc asTouchFilters[] = {
	{@"Hybrid Mode", @"Tap or Hold", TOUCH_INPUT, "Stylus Control"},
  {@"Boxes", @"Tap box to select", TOUCH_INPUT, "Direct Mode"},
};

SFilterDesc asDynamicFilters[] = {
  {@"Scanning", @"Tap screen when box highlighted", TOUCH_INPUT, "Menu Mode"},
  {@"One Button Mode", @"Tap screen when cursor near", TOUCH_INPUT, "Static One Button Mode"},
	{@"Dynamic 1B Mode", @"Tap anywhere - twice", TOUCH_INPUT, "Two-push Dynamic Mode (New One Button)"},
  {@"Dynamic 2B Mode", @"Tap Top or Bottom", TOUCH_INPUT, "Two Button Dynamic Mode"},
};

SFilterDesc asTiltFilters[] = {
	{@"Full 2D",@"hold-to-go", TILT_INPUT, "Hold-down filter"},
	{@"Single-axis",@"with slowdown & tap-to-start", TILT_INPUT, ONE_D_FILTER},
};

SFilterDesc asMixedFilters[] = {
	{@"(X,Y)", @"by (touch,tilt)", MIXED_INPUT, "Hold-down filter"},
	{@"1D-mode", @"tilt for direction, X-touch for speed", MIXED_INPUT, POLAR_FILTER},
	{@"(Y,X)", @"by (touch,tilt)", REVERSE_MIX, "Stylus Control"},
};

typedef struct __SECTION_DESC__ {
	NSString *displayName;
	SFilterDesc *filters;
	int numFilters;
} SSectionDesc;
	
SSectionDesc allMeths[] = {
{@"Touch Control", asTouchFilters, sizeof(asTouchFilters) / sizeof(asTouchFilters[0])},
{@"Button Modes", asDynamicFilters, sizeof(asDynamicFilters) / sizeof(asDynamicFilters[0])},
{@"Tilt Control", asTiltFilters, sizeof(asTiltFilters) / sizeof(asTiltFilters[0])},
{@"Combined Touch & Tilt", asMixedFilters, sizeof(asMixedFilters) / sizeof(asMixedFilters[0])},
};

int numSections = sizeof(allMeths) / sizeof(allMeths[0]);

@interface InputMethodSelector ()
@property (retain) NSIndexPath *selectedPath;
- (void)doSelect;
@end

@implementation InputMethodSelector

@synthesize selectedPath;

- (id)init {
  if (self = [super initWithStyle:UITableViewStyleGrouped]) {
		self.tabBarItem.title = @"Control";
		self.tabBarItem.image = [UIImage imageNamed:@"pen.png"];
  }
  return self;
}

/*
- (void)viewDidLoad {
    [super viewDidLoad];

    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}
*/

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
	[super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
	[super viewDidDisappear:animated];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return numSections + 1; //add 1 for calibrate button
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  return (section==numSections) ? 0 : allMeths[section].numFilters;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	DasherAppDelegate *app = [DasherAppDelegate theApp];
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
	UILabel *subText;
	if (cell)
		subText = [cell viewWithTag:1];
	else {
        cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
		subText = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
		subText.tag = 1;
		[cell addSubview:subText];
		//cell.autoresizesSubviews = YES; //ineffective even if done
		[subText setAdjustsFontSizeToFitWidth:YES];
    }
	
	// Set up the cell...
	SFilterDesc *filter = &allMeths[ [indexPath section] ].filters[ [indexPath row] ];
	if (filter->deviceName == app.dasherInterface->GetStringParameter(SP_INPUT_DEVICE)
		&& filter->filterName == app.dasherInterface->GetStringParameter(SP_INPUT_FILTER)
    && (!selectedPath || [indexPath compare:selectedPath])) {
      self.selectedPath = indexPath;
      [self performSelectorOnMainThread:@selector(doSelect) withObject:nil waitUntilDone:NO];
	}
	
  UIButton *btn = [UIButton buttonWithType:UIButtonTypeDetailDisclosure];
  cell.accessoryView = btn;
  btn.tag = (int)filter;
  [btn addTarget:self action:@selector(settings:) forControlEvents:UIControlEventTouchUpInside];
  
	cell.text = filter->title;
	CGSize titleSize = [filter->title sizeWithFont:cell.font];
	subText.frame = CGRectMake(titleSize.width + 30.0, 5.0, 245.0 - titleSize.width, 34.0);

	subText.text = filter->subTitle;

    return cell;
}

- (void)moduleSettingsDone {
  [self.navigationController popViewControllerAnimated:YES];
  [self doSelect];
}

- (void)doSelect {
  [self.tableView selectRowAtIndexPath:selectedPath animated:NO scrollPosition:UITableViewScrollPositionMiddle];
}

- (void)settings:(id)button {
  UIButton *btn = (UIButton *)button;
  SFilterDesc *desc = (SFilterDesc *)btn.tag;
  CDasherModule *mod = [DasherAppDelegate theApp].dasherInterface->GetModuleByName(desc->filterName);
  SModuleSettings *settings=NULL; int count=0;
  if (mod->GetSettings(&settings, &count)) {
    ParametersController *params = [[[ParametersController alloc] initWithTitle:NSStringFromStdString(desc->filterName) Settings:settings Count:count] autorelease];
    [params setTarget:self Selector:@selector(moduleSettingsDone)];
    [self.navigationController pushViewController:params animated:YES];
  }
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
  if (section < numSections) return nil; //no special view => default, using title method (below)
  if (!calibButton) {
    calibButton = [[UIView alloc] initWithFrame:CGRectZero];
    calibButton.backgroundColor = [UIColor clearColor];
    calibButton.opaque = NO;
    UIButton *btn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [btn setTitle:@"Calibrate Tilting..." forState:UIControlStateNormal];
    btn.font = [UIFont boldSystemFontOfSize:18.0];
    btn.frame = CGRectMake(9.0,2.0,302.0,[self tableView:tableView heightForHeaderInSection:section]-4.0);
    [btn addTarget:self action:@selector(calibrate) forControlEvents:UIControlEventTouchUpInside];
    [calibButton addSubview:btn];
  }
  return calibButton;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    return allMeths[section].displayName;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger) section {
	return 40.0f;
}

- (void)calibrate {
  CalibrationController *calCon = [[[CalibrationController alloc] init] autorelease];
  [self.navigationController pushViewController:calCon animated:YES];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	//[tableView deselectRowAtIndexPath:indexPath animated:NO];
	self.selectedPath = indexPath;
	DasherAppDelegate *app = [DasherAppDelegate theApp];
	SFilterDesc *filter = &allMeths[ [indexPath section] ].filters[ [indexPath row] ];
	app.dasherInterface->SetStringParameter(SP_INPUT_DEVICE, filter->deviceName);
	app.dasherInterface->SetStringParameter(SP_INPUT_FILTER, filter->filterName);
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/


/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


- (void)dealloc {
    [super dealloc];
}


@end

