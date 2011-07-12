//
//  CalibrationController.mm
//  Dasher
//
//  Created by Alan Lawrence on 26/06/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//
#import "CalibrationController.h"
#import "Common.h"
#import "Parameters.h"
#import "DasherAppDelegate.h"
#import "DasherUtil.h"
#include <sstream>

static NSString *CUSTOM_TILT_STRING = @"CustomTiltParams";
static NSString *VERTICAL_TILT_STRING = @"VerticalTiltParams";
static NSString *USE_CUSTOM_TILT = @"CustomTilt";

using namespace std;

//private method...
@interface CalibrationController ()
- (void)loadVerticalLabels;
@end

//utility functions...
template <class charT, class traits> basic_istream<charT,traits>& operator>> (basic_istream<charT,traits>& is, Vec3 &dest)
{
	char open, comma1, comma2, close;
	is >> open >> dest.x >> comma1 >> dest.y >> comma2 >> dest.z >> close;
	DASHER_ASSERT(open == '(' && comma1 == ',' && comma2 == ',' && close == ')');
	return is;
}

ostream &operator<<(ostream &os, const Vec3 &v)
{
	return os << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}

void saveVerticalState(float minY, float maxY, float minX, float maxX, BOOL invert)
{
	ostringstream os;
	if (invert) os << maxY << " - " << minY; else os << minY << " - " << maxY;
	os << " / " << minX << " - " << maxX;
  [[NSUserDefaults standardUserDefaults] setObject:NSStringFromStdString(os.str()) forKey:VERTICAL_TILT_STRING];
}

void getVerticalState(float &minY, float &maxY, float &minX, float &maxX, BOOL &invert)
{
  NSString *val = ([[NSUserDefaults standardUserDefaults] objectForKey:VERTICAL_TILT_STRING])
  ? [[NSUserDefaults standardUserDefaults] stringForKey:VERTICAL_TILT_STRING]
  : @"-0.1 - -0.9 / -0.4 - 0.4";

	istringstream is(StdStringFromNSString(val));
	string hyphen1, stroke, hyphen2;
	is >> minY >> hyphen1 >> maxY >> stroke >> minX >> hyphen2 >> maxX;
	DASHER_ASSERT(hyphen1 == "-" && stroke == "/" && hyphen2 == "-");
	if (maxY < minY)
	{
		float temp = minY;
		minY = maxY;
		maxY = temp;
		invert = YES;
	}
	else invert = NO;
}

void saveCustomState(const Vec3 &min, const Vec3 &max, const Vec3 &slow)
{
	ostringstream os;
	os << min << " - " << max << " / " << slow;
  [[NSUserDefaults standardUserDefaults] setObject:NSStringFromStdString(os.str()) forKey:CUSTOM_TILT_STRING];
}

void getCustomState(Vec3 &min, Vec3 &max, Vec3 &slow)
{
  NSString *val = ([[NSUserDefaults standardUserDefaults] objectForKey:CUSTOM_TILT_STRING] == nil)
     ? @"(0.0,0.0,0.0) - (0.0,-1.0,0.0) / (-1.0,0.0,0.0)" //default
  : [[NSUserDefaults standardUserDefaults] stringForKey:CUSTOM_TILT_STRING];
    
	istringstream is(StdStringFromNSString(val));
	string hyphen, stroke;
	is >> min >> hyphen >> max >> stroke >> slow;
	DASHER_ASSERT(hyphen == "-" && stroke == "/");
}


//setting functions...
void setCustomTiltAxes(Vec3 min, Vec3 max, Vec3 slow) {
	Vec3 dir = Vec3Sub(max,min);
	[DasherAppDelegate theApp].dasherInterface->SetTiltAxes(dir, Vec3Dot(dir, min), slow, 0.0);
}

void setVerticalTiltAxes(float minY, float maxY, float minX, float maxX, BOOL invert) {
	DASHER_ASSERT(minY < maxY);
	DASHER_ASSERT(minX < maxX);
	if (invert) {float temp = minY; minY = maxY; maxY = temp;}
	[DasherAppDelegate theApp].dasherInterface->SetTiltAxes(Vec3Make(0.0,1.0/(maxY-minY),0.0),
								 minY/(maxY-minY),
								 Vec3Make(1.0/(maxX-minX),0.0,0.0), minX/(maxX-minX));
	
}

