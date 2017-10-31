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
#import "IPhoneFilters.h"

typedef struct __FILTER_DESC__ {
	NSString *title;
	NSString *subTitle;
	const char *deviceName;
	const char *filterName;
  ///Null-terminated list of NSUserDefaults keys for extra non-Core settings
  NSString **iPhoneOpts;
} SFilterDesc;

NSString *touchSettings[] = {TOUCH_USE_TILT_X, DOUBLE_TOUCH_X, NULL};
NSString *tiltSettings[] = {HOLD_TO_GO, TILT_USE_TOUCH_X, DOUBLE_TOUCH_X, TILT_1D, @"CircleStart", NULL};

NSString *calibBtn=@"Calibrate...";//pointer equality used to mark cell for special-casing in cellForRowAtIndexPath: below

SFilterDesc asNormalFilters[] = {
	{@"Touch Control", @"Tap or Hold", TOUCH_INPUT, TOUCH_FILTER, touchSettings},
  {@"Multitouch",@"Two fingers, further apart = slower", TWO_FINGER_INPUT, TWO_FINGER_FILTER, NULL},
  {@"Tilt Control", calibBtn, TILT_INPUT, TILT_FILTER, tiltSettings},
};

SFilterDesc asBoxFilters[] = {
  {@"Direct Mode", @"Tap box to select", TOUCH_INPUT, "Direct Mode", NULL},
  {@"Scanning", @"Tap screen when box highlighted", TOUCH_INPUT, "Menu Mode", NULL},
};

SFilterDesc asDynamicFilters[] = {
	{@"Dynamic 1B Mode", @"Tap anywhere - twice", TOUCH_INPUT, "Two-push Dynamic Mode (New One Button)", NULL},
  {@"Dynamic 2B Mode", @"Tap Top or Bottom", TOUCH_INPUT, "Two Button Dynamic Mode", NULL},
};

typedef struct __SECTION_DESC__ {
	NSString *displayName;
	SFilterDesc *filters;
	int numFilters;
} SSectionDesc;
	
SSectionDesc allMeths[] = {
{@"Normal Steering", asNormalFilters, sizeof(asNormalFilters) / sizeof(asNormalFilters[0])},
{@"Box Modes", asBoxFilters, sizeof(asBoxFilters) / sizeof(asBoxFilters[0])},
{@"Dynamic Modes", asDynamicFilters, sizeof(asDynamicFilters) / sizeof(asDynamicFilters[0])},
};

int numSections = sizeof(allMeths) / sizeof(allMeths[0]);

@interface ExtraParametersController : ParametersController {
  SModuleSettings *m_pSettings2;
  int m_iCount2;
  NSString **iPhonePrefKeys;
}
-(id)initForFilter:(SFilterDesc *)filtr;
@end


@interface InputMethodSelector ()
-(UITableViewCellAccessoryType)accessoryTypeForFilter:(SFilterDesc *)filter;
@end

@implementation InputMethodSelector

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
  return numSections;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  return allMeths[section].numFilters;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {

  SFilterDesc *filter = &allMeths[ [indexPath section] ].filters[ [indexPath row] ];
  UITableViewCell *cell;
  
  if (filter->subTitle==calibBtn) {
    static NSString *CalibCellId = @"CalibCell";
    cell = [tableView dequeueReusableCellWithIdentifier:CalibCellId];
    if (!cell) {
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:CalibCellId] autorelease];
      
      UIButton *btn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
      [btn setTitle:calibBtn forState:UIControlStateNormal];
      CGSize textSize = [calibBtn sizeWithFont:btn.titleLabel.font];
      btn.frame = CGRectMake(140.0,9.0,textSize.width+10,textSize.height+6);
      [btn addTarget:self action:@selector(calibrate) forControlEvents:UIControlEventTouchUpInside];
      
      [cell.contentView addSubview:btn];
    }
  } else {
    static NSString *CellId = @"Cell";
    
    cell = [tableView dequeueReusableCellWithIdentifier:CellId];
    if (!cell)
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellId] autorelease];
    cell.detailTextLabel.text = filter->subTitle;
  }
	
	// Set up the cell...
  cell.textLabel.text = filter->title;
	CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
  if (filter->deviceName == intf->GetStringParameter(SP_INPUT_DEVICE)
      && filter->filterName == intf->GetStringParameter(SP_INPUT_FILTER)) {
    //filter is currently selected...
    DASHER_ASSERT(selectedPath==nil || [selectedPath isEqual:indexPath]);
    selectedPath = indexPath;
    cell.accessoryType = UITableViewCellAccessoryCheckmark;
  } else {
    cell.accessoryType = [self accessoryTypeForFilter:filter];
  }
	
  return cell;
}

