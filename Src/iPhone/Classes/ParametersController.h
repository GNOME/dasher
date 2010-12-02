//
//  ParametersController.h
//  Dasher
//
//  Created by Alan Lawrence on 10/02/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ModuleSettings.h"

@interface ParametersController : UIViewController {
  SModuleSettings *m_pSettings;
  int m_iCount;
}

-(id)initWithTitle:(NSString *)title Settings:(SModuleSettings *)settings Count:(int)count;
-(void)setTarget:(id)target Selector:(SEL)selector;

///These are meant to be protected...
-(int)layoutOptionsOn:(UIView *)view startingAtY:(int)y;
-(int)layoutModuleSettings:(SModuleSettings *)settings count:(int)count onView:(UIView *)view startingAtY:(int)y;
-(UISwitch *)makeSwitch:(NSString *)title onView:(UIView *)view atY:(int *)pY;
-(int)makeNoSettingsLabelOnView:(UIView *)view atY:(int)y;
@end