@implementation CalibrationController
+(void)doSetup {
	if ([[NSUserDefaults standardUserDefaults] boolForKey:USE_CUSTOM_TILT])
	{
		Vec3 min, max, slow;
		getCustomState(min, max, slow);
		setCustomTiltAxes(min, max, slow);
	}
	else
	{
		float y1, y2, x1, x2; BOOL invert;
		getVerticalState(y1, y2, x1, x2, invert);
		setVerticalTiltAxes(y1, y2, x1, x2, invert);
	}
}

- (id)init {
  if (self = [super initWithStyle:UITableViewStyleGrouped]) {
    self.tabBarItem.title = @"Tilting";
    self.tabBarItem.image = [UIImage imageNamed:@"tilt.png"];
  }
  return self;
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation {
  if ([UIAccelerometer sharedAccelerometer].delegate == self)
    return toInterfaceOrientation == m_fixOrientation;
  return toInterfaceOrientation != UIInterfaceOrientationPortraitUpsideDown;
}

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/

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
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView { return 2; }

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section { return 1; }

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	DASHER_ASSERT([indexPath row] == 0);
	DASHER_ASSERT(([indexPath section] & ~1) == 0); //0 or 1
	return ([indexPath section] == 0) ? 170 : 130;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	DASHER_ASSERT([indexPath row] == 0);
	switch ([indexPath section])
	{
	case 0:
		if (!vertical)
		{
			vertical = [[UITableViewCell alloc] initWithFrame:CGRectZero];
			vertMin = [[[UILabel alloc] initWithFrame:CGRectMake(10, 10, 140, 30)] autorelease];
			[vertical.contentView addSubview:vertMin];
			vertMax = [[[UILabel alloc] initWithFrame:CGRectMake(10, 50, 140, 30)] autorelease];
			[vertical.contentView addSubview:vertMax];
			vertX = [[[UILabel alloc] initWithFrame:CGRectMake(10, 90, 290, 30)] autorelease];
			[vertical.contentView addSubview:vertX];
			setRange = [UIButton buttonWithType:UIButtonTypeRoundedRect];
			setRange.frame = CGRectMake(150, 10, 140, 80);
			[setRange setTitle:@"Set" forState:UIControlStateNormal];
			[vertical.contentView addSubview:setRange];
			[setRange addTarget:self action:@selector(set:) forControlEvents:UIControlEventTouchUpInside];
			UILabel *invertLabel = [[[UILabel alloc] initWithFrame:CGRectMake(10, 130, 100, 30)] autorelease];
			invertLabel.text = @"Invert Y:";
			[vertical.contentView addSubview:invertLabel];
			invert = [[[UISwitch alloc] initWithFrame:CGRectMake(120, 130, 190, 30)] autorelease];
			[vertical.contentView addSubview:invert];
			[invert addTarget:self action:@selector(invertChanged) forControlEvents:UIControlEventValueChanged];
			[self loadVerticalLabels];
		}
		return vertical;
	case 1:
		if (!custom)
		{
			custom = [[UITableViewCell alloc] initWithFrame:CGRectZero];
			Vec3 min, max, slow;
			getCustomState(min, max, slow);
			custMin = [UIButton buttonWithType:UIButtonTypeRoundedRect];
			custMin.frame = CGRectMake(10,10,280,30);
			[custMin setTitle:[NSString stringWithFormat:@"Min: (%1.3f,%1.3f,%1.3f)",min.x,min.y,min.z] forState:UIControlStateNormal];
			[custom.contentView addSubview:custMin];
			[custMin addTarget:self action:@selector(set:) forControlEvents:UIControlEventTouchUpInside];
			custMax = [UIButton buttonWithType:UIButtonTypeRoundedRect];
			custMax.frame = CGRectMake(10,50,280,30);
			[custMax setTitle:[NSString stringWithFormat:@"Max: (%1.3f,%1.3f,%1.3f)",max.x,max.y,max.z] forState:UIControlStateNormal];
			[custom.contentView addSubview:custMax];
			[custMax addTarget:self action:@selector(set:) forControlEvents:UIControlEventTouchUpInside];
			custX = [UIButton buttonWithType:UIButtonTypeRoundedRect];
			custX.frame = CGRectMake(10,90,280,30);
			[custX setTitle:[NSString stringWithFormat:@"Slow: (%1.3f,%1.3f,%1.3f)",slow.x,slow.y,slow.z] forState:UIControlStateNormal];
			[custom.contentView addSubview:custX];
			[custX addTarget:self action:@selector(set:) forControlEvents:UIControlEventTouchUpInside];
		}
		return custom;
	}
	NSLog(@"Unknown section %i\n",[indexPath section]);
	return nil;
}

