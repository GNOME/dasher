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
  SModuleSettings *settings;
  int count;
  id targetOnDone;
  SEL selector;
}

-(id)initWithTitle:(NSString *)title Settings:(SModuleSettings *)settings Count:(int)count;
-(void)setTarget:(id)target Selector:(SEL)selector;

@end
