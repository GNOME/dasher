//
//  StringParamController.mm
//  Dasher
//
//  Created by Alan Lawrence on 20/04/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import <string>
#import <vector>

#import "StringParamController.h"
#import "DasherAppDelegate.h"
#import "Parameters.h"
#import "DasherUtil.h"

@implementation StringParamController

- (id)initWithTitle:(NSString *)title image:(UIImage *)image settingParam:(int)iParam {
    // Override initWithStyle: if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
  if (self = [super initWithStyle:UITableViewStylePlain]) {
		self.tabBarItem.title = title;
		self.tabBarItem.image = image;
    m_iParam = iParam;
  }
  return self;
}


- (void)viewDidLoad {
    [super viewDidLoad];

	vector<string> alist;
	[DasherAppDelegate theApp].dasherInterface->GetPermittedValues(m_iParam, alist);
	
	NSMutableArray *temp = [NSMutableArray arrayWithCapacity:alist.size()];
	
	for (vector<string>::iterator it = alist.begin(); it != alist.end(); it++)
	{
		[temp addObject:NSStringFromStdString(*it)];
	}
	
	items = [NSArray arrayWithArray:temp];
	[items retain];
	
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

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
    return 1;
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	return [items count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
    }
    
    // Set up the cell...
	NSString *s = [items objectAtIndex:[indexPath row]];
	if ([s isEqualToString:NSStringFromStdString([DasherAppDelegate theApp].dasherInterface->GetStringParameter(m_iParam))])
	{
		cell.accessoryType = UITableViewCellAccessoryCheckmark;
		if (selectedPath) [selectedPath release];
		selectedPath = [indexPath retain];
	}
	else
		cell.accessoryType = UITableViewCellAccessoryNone;
	cell.text = s;
    return cell;
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tableView deselectRowAtIndexPath:indexPath animated:NO];
	if (selectedPath)
		[tableView cellForRowAtIndexPath:[selectedPath autorelease]].accessoryType = UITableViewCellAccessoryNone;
	[tableView cellForRowAtIndexPath:(selectedPath = [indexPath retain])].accessoryType = UITableViewCellAccessoryCheckmark;
  [self doSet:[items objectAtIndex:[indexPath row]]];
}

- (void)doSet:(NSString *)val {
	[DasherAppDelegate theApp].dasherInterface->SetStringParameter(m_iParam, StdStringFromNSString(val));
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
	[items release];
    [super dealloc];
}


@end

