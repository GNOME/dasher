//
//  DasherApp.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherApp.h"
#import "PreferencesController.h"
#import "DasherUtil.h"
#import "DasherTextView.h"
#import "config.h"

/*
 * Created by Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com), 18 April 2003
 */

#import <Cocoa/Cocoa.h>

@interface FilenameTransformer : NSValueTransformer {
}
@end
@implementation FilenameTransformer
+(Class)transformedValueClass {
  return [NSString class];
}
+(BOOL)allowsReverseTransformation {
  return NO;
}
-(id)transformedValue:(id)value {
  return (value) ? value : @"Untitled";
}
@end
static NSString *FilenameToUntitledName = @"NilToUntitled";

@interface DasherApp ()
@property (retain) NSString *filename;
-(NSString *)filenameOrUntitled;
//invokes selector on self with nil parameter, if successfully saved or user chose to discard
-(void)promptToSave:(SEL)ifDone;
@end


@implementation DasherApp

@synthesize dasherView;
@synthesize modified;
@synthesize filename;

+(void)initialize {
  [NSValueTransformer setValueTransformer:[[[FilenameTransformer alloc] init] autorelease] forName:FilenameToUntitledName];
}

-(NSString *)filenameOrUntitled {
  return [[NSValueTransformer valueTransformerForName:FilenameToUntitledName] transformedValue:self.filename];
}

- (void)redraw {
  aquaDasherControl->ScheduleRedraw();
}

