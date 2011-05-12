//
//  Actions.h
//  Dasher
//
//  Created by Alan Lawrence on 26/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef struct {
  NSString *dispName;
  NSString *settingName;
  NSString *toolbarIconFile;
} SAction;

//#ifndef __ACTIONS_MM__
extern SAction actions[];
extern const int numActions;
//#endif

@interface ActionButton : UIBarButtonItem <UIActionSheetDelegate> {
  UIToolbar *toolbar;
}
+(ActionButton *)buttonForToolbar:(UIToolbar *)toolbar;
@end
