//
//  DasherApp.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 20 April 2003
 */

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>


@interface DasherApp : NSObject
{
}

- (IBAction)importTrainingText:(id)sender;
- (IBAction)showPreferences:(id)sender;

@end
