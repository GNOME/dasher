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
#include <string>

@class DasherView;
@class NSTimer, NSWindow, NSTextView, NSSplitView;

NSString *NSStringFromStdString(const std::string& aString);

unsigned long int get_time();

@interface DasherApp : NSObject
{
    IBOutlet DasherView *dasherView;
    IBOutlet NSWindow *dasherWindow;
    IBOutlet NSTextView *dasherEdit;

    NSTimer *_timer;
    int flushCount;
}

- (NSTimer *)timer;
- (void)setTimer:(NSTimer *)newTimer;

- (void)blankCallback;
- (void)displayCallback;

- (IBAction)importTrainingText:(id)sender;
- (IBAction)changeSpeed:(id)sender;

- (IBAction)showPreferences:(id)sender;

- (void)startDashing;
- (void)stopDashing;
- (BOOL)isDashing;

@end
