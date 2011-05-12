//
//  Actions.mm
//  Dasher
//
//  Created by Alan Lawrence on 26/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//
#define __ACTIONS_MM__

#import "Actions.h"
#import "DasherAppDelegate.h"
#import "ActionConfigurator.h"

SAction actions[] = {
  {@"Email",@"iphone_act_email", @"mail.png"},
  {@"Speak",@"iphone_act_speak", @"bubble.png"},
  {@"Speak and Clear",@"iphone_act_speak_clear", @"bubbletrash.png"},
  {@"Copy to Clipboard",@"iphone_act_copy", @"copy.png"},
  {@"Cut to Clipboard", @"iphone_act_cut", @"scissors.png"},
  {@"Paste from Clipboard", @"iphone_act_paste", @"paste.png"},
  {@"Discard",@"iphone_act_discard",@"trash.png"},
};

const int numActions = sizeof(actions) / sizeof(actions[0]);

@interface ActionButton ()
- (void)performAction:(int)which checkClear:(BOOL)bCheck;
@end;

int numActionsOn;
int actionsOn[numActions];

static NSString *actionIconFile = @"spanner.png";

ActionButton *sysTrash=nil, *general=nil;

@implementation ActionButton

+(ActionButton *)buttonForToolbar:(UIToolbar *)_toolbar {
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
  if (numActionsOn==1 && actionsOn[0]==6) {
    if (!sysTrash) {
      sysTrash = [[ActionButton alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemTrash target:nil action:@selector(clicked)];
      sysTrash.target = sysTrash;
    }
    sysTrash->toolbar = _toolbar;
    return sysTrash;
  }
  if (numActionsOn!=1) iconFile = actionIconFile;
  if (!general) {
    general = [[ActionButton alloc] initWithImage:[UIImage imageNamed:actionIconFile] style:UIBarButtonItemStylePlain target:nil action:@selector(clicked)];
    general.target = general;
  } else
    [general setImage:[UIImage imageNamed:iconFile]];
  general->toolbar = _toolbar;
  return general;
}

- (void)clicked {
  if (numActionsOn==0) {
    //no actions enabled! display configurator...
    [[DasherAppDelegate theApp] presentModalViewController:[[[UINavigationController alloc] initWithRootViewController:[[[ActionConfigurator alloc] init] autorelease]] autorelease] animated:YES];
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
      case 7:
        choice = [[[UIActionSheet alloc] initWithTitle:@"Actions" delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:actions[actionsOn[0]].dispName,actions[actionsOn[1]].dispName,actions[actionsOn[2]].dispName,actions[actionsOn[3]].dispName,actions[actionsOn[4]].dispName,actions[actionsOn[5]].dispName,actions[actionsOn[6]].dispName,nil] autorelease];
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