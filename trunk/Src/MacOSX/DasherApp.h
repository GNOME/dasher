//
//  DasherApp.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>

@ interface DasherApp:NSObject {
}

-(IBAction) importTrainingText:(id) sender;
-(IBAction) showPreferences:(id) sender;
-(IBAction) showDasherPanel:(id) sender;

@end