- (void)set:(id)sender {
  NSString *title;
  oldDeleg = [UIAccelerometer sharedAccelerometer].delegate;
  if (sender == setRange) {
    minX = 1.0; minY = 1.0; maxX = -1.0; maxY = -1.0;
    settingParam = SETTING_VERT;
    title=@"Set Range";
    settingLabel = nil;
  } else {
    if (sender == custX) {
      title=@"Set Minor Axis";
      settingParam = SETTING_CUST_X;
    } else {
      title=@"Set Endpoint";
      settingParam = (sender == custMin) ? SETTING_CUST_MIN : SETTING_CUST_MAX;
    }
    settingLabel = [[[UILabel alloc] initWithFrame:[UIScreen mainScreen].bounds] autorelease];
    [settingLabel setFont:[UIFont systemFontOfSize:16.0]];
  }
  UIActionSheet *act = [[[UIActionSheet alloc] initWithTitle:title delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:@"Set" otherButtonTitles:nil] autorelease];
  if (settingLabel) {
    UIViewController *viewCon = [[UIViewController alloc] autorelease];
    viewCon.view = settingLabel;
    [self presentModalViewController:viewCon animated:NO];
    [act showInView:settingLabel];
  }
//	[act addSubview:settingLabel];
  else [act showInView:self.view];
  m_fixOrientation = self.interfaceOrientation;
  [UIAccelerometer sharedAccelerometer].delegate = self;
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == [actionSheet destructiveButtonIndex])
	{
		if (settingParam == SETTING_VERT)
		{
			saveVerticalState(minY, maxY, minX, maxX, invert.on);
			//labels up-to-date, were changed by didAccelerate
			if (![[NSUserDefaults standardUserDefaults] boolForKey:USE_CUSTOM_TILT]) setVerticalTiltAxes(minY, maxY, minX, maxX, invert.on);
		}
		else
		{
			//update label and param
			Vec3 min, max, x;
			getCustomState(min, max, x);
			switch (settingParam)
			{
				case SETTING_CUST_MIN:
					[custMin setTitle:settingLabel.text forState:UIControlStateNormal];
					min = setVec;
					break;
				case SETTING_CUST_MAX:
					[custMax setTitle:settingLabel.text forState:UIControlStateNormal];
					max = setVec;
					break;
				case SETTING_CUST_X:
					Vec3 dir = Vec3Sub(max, min);
					x = Vec3Cross(Vec3Cross(setVec,dir),dir);
					x = Vec3Mul(x, Vec3Len(x));
					ostringstream os;
					os << x;
					[custX setTitle:NSStringFromStdString(os.str()) forState:UIControlStateNormal];
					break;
			}
			saveCustomState(min, max, x);
			if ([[NSUserDefaults standardUserDefaults] boolForKey:USE_CUSTOM_TILT]) setCustomTiltAxes(min, max, x);

		}
	}
	else if (settingParam == SETTING_VERT)
	{
		//no change - reload captions from stored VERTICAL_TILT_STRING
		[self loadVerticalLabels];
	}
	//else, was setting custom; ignore.
    [UIAccelerometer sharedAccelerometer].delegate = oldDeleg;
	if (settingLabel) [self dismissModalViewControllerAnimated:NO];
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration {
  float x,y;
  switch(m_fixOrientation) {
    case UIInterfaceOrientationLandscapeLeft:
      x = acceleration.y; y=-acceleration.x; break;
    case UIInterfaceOrientationLandscapeRight:
      x = -acceleration.y; y=acceleration.x; break;
    default:
      DASHER_ASSERT(false);
    case UIInterfaceOrientationPortrait:
      x=acceleration.x; y=acceleration.y; break;
  }
  if (settingParam == SETTING_VERT) {
		minY = std::min(minY, y);
		maxY = std::max(maxY, y);
		vertMin.text = [NSString stringWithFormat:@"%1.3f",minY];
		vertMax.text = [NSString stringWithFormat:@"%1.3f",maxY];
		minX = std::min(minX, x);
		maxX = std::max(maxX, x);
		vertX.text = [NSString stringWithFormat:@"%1.3f - %1.3f", minX, maxX];
	}
	else
		settingLabel.text = [NSString stringWithFormat:@"(%1.3f,%1.3f,%1.3f)",
							 setVec.x=x,
							 setVec.y=y,
							 setVec.z=acceleration.z];
}

