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
#import "DasherViewOpenGL.h"
#import "Queue.h"

@class AppWatcher;
@class DasherView;

@interface DasherApp : NSObject
{
  IBOutlet NSPanel *dasherPanelUI;
  
  COSXDasherControl *aquaDasherControl;
  DasherViewOpenGL *dasherView;
  
  IBOutlet AppWatcher *appWatcher;
  NSTimer *_timer;
  Queue *spQ;
  NSSpeechSynthesizer *spSyn;
}

- (void)start;
- (void)redraw;
- (void)changeScreen:(CDasherScreen *)aScreen;
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
- (void)startTimer;
- (void)shutdownTimer;
- (NSTimer *)timer;
- (void)setTimer:(NSTimer *)newTimer;
- (void)timerCallback:(NSTimer *)aTimer;
- (void)dealloc;
- (bool)supportsSpeech;
- (void)speak:(NSString *)sText interrupt:(bool)bInt;
- (void)copyToClipboard:(NSString *)sText;

@property (assign) DasherViewOpenGL *dasherView;
@end
