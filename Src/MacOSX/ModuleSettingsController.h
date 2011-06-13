//
//  ModuleSettingsController.h
//  Dasher
//
//  Created by Alan Lawrence on 10/02/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "COSXDasherControl.h"

@interface ModuleSettingsController : NSWindowController {
  COSXDasherControl *intf;
  SModuleSettings *settings;
  int count;
}

///Create a parameters controller whose window has the specified title and allows to adjust the provided settings.
-(id)initWithTitle:(NSString *)title Interface:(COSXDasherControl *)intf Settings:(SModuleSettings *)settings Count:(int)count;

///Show the window modally. (blocks until window dismissed)
-(void)showModal;

@end