- (void)invertChanged {
  float y1, y2, x1, x2;
  BOOL oldInvert;
  getVerticalState(y1, y2, x1, x2, oldInvert);
  //chances are, that oldInvert = !invert.on...but not relying on that.
  saveVerticalState(y1, y2, x1, x2, invert.on);
  if (![[NSUserDefaults standardUserDefaults] boolForKey:USE_CUSTOM_TILT]) setVerticalTiltAxes(y1, y2, x1, x2, invert.on);
}
	
-(void) loadVerticalLabels {
	float y1, y2, x1, x2;
	BOOL bInvert;
	getVerticalState(y1, y2, x1, x2, bInvert);
	vertMin.text = [NSString stringWithFormat:@"Min: %1.3f", y1];
	vertMax.text = [NSString stringWithFormat:@"Max: %1.3f", y2];
	vertX.text = [NSString stringWithFormat:@"Minor axis: %1.3f - %1.3f", x1, x2];
	invert.on = bInvert;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    return (section == 0) ? @"Vertical" : @"Custom";
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger) section {
	return 40.0f;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger) section {
    if (!headers[section])
	{
		UIView *header = headers[section] = [[UIView alloc] init];
        UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 10.0, 200.0, 20.0)] autorelease];
        label.font = [UIFont boldSystemFontOfSize:18];
		label.textColor = [UIColor grayColor];
		label.backgroundColor = [UIColor clearColor];
		label.text = [self tableView:tableView titleForHeaderInSection:section];
		[header addSubview:label];
	
		UIButton *sel = selButtons[section] = [UIButton buttonWithType:UIButtonTypeRoundedRect];
		[sel setTitle:@"Select" forState:UIControlStateNormal];
		[sel setTitle:@"(Selected)" forState:UIControlStateDisabled];
		[sel setTitleColor:[UIColor grayColor] forState:UIControlStateDisabled];
		[header addSubview:sel];
		sel.frame = CGRectMake(210.0, 7.0, 100.0, 25.0);
		[sel addTarget:self action:@selector(selectClicked:) forControlEvents:UIControlEventTouchUpInside];
		NSInteger currentlySelected = [[NSUserDefaults standardUserDefaults] boolForKey:USE_CUSTOM_TILT] ? 1 : 0; 
		sel.enabled = (section != currentlySelected);
	}
	return headers[section];
}

- (void)selectClicked:(id)sender {
	UIButton *button = (UIButton *)sender;
	for (int i=0; i<3; i++) {
		DASHER_ASSERT(i < 2);
		if (button == selButtons[i])
		{
			button.enabled = NO;
			selButtons[1-i].enabled = YES;
      [[NSUserDefaults standardUserDefaults] setBool:(i==1) forKey:USE_CUSTOM_TILT];
			[CalibrationController doSetup];
			break;
		}
	}
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tableView deselectRowAtIndexPath:indexPath animated:NO];
}
/*	if (selectedPath)
		[tableView cellForRowAtIndexPath:selectedPath].accessoryType = UITableViewCellAccessoryNone;
	[tableView cellForRowAtIndexPath:(selectedPath = indexPath)].accessoryType = UITableViewCellAccessoryCheckmark;
	DasherAppDelegate *app = [DasherAppDelegate theApp];
	SFilterDesc *filter = &allMeths[ [indexPath section] ].filters[ [indexPath row] ];
	app.dasherInterface->SetStringParameter(SP_INPUT_DEVICE, filter->deviceName);
	app.dasherInterface->SetStringParameter(SP_INPUT_FILTER, filter->filterName);
}*/

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
	for (int i=0; i<2; i++) [headers[i] release];
	[vertical release];
	[custom release];
    [super dealloc];
}


@end