- (void)changeScreen:(CDasherScreen *)aScreen {
  aquaDasherControl->ChangeScreen( aScreen );
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

- (id)init
{
  if (self = [super init])
    {
      [self setAquaDasherControl:new COSXDasherControl(self)];
      spQ = [[Queue alloc] init];
      _keyboardHelper = new CKeyboardHelper();
      dasherPanelUI.delegate = self;
    }

  return self;
}

- (IBAction)importTrainingText:(id)sender {
  NSOpenPanel *op = [NSOpenPanel openPanel];
  //[op setDirectory:<#(NSString *)path#>];
  //[op setAllowedFileTypes:<#(NSArray *)types#>];
  
  //following method is deprecated in 10.6, and one should use
  // beginSheetModalForWindow:CompletionHandler: instead. The new
  // method (taking a closure/block) doesn't even exist before 10.6,
  // so I'm sticking with:
  [op beginSheetForDirectory:nil file:nil modalForWindow:dasherPanelUI modalDelegate:self didEndSelector:@selector(importPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)importPanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
  [sheet orderOut:dasherPanelUI];
  if (returnCode == NSOKButton) {
    aquaDasherControl->Train([[sheet URL] path]);
    NSBeep();
  }
}

- (IBAction)showPreferences:(id)sender {
  [[PreferencesController preferencesController] makeKeyAndOrderFront:sender];
}

-(BOOL)gameModeOn {
  return aquaDasherControl->GetGameModule()!=NULL;
}

-(void)setGameModeOn:(BOOL)bVal {
  //called from the main event loop when game mode menu item is clicked.
  //If we try to set BP_GAME_MODE now, and we can't find any game sentences,
  // we'll clear BP_GAME_MODE before the first call to SetBoolParam returns.
  //Then, later in the same event handling code for the first menu click,
  // the game mode menu item will be checked - even tho game mode is not on.
  //Hence, let the event handler for the menu finish first:
  [self performSelectorOnMainThread:@selector(doSetGameMode:) withObject:(bVal ? self : nil) waitUntilDone:NO];
}

-(void)doSetGameMode:(id)obj {
  //we can now try and startup game mode in the core;
  // with no automatic checking of the menuitem pending, any changes we make
  // to the gameModeOn property will be correctly reflected in the menu... 
  if (obj) {
    if (directMode) {
      self.directMode = false; //turn off direct mode first _if_necessary_ (properties do not check for no-change)
      appWatcher.directMode = false;
    }
    if (!aquaDasherControl->GetGameModule())
      aquaDasherControl->EnterGameMode(NULL);
  } else if (aquaDasherControl->GetGameModule())
    aquaDasherControl->LeaveGameMode();
}

-(void)setDirectMode:(BOOL)bVal {
  //hidden-ness of textview / appwatcher controls are automatically linked to property changes
  if (bVal) self.gameModeOn=false; //exit game mode
  aquaDasherControl->SetEdit(bVal ? [[[DirectEdit alloc] initWithIntf:aquaDasherControl AppWatcher:appWatcher] autorelease] : textView);
  self->directMode = bVal;
  appWatcher.directMode = bVal;
    
  // Save on local storage.
  NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
  [userDefaults setBool: bVal forKey: DASHER_DIRECT_MODE];
  
  if (bVal) {
    [dasherPanelUI setLevel: NSFloatingWindowLevel];
  } else {
    [dasherPanelUI setLevel: NSNormalWindowLevel];
    [dasherPanelUI makeKeyAndOrderFront: nil];
  }
}

- (BOOL)directMode {
  return self->directMode;
}

- (void)setPanelAlphaValue:(float)anAlphaValue {
  [dasherPanelUI setAlphaValue:anAlphaValue];
}

- (void)finishRealization {
  aquaDasherControl->Realize2();
  
  NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
  BOOL directModeValue = [userDefaults boolForKey: DASHER_DIRECT_MODE];

  [self setDirectMode: directModeValue];
}

- (void)awakeFromNib {
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

- (void)startTimer {
#define FPS 40.0f
  [self shutdownTimer]; //in case there was one before
  NSTimer *timer = [NSTimer timerWithTimeInterval:(1.0f/FPS) target:self selector:@selector(timerCallback:) userInfo:nil repeats:YES];
  _timer = [timer retain];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode];
  
//  [self setTimer:[NSTimer scheduledTimerWithTimeInterval:1.0/FPS target:self selector:@selector(timerCallback:) userInfo:nil repeats:YES]];
}

- (void)shutdownTimer {
  [_timer invalidate];
  [_timer release];
  _timer = nil;
}

- (NSTimer *)timer {
  return _timer;
}

- (void)timerCallback:(NSTimer *)aTimer {
  aquaDasherControl->TimerFired([dasherView mouseLocation]);
}

-(BOOL)windowShouldClose:(id)sender {
  DASHER_ASSERT(sender == dasherPanelUI);
  if (self.modified) {
    [self promptToSave:@selector(doCloseWindow:)];
    return NO;
  }
  return YES;
}

-(void)doCloseWindow:(id)none {
  [dasherPanelUI performClose:nil];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
  [self shutdownTimer];
  aquaDasherControl->WriteTrainFileFull();
  delete aquaDasherControl;
  aquaDasherControl=NULL;
}

- (void)dealloc {
  [self shutdownTimer];
  [super dealloc]; 
}  

- (bool)supportsSpeech {
  if (!spSyn) {
    //hmmm. don't see any way for this to (indicate) fail(ure)...???
    spSyn = [[NSSpeechSynthesizer alloc] init];
    [spSyn setDelegate:self];
  }
  return YES;
}

- (void)speak:(NSString *)sText interrupt:(bool)bInt {
  if (bInt)
    [spQ clear];
  else {
    @synchronized(spQ) {
      if ([spSyn isSpeaking] || [spQ hasItems]) {
        [spQ push:sText];
        return;
      }
    }
  }
  [spSyn startSpeakingString:sText];
}

-(void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)success {
  @synchronized(spQ) {
    if ([spQ hasItems]) {
      [spSyn startSpeakingString:[spQ pop]];
    }
  }
}

-(void)copyToClipboard:(NSString *)sText {
  NSPasteboard *pboard = [NSPasteboard generalPasteboard];
  [pboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
  [pboard setString:sText forType:NSStringPboardType];
}

-(void)handleKeyDown:(NSEvent *)e {
  NSString *chars = [e characters];
  if ([chars length] > 1)
    NSLog(@"KeyDown event for %i chars %@ - what to do? Ignoring all but first...\n", [chars length], chars);
  int keyCode = _keyboardHelper->ConvertKeyCode([chars characterAtIndex:0]);
  if (keyCode != -1)
    aquaDasherControl->KeyDown(get_time(), keyCode);
}

-(void)handleKeyUp:(NSEvent *)e {
  NSString *chars = [e characters];
  if ([chars length] > 1)
    NSLog(@"KeyUp event for %i chars %@ - what to do? Ignoring all but first...\n", [chars length], chars);
  int keyCode = _keyboardHelper->ConvertKeyCode([chars characterAtIndex:0]);
  if (keyCode != -1)
    aquaDasherControl->KeyUp(get_time(), keyCode);  
}

- (void)promptToSave:(SEL)continueSel {
  DASHER_ASSERT(modified);
  [[NSAlert alertWithMessageText:[NSString stringWithFormat:@"Do you want to save the changes you made to the document \"%@\"", [self filenameOrUntitled]] defaultButton:@"Save" alternateButton:@"Cancel" otherButton:@"Discard" informativeTextWithFormat:@"Your changes will be lost if you don't save them"]
   beginSheetModalForWindow:dasherPanelUI modalDelegate:self didEndSelector:@selector(promptedToSave:returnCode:contextInfo:) contextInfo:continueSel];
}
//contextInfo is a selector (taking a single 'id' param) of what to do once we have saved/discarded
- (void)promptedToSave:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo {
  [[alert window] orderOut:dasherPanelUI];
  switch (returnCode) {
    case NSAlertDefaultReturn:
      //Save
      if (!filename){
        [[NSSavePanel savePanel] beginSheetForDirectory:nil file:nil modalForWindow:dasherPanelUI modalDelegate:self didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:contextInfo];
        return;
      }
      [self saveDoc:nil];
      break;
    case NSAlertAlternateReturn:
      return; //cancel => abort
    case NSAlertOtherReturn:
      //don't save => continue
      self.modified=NO;
      break;
  }
  DASHER_ASSERT(!modified);
  [self performSelector:(SEL)contextInfo withObject:nil];
}

- (IBAction)newDoc:(id)sender {
  if (self.modified) {
    [self promptToSave:@selector(newDoc:)];
    return;
  }
  [textView setString:@""];
  aquaDasherControl->SetOffset(0, true);
}

- (IBAction)openDoc:(id)sender {
  if (self.modified) {
    [self promptToSave:@selector(openDoc:)];
    return;
  }
  NSOpenPanel *op = [NSOpenPanel openPanel];
  //[op setDirectory:<#(NSString *)path#>];
  //[op setAllowedFileTypes:<#(NSArray *)types#>];
  [op beginSheetForDirectory:nil file:nil modalForWindow:dasherPanelUI modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)openPanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
  [sheet orderOut:dasherPanelUI];
  if (returnCode == NSOKButton) {
    self.filename = [[sheet URL] path];
    self.modified = NO;
    [self revertToSaved:nil];
  }
}

- (IBAction)saveDoc:(id)sender {
  if (!filename) {
    [self saveDocAs:nil];
    return;
  }
  //TODO errors
  [[textView string] writeToFile:filename atomically:YES encoding:NSUTF8StringEncoding error:nil];
  self.modified = NO;
}

- (IBAction)saveDocAs:(id)sender {
  [[NSSavePanel savePanel] beginSheetForDirectory:nil file:nil modalForWindow:dasherPanelUI modalDelegate:self didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
  [sheet orderOut:dasherPanelUI];
  if (returnCode == NSOKButton) {
    self.filename = [[sheet URL] path];
    [self saveDoc:nil];
    if (contextInfo) [self performSelector:(SEL)contextInfo withObject:nil];
  }
}

- (IBAction)revertToSaved:(id)sender {
  DASHER_ASSERT(filename);
  if (self.modified) {
    NSString *msg = [NSString stringWithFormat:@"Do you want to revert to the most recently saved version of the document \"%@\"",[self filenameOrUntitled]];
    [[NSAlert alertWithMessageText:msg defaultButton:@"Revert" alternateButton:@"Cancel" otherButton:nil informativeTextWithFormat:@"Your current changes will be lost"]
     beginSheetModalForWindow:dasherPanelUI modalDelegate:self didEndSelector:@selector(promptedToRevert:returnCode:contextInfo:) contextInfo:nil];
  } else
    [textView setString:[NSString stringWithContentsOfFile:filename encoding:NSUTF8StringEncoding error:NULL]]; //TODO error
}

-(void)promptedToRevert:(NSAlert *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo {
  if (returnCode==NSAlertDefaultReturn) {
    //revert
    self.modified=NO;
    [self revertToSaved:nil];
  } //else, cancel - do nothing
}

//"private" method, used by COSXGameModule
-(NSTextView *)textView {
  return textView;
}

- (IBAction)onGameMode:(id)sender {
    
}

- (IBAction)onDirectMode:(id)sender {
    
}

// NSWindowDelegate
- (void)windowDidBecomeKey:(NSNotification *)notification {
  if (!directMode) {
    [NSApp activateIgnoringOtherApps:YES];
  }
}

@end
