//
//  ActionConfigurator.mm
//  Dasher
//
//  Created by Alan Lawrence on 26/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import "ActionConfigurator.h"
#import "Parameters.h"
#import "DasherAppDelegate.h"
#import "DasherUtil.h"

int OTHER_BPS[] = {BP_COPY_ALL_ON_STOP, BP_SPEAK_ALL_ON_STOP, BP_SPEAK_WORDS};

using Dasher::Settings::GetParameterName;

@implementation ActionConfigurator

- (id)init {
  if (self = [super initWithStyle:UITableViewStyleGrouped]) {
    self.tabBarItem.title=@"Actions";
    self.tabBarItem.image=[UIImage imageNamed:@"spanner_lg.png"];
    self.navigationItem.title=@"Configure Actions";
    self.navigationItem.leftBarButtonItem = [[[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:[DasherAppDelegate theApp] action:@selector(settingsDone)] autorelease];
  }
  return self;
}

#pragma mark -
#pragma mark View lifecycle

/*
- (void)viewDidLoad {
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    self.clearsSelectionOnViewWillAppear = NO;
 
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


#pragma mark -
#pragma mark Table view data source

-(UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
  if (!headers[section]) {
    UIView *header = headers[section] = [[UIView alloc] init];
    UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, 10.0, 200.0, 20.0)] autorelease];
    label.font = [UIFont boldSystemFontOfSize:18];
    label.textColor = [UIColor grayColor];
    label.backgroundColor = [UIColor clearColor];
    label.text = [self tableView:tableView titleForHeaderInSection:section];
    [header addSubview:label];
  }
  return headers[section];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
  switch (section) {
    case 0: return @"Triggers";
    case 1: return @"Actions Menu";
  }
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger) section {
  return 40.0f;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  switch (section) {
    case 0:
      return sizeof(OTHER_BPS)/sizeof(OTHER_BPS[0]);
    case 1:
      return numActions;
  }
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    UISwitch *sw;
  
    if (cell == nil) {
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:CellIdentifier] autorelease];
      sw = [[[UISwitch alloc] initWithFrame:CGRectMake(200.0,5.0,80.0,18.0)] autorelease];
      [cell.contentView addSubview:sw];
    } else {
      sw = (UISwitch *)[cell.contentView viewWithTag:cell.tag];
      DASHER_ASSERT([sw isKindOfClass:[UISwitch class]]);
    }
  sw.enabled=YES;
  // Configure the cell...
    if ([indexPath section]==1) {
      SAction *act = &actions[ [indexPath row] ];
      [sw addTarget:self action:@selector(actionSlid:) forControlEvents:UIControlEventValueChanged];
      cell.textLabel.text = act->dispName;
      //ensure we don't assign tag 0: 0 is default, so 'viewWithTag:0' could
      // return any subview.
      sw.tag = cell.tag = [indexPath row] + 1;
      sw.on = [[NSUserDefaults standardUserDefaults] boolForKey:act->settingName];      
    } else {
      CDasherInterfaceBridge *intf=[DasherAppDelegate theApp].dasherInterface;
      int *params;
      DASHER_ASSERT([indexPath section]==0);
      params=OTHER_BPS;
      int iParameter = params[[indexPath row]];
      [sw addTarget:self action:@selector(paramSlid:) forControlEvents:UIControlEventValueChanged];
      cell.textLabel.text = NSStringFromStdString(GetParameterName(iParameter));
      //ensure we don't assign tag 0: 0 is default, so 'viewWithTag:0' could
      // return any subview.
      sw.tag = cell.tag = iParameter + 1;
      sw.on = intf->GetBoolParameter(iParameter);
    }
  return cell;
}

- (void)actionSlid:(id)sender {
  DASHER_ASSERT([sender isKindOfClass:[UISwitch class]]);
  UISwitch *sw = (UISwitch *)sender;
  SAction *act = &actions[ sw.tag -1 ];
  [[NSUserDefaults standardUserDefaults] setBool:sw.on forKey:act->settingName];
  [[DasherAppDelegate theApp] refreshToolbar];
}

-(void)paramSlid:(id)sender {
  DASHER_ASSERT([sender isKindOfClass:[UISwitch class]]);
  UISwitch *sw = (UISwitch *)sender;
  int iParameter = sw.tag - 1; //we added 1 on when we assigned the tag. 
  [DasherAppDelegate theApp].dasherInterface->SetBoolParameter(iParameter,sw.on);
  if (iParameter==BP_CONTROL_MODE) {
    //find the table view...
    UITableView *tv;
    for (UIView *v = sw; ;v=[v superview]) {
      if ([v isKindOfClass:[UITableView class]]) {
        tv = (UITableView *)v;
        break;
      }
    }
  }
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


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  [tableView deselectRowAtIndexPath:indexPath animated:NO];
    // Navigation logic may go here. Create and push another view controller.
	/*
	 <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
	 [self.navigationController pushViewController:detailViewController animated:YES];
	 [detailViewController release];
	 */
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
  [super dealloc];
}


@end

