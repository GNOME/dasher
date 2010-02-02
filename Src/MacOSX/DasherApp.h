//
//  DasherApp.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//


#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import "COSXDasherControl.h"
//#import "DasherAppInterface.h"
#import "COSXDasherScreen.h"
#import "DasherViewCocoa.h"

@class AppWatcher;
@class DasherView;

@interface DasherApp : NSObject
{
  IBOutlet NSPanel *dasherPanelUI;
  
  COSXDasherControl *aquaDasherControl;
  id <DasherViewCocoa>dasherView;
  
  IBOutlet AppWatcher *appWatcher;
  NSTimer *_timer;
  
}

- (void)start;
- (void)redraw;
- (void)changeScreen:(COSXDasherScreen *)aScreen;
- (void)pause;
- (void)unpause:(unsigned long int)time;
- (NSDictionary *)parameterDictionary;
- (NSArray *)permittedValuesForParameter:(int)aParameter;
- (id)getParameterValueForKey:(NSString *)aKey;
- (void)setParameterValue:(id)aValue forKey:(NSString *)aKey;
- (AXUIElementRef)targetAppUIElementRef;
- (id)init;
- (IBAction)importTrainingText:(id)sender;
- (IBAction)showPreferences:(id)sender;
- (void)setPanelAlphaValue:(float)anAlphaValue;
- (void)awakeFromNib;
- (void)windowWillClose:(NSNotification *)aNotification;
- (COSXDasherControl *)aquaDasherControl;
- (void)setAquaDasherControl:(COSXDasherControl *)value;
- (id <DasherViewCocoa>)dasherView;
- (void)setDasherView:(id <DasherViewCocoa>)value;
- (void)startTimer;
- (void)shutdownTimer;
- (NSTimer *)timer;
- (void)setTimer:(NSTimer *)newTimer;
- (void)timerCallback:(NSTimer *)aTimer;
- (void)dealloc;

@end