-(UITableViewCellAccessoryType)accessoryTypeForFilter:(SFilterDesc *)filter {
  SModuleSettings *sets; int count;
  Dasher::CDasherInterfaceBase *intf = [DasherAppDelegate theApp].dasherInterface;
  if (intf->GetModuleSettings(filter->filterName, &sets, &count))
    if (count>0) return UITableViewCellAccessoryDisclosureIndicator;
  if (intf->GetModuleSettings(filter->deviceName, &sets, &count))
    if (count>0) return UITableViewCellAccessoryDisclosureIndicator;
  if (filter->iPhoneOpts && *(filter->iPhoneOpts))
    return UITableViewCellAccessoryDisclosureIndicator;
  return UITableViewCellAccessoryNone;
}

- (void)moduleSettingsDone {
  [self.navigationController popViewControllerAnimated:YES];
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
	[tableView deselectRowAtIndexPath:indexPath animated:NO];
  //self->selectedPath is the PREVIOUSLY-SELECTED filter. Take away the checkmark...
  [tableView cellForRowAtIndexPath:selectedPath].accessoryType = [self accessoryTypeForFilter: &allMeths[[selectedPath section]].filters[[selectedPath row]]];
  //now give the NEWLY-SELECTED filter a checkmark
  [tableView cellForRowAtIndexPath:indexPath].accessoryType = UITableViewCellAccessoryCheckmark;

  //and record it as selected...
	self->selectedPath = indexPath;
	CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
	SFilterDesc *filter = &allMeths[ [indexPath section] ].filters[ [indexPath row] ];
	intf->SetStringParameter(SP_INPUT_DEVICE, filter->deviceName);
	intf->SetStringParameter(SP_INPUT_FILTER, filter->filterName);
  
  ParametersController *params = [[[ExtraParametersController alloc] initForFilter:filter] autorelease];
  [params setTarget:self Selector:@selector(moduleSettingsDone)];
  [self.navigationController pushViewController:params animated:YES];
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


@implementation ExtraParametersController

-(id)initForFilter:(SFilterDesc *)filter {
  Dasher::CDasherInterfaceBase *intf=[DasherAppDelegate theApp].dasherInterface;
  SModuleSettings *settings; int count;
  if (!intf->GetModuleSettings(filter->filterName, &settings, &count)) {
    settings=NULL; count=0;
  }
  if (self = [super initWithTitle:NSStringFromStdString(filter->filterName) Settings:settings Count:count]) {
    if (!intf->GetModuleSettings(filter->deviceName, &m_pSettings2, &m_iCount2)) {
      m_pSettings2=NULL; m_iCount2=0;
    }
    iPhonePrefKeys = filter->iPhoneOpts;
  }
  return self;
}

-(int)layoutOptionsOn:(UIView *)view startingAtY:(int)y {
  if (m_iCount)
    y=[super layoutOptionsOn:view startingAtY:y];
  else if (m_iCount2==0 && !iPhonePrefKeys)
    return [self makeNoSettingsLabelOnView:view atY:y];
  y=[self layoutModuleSettings:m_pSettings2 count:m_iCount2 onView:view startingAtY:y];
  //finally, iPhone-specific keys...
  NSUserDefaults *ud=[NSUserDefaults standardUserDefaults];
  if (iPhonePrefKeys) {
    for (NSString **key=iPhonePrefKeys; *key; key++) {
      UISwitch *sw=[self makeSwitch:*key onView:view atY:&y];
      sw.tag=(NSInteger)*key;
      sw.on = [ud boolForKey:*key];
      [sw addTarget:self action:@selector(boolUserDefChanged:) forControlEvents:UIControlEventValueChanged];
    }
  }
  return y;
}

-(void)boolUserDefChanged:(id)uiswitch {
  UISwitch *sw=(UISwitch *)uiswitch;
  NSString *key = (NSString *)sw.tag;
  [[NSUserDefaults standardUserDefaults] setBool:sw.on forKey:key];
}
@end

