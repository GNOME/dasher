//
//  DasherApp.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherApp.h"
#import "PreferencesController.h"
#import "DasherEdit.h"
#import "DasherUtil.h"
#import "DasherViewCocoa.h"
#import "AppWatcher.h"
#import "COSXDasherScreen.h"

/*
 * Created by Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com), 18 April 2003
 */

#import <Cocoa/Cocoa.h>


@implementation DasherApp

- (void)start {
//  aquaDasherControl->Start();
}

- (void)redraw {
  aquaDasherControl->ScheduleRedraw();
}

- (void)changeScreen:(COSXDasherScreen *)aScreen {
  aquaDasherControl->ChangeScreen( aScreen );
}

- (void)pauseAtX:(int)x y:(int)y {
  aquaDasherControl->PauseAt(x, y);
}

- (void)unpause:(unsigned long int)time {
  aquaDasherControl->Unpause( time );
}

- (NSDictionary *)parameterDictionary {
  return aquaDasherControl->ParameterDictionary();
}

- (NSArray *)permittedValuesForParameter:(int)aParameter {
  
  vector< string > alist;
  
  aquaDasherControl->GetPermittedValues(aParameter, alist);
  
  NSMutableArray *result = [NSMutableArray arrayWithCapacity:alist.size()];
  
  for (vector<string>::iterator it = alist.begin(); it != alist.end(); it++)
    {
    [result addObject:NSStringFromStdString(*it)];
    }
  
  return result;
}


- (id)getParameterValueForKey:(NSString *)aKey {
  return aquaDasherControl->GetParameter(aKey);
}

- (void)setParameterValue:(id)aValue forKey:(NSString *)aKey {
  aquaDasherControl->SetParameter(aKey, aValue);
}

- (AXUIElementRef)targetAppUIElementRef {
  return [appWatcher targetAppUIElementRef];
}

- (id)init
{
  if (self = [super init])
    {
    [self setAquaDasherControl:new COSXDasherControl(self)];
    }

  return self;
}

- (IBAction)importTrainingText:(id)sender
{
  NSOpenPanel *op = [NSOpenPanel openPanel];

  int returnCode = [op runModalForDirectory:nil file:nil types:nil];

  if (returnCode == NSOKButton)
    {
    aquaDasherControl->Train([op filename]);
    NSBeep();
    }
}

- (IBAction)showPreferences:(id)sender
{
  [[PreferencesController preferencesController] makeKeyAndOrderFront:sender];
}

- (void)setPanelAlphaValue:(float)anAlphaValue
{
  [dasherPanelUI setAlphaValue:anAlphaValue];
}

- (void)finishRealization {
  
  aquaDasherControl->Realize2();
  [dasherView finishRealization];
  
}

- (void)awakeFromNib
{
  [dasherPanelUI setBecomesKeyOnlyIfNeeded:YES];
  [dasherPanelUI setFloatingPanel:YES];
  
  // TODO leave out until defaults works properly
//  [self setPanelAlphaValue:[[NSUserDefaults standardUserDefaults] floatForKey:DASHER_PANEL_OPACITY]];
  
  // not sure if this is the right place for this.  if we ever have a local/remote switch (to control typing into self or others, it should probably be turned off/on there)
  // not sure if this is the right call; is there an equivalent in the AXUI API?  I can't find it...
  // default value for seconds is 0.25, which makes the app miss eg mouse presses sometimes
//  CGSetLocalEventsSuppressionInterval(0.1);
  [self performSelector:@selector(finishRealization) withObject:nil afterDelay:0.0];
}

- (void)windowWillClose:(NSNotification *)aNotification {
  // terminate the app when the Dasher window closes.  N.B. can't do this with the NSApplication
  // delegate method applicationShouldTerminateAfterLastWindowClosed: because we have no windows
  // only panels, and when any panel is closed (like prefs!) the app terminates
  [[NSApplication sharedApplication] terminate:self];
}

- (COSXDasherControl *)aquaDasherControl {
  return aquaDasherControl;
}

- (void)setAquaDasherControl:(COSXDasherControl *)value {
    aquaDasherControl = value;
}

- (id <DasherViewCocoa>)dasherView {
  return dasherView;
}

- (void)setDasherView:(id <DasherViewCocoa>)value {
    dasherView = value;
  }

- (void)startTimer {
#define FPS 40.0f
  
  NSTimer *timer = [NSTimer timerWithTimeInterval:(1.0f/FPS) target:self selector:@selector(timerCallback:) userInfo:nil repeats:YES];
  [self setTimer:timer];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode];
  
//  [self setTimer:[NSTimer scheduledTimerWithTimeInterval:1.0/FPS target:self selector:@selector(timerCallback:) userInfo:nil repeats:YES]];
}

- (void)shutdownTimer {
  [self setTimer:nil];
}

- (NSTimer *)timer {
  return _timer;
}

- (void)setTimer:(NSTimer *)newTimer {
  if (_timer != newTimer) {
    NSTimer *oldValue = _timer;
    _timer = [newTimer retain];
    [oldValue invalidate];
    [oldValue release];
  }
}

- (void)timerCallback:(NSTimer *)aTimer
{
  aquaDasherControl->TimerFired([dasherView mouseLocation]);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
  aquaDasherControl->StartShutdown();
}

- (void)dealloc {
  [self setTimer:nil];
  [super dealloc]; 
}  

@end
