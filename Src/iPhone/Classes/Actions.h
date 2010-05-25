//
//  U.h
//  Dasher
//
//  Created by Alan Lawrence on 26/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ActionButton : UIBarButtonItem <UIActionSheetDelegate> {
  UIToolbar *toolbar;
  int numActionsOn, *actionsOn;
}
- (UIViewController *)tabConfigurator;
- (id)initForToolbar:(UIToolbar *)toolbar;

@end
