//
//  U.m
//  Dasher
//
//  Created by Alan Lawrence on 26/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import "Actions.h"
#import "DasherAppDelegate.h"

@interface ActionButton ()
- (void)performAction:(int)which checkClear:(BOOL)bCheck;
- (void)scan;
- (UIViewController *)navConfigurator;
@end;

@interface ActionConfigurator : UITableViewController {
  ActionButton *button;
}
-(id)initWithButton:(ActionButton *)_button;
@end

typedef struct {
  NSString *dispName;
  NSString *settingName;
  NSString *toolbarIconFile;
} SAction;

static SAction actions[] = {
  {@"Email",@"iphone_act_email", @"mail.png"},
  {@"Speak",@"iphone_act_speak", @"bubble.png"},
  {@"Speak and Clear",@"iphone_act_speak_clear", @"bubbletrash.png"},
  {@"Copy to Clipboard",@"iphone_act_copy", @"copy.png"},
  {@"Cut to Clipboard", @"iphone_act_cut", @"scissors.png"},
  {@"Paste from Clipboard", @"iphone_act_paste", @"paste.png"},
};

static const int numActions = sizeof(actions) / sizeof(actions[0]);

static NSString *actionIconFile = @"spanner.png";

@implementation ActionButton

-(id)initForToolbar:(UIToolbar *)_toolbar {
  if (self = [super initWithImage:[UIImage imageNamed:actionIconFile] style:UIBarButtonItemStylePlain target:self action:@selector(clicked)]) {
    toolbar = _toolbar;
    actionsOn = new int[numActions];
    [self scan];
  }
  return self;
}

-(UIViewController *)tabConfigurator {
  ActionConfigurator *conf=[[[ActionConfigurator alloc] initWithButton:self] autorelease];
  //for a tab in the settings tabcontroller...
  conf.tabBarItem.title=@"Actions";
  conf.tabBarItem.image=[UIImage imageNamed:@"spanner_lg.png"];
  return conf;
}

-(UIViewController *)navConfigurator {
  ActionConfigurator *conf = [[[ActionConfigurator alloc] initWithButton:self] autorelease];
  //for root of a navigationcontroller...
  conf.navigationItem.title=@"Configure Actions";
  conf.navigationItem.leftBarButtonItem = [[[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(settingsDone)] autorelease];
  return conf;
}

-(void)dealloc {
  delete[] actionsOn;
  [super dealloc];
}

-(void)scan {
  numActionsOn=0;
  NSUserDefaults *settings=[NSUserDefaults standardUserDefaults];
  NSString *iconFile=nil;
  for (int i=0; i<numActions; i++) {
    if ([settings boolForKey:actions[i].settingName]) {
      actionsOn[numActionsOn++]=i;
      iconFile = actions[i].toolbarIconFile;
    }
  }
  //just to be safe, fill to end of actionsOn array with -1s:
  for (int i=numActionsOn; i<numActions; i++) actionsOn[i]=-1;
  
  //multiple items, _or_ none (=>configure), display generic actions/tools icon
  if (numActionsOn!=1) iconFile = actionIconFile;
  [self setImage:[UIImage imageNamed:iconFile]];
}

- (void)clicked {
  if (numActionsOn==0) {
    //no actions enabled! display configurator...
    [[DasherAppDelegate theApp] presentModalViewController:[[[UINavigationController alloc] initWithRootViewController:[self navConfigurator]] autorelease] animated:YES];
  } else if (numActionsOn==1) {
    //a single action is enabled...
    [self performAction:actionsOn[0] checkClear:YES];
  } else {
    //multiple actions enabled; display a menu to choose...
    
    //we display the choice to the user as a UIActionSheet, with one button per enabled action plus 'cancel'.
    // Unfortunately, to get the 'cancel' button to display properly, the API requires we pass in all the button titles
    // to the constructor as a varargs array - so the number of args is statically hard-coded as part of the call site
    // (there is no portable/guaranteed way to pass in a standard NSArray or NSString **. There may be implementation-dependent
    // hacks that happen to work atm but these are not part of the API/spec and hence not to be relied upon!!)
    UIActionSheet *choice;
    switch (numActionsOn) {
      case 2:
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:actions[actionsOn[0]].dispName,actions[actionsOn[1]].dispName,nil] autorelease];
        break;
      case 3:
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:actions[actionsOn[0]].dispName,actions[actionsOn[1]].dispName,actions[actionsOn[2]].dispName,nil] autorelease];
        break;
      case 4:
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:actions[actionsOn[0]].dispName,actions[actionsOn[1]].dispName,actions[actionsOn[2]].dispName,actions[actionsOn[3]].dispName,nil] autorelease];
        break;
      case 5:
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:actions[actionsOn[0]].dispName,actions[actionsOn[1]].dispName,actions[actionsOn[2]].dispName,actions[actionsOn[3]].dispName,actions[actionsOn[4]].dispName,nil] autorelease];
        break;
      case 6:
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:actions[actionsOn[0]].dispName,actions[actionsOn[1]].dispName,actions[actionsOn[2]].dispName,actions[actionsOn[3]].dispName,actions[actionsOn[4]].dispName,actions[actionsOn[5]].dispName,nil] autorelease];
        break;
      default:
        //ok, some other number! But implementing for future-proofing...
        // We don't use this method normally because the cancel button will appear in the wrong place (at the top!),
        // but it'll do as a fallback...
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:nil] autorelease];
        for (int i = 0; i<numActionsOn; i++)
          [choice addButtonWithTitle:actions[actionsOn[i]].dispName];
        break;
      //case 0, case 1: handled above
    }
    [choice showFromToolbar:toolbar];
  }
}

