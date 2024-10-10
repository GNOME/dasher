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
#import "DasherEdit.h"
#import "KeyboardHelper.h"

@class AppWatcher;
@class DasherTextView;

@interface DasherApp : NSObject <NSWindowDelegate, NSSpeechSynthesizerDelegate>
{
  IBOutlet NSWindow *dasherPanelUI;
  
  COSXDasherControl *aquaDasherControl;
  DasherViewOpenGL *dasherView;
  
  IBOutlet AppWatcher *appWatcher;
  IBOutlet DasherTextView *textView;
  NSTimer *_timer;
  Queue *spQ;
  NSSpeechSynthesizer *spSyn;
  CKeyboardHelper *_keyboardHelper;
  BOOL directMode;
  NSString *filename;
  BOOL modified;
}
@property BOOL gameModeOn;
@property BOOL directMode;
@property (readonly,retain) NSString *filename;
@property BOOL modified;

- (void)redraw;
- (void)changeScreen:(CDasherScreen *)aScreen;
- (NSDictionary *)parameterDictionary;
- (NSArray *)permittedValuesForParameter:(int)aParameter;
- (id)getParameterValueForKey:(NSString *)aKey;
- (void)setParameterValue:(id)aValue forKey:(NSString *)aKey;
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
- (void)timerCallback:(NSTimer *)aTimer;
- (void)dealloc;
- (bool)supportsSpeech;
- (void)speak:(NSString *)sText interrupt:(bool)bInt;
- (void)copyToClipboard:(NSString *)sText;
- (void)handleKeyDown:(NSEvent *)e;
- (void)handleKeyUp:(NSEvent *)e;
- (IBAction)newDoc:(id)sender;
- (IBAction)openDoc:(id)sender;
- (IBAction)saveDoc:(id)sender;
- (IBAction)saveDocAs:(id)sender;
- (IBAction)revertToSaved:(id)sender;
- (IBAction)onGameMode:(id)sender;
- (IBAction)onDirectMode:(id)sender;

@property (assign) DasherViewOpenGL *dasherView;

@end