-(void)settingsDone {
  [[DasherAppDelegate theApp] dismissModalViewControllerAnimated:YES];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
  if (buttonIndex == [actionSheet cancelButtonIndex]) return;
  
  //buttons should all be before cancel, but if we used fallback case above, they might be after; be defensive...
  if (buttonIndex > [actionSheet cancelButtonIndex]) buttonIndex--;
  buttonIndex -= [actionSheet firstOtherButtonIndex];

  //the actionsOn array should have been setup when the UIActionSheet was posted...
  int which = actionsOn[buttonIndex];
  DASHER_ASSERT(which!=-1);
  if (which!=-1) [self performAction:which checkClear:NO];
}

- (void)performAction:(int)which checkClear:(BOOL)bCheck {
  DasherAppDelegate *deleg = [DasherAppDelegate theApp];
  NSString *theText = [deleg allText];
  switch (which) {
    case 0: { //email
      NSString *mailString = [NSString stringWithFormat:@"mailto:?body=%@", 
                              [theText stringByAddingPercentEscapesUsingEncoding:NSASCIIStringEncoding]];
      [[UIApplication sharedApplication] openURL:[NSURL URLWithString:mailString]];      
      return; //tho we won't actually get even here...
    }
    case 1: //speak
    case 2:
      [deleg speak:theText interrupt:YES];
      if (which==1) return;
      //continue after switch to clear
      break;
    case 3:
    case 4:
      [deleg copy:theText];
      if (which==3) return;
      //continue after switch to clear
      break;
    case 5:
      [deleg insertText:[UIPasteboard generalPasteboard].string];
      return;
  }
  //clear...
  if (bCheck) [deleg clearBtn]; else [deleg clearText];
}

@end

@implementation ActionConfigurator

- (id)initWithButton:(ActionButton *)_button {
  if (self = [super initWithStyle:UITableViewStylePlain]) {
    button = _button;
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

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
  return numActions;
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    UISwitch *sw;
  
    if (cell == nil) {
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
      sw = [[[UISwitch alloc] initWithFrame:CGRectMake(210.0,5.0,100.0,20.0)] autorelease];
      [sw addTarget:self action:@selector(slid:) forControlEvents:UIControlEventValueChanged];
      sw.tag=1; 
      [cell addSubview:sw];
    } else {
      DASHER_ASSERT([[cell viewWithTag:1] isKindOfClass:[UISwitch class]]);
      sw = (UISwitch *)[cell viewWithTag:1];
    }

    
    // Configure the cell...
  SAction *act = &actions[ [indexPath row] ];
  cell.text = act->dispName;
  cell.tag = [indexPath row];
  sw.on = [[NSUserDefaults standardUserDefaults] boolForKey:act->settingName];
  return cell;
}

- (void)slid:(id)sender {
  DASHER_ASSERT([sender isKindOfClass:[UISwitch class]]);
  UISwitch *sw = (UISwitch *)sender;
  DASHER_ASSERT([[sw superview] isKindOfClass:[UITableViewCell class]]);
  SAction *act = &actions[ [sw superview].tag ];
  [[NSUserDefaults standardUserDefaults] setBool:sw.on forKey:act->settingName];
  [button scan];
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

