//
//  PreferencesController.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define ALTER_FONT_DASHER 1
#define ALTER_FONT_EDIT 2

@interface PreferencesController : NSObject
{
  IBOutlet NSPanel *panel;

  IBOutlet NSButtonCell *drawMouseUI;
  IBOutlet NSButtonCell *dasherDimensionsUI;
  IBOutlet NSMatrix *screenOrientationUI;
  IBOutlet NSSlider *maxBitRateUI;

  IBOutlet NSBrowser *alphabetIDUI;
  IBOutlet NSButtonCell *copyAllOnStopUI;
  IBOutlet NSTextField *dasherFontUI;
  IBOutlet NSTextField *editFontUI;
  IBOutlet NSMatrix *fileEncodingUI;
  IBOutlet NSMatrix *dasherFontSizeUI;
  IBOutlet NSButtonCell *keyControlUI;
  IBOutlet NSButtonCell *startMouseUI;
  IBOutlet NSButtonCell *startSpaceUI;
  IBOutlet NSButtonCell *windowPauseUI;

  IBOutlet NSSlider *dasherPanelOpacityUI;

  NSArray *_alphabetList;
  int fontToAlter;
}



+ (id)preferencesController;
- (void)show;

- (IBAction)drawMouseAction:(id)sender;
- (IBAction)dasherDimensionsAction:(id)sender;
- (IBAction)startMouseAction:(id)sender;
- (IBAction)startSpaceAction:(id)sender;
- (IBAction)windowPauseAction:(id)sender;
- (IBAction)keyControlAction:(id)sender;
- (IBAction)copyAllOnStopAction:(id)sender;

- (IBAction)maxBitRateAction:(id)sender;
- (IBAction)screenOrientationAction:(id)sender;

- (IBAction)alphabetIDAction:(id)sender;

- (IBAction)dasherFontSizeAction:(id)sender;

- (IBAction)dasherFontAction:(id)sender;

- (IBAction)editFontAction:(id)sender;

- (IBAction)dasherPanelOpacityAction:(id)sender;

- (NSArray *)alphabetList;
- (void)setAlphabetList:(NSArray *)newAlphabetList;

@end

// Mac OS X only parameters
#define DASHER_PANEL_OPACITY @"DasherPanelOpacity"


// TODO figure out how to use these strings from Dasher::Keys:: rather than repeating
// the definitions here

	// bool options
#define COPY_ALL_ON_STOP @"CopyAllOnStop"
#define DRAW_MOUSE @"DrawMouse"
#define START_MOUSE @"StartOnLeft"
#define START_SPACE @"StartOnSpace"
#define KEY_CONTROL @"KeyControl"
#define WINDOW_PAUSE @"PauseOutsideWindow"
#define TIME_STAMP @"TimeStampNewFiles"
#define FIX_LAYOUT @"FixLayout"
#define SHOW_TOOLBAR @"ViewToolbar"
#define SHOW_TOOLBAR_TEXT @"ShowToolbarText"
#define SHOW_LARGE_ICONS @"ShowLargeIcons"
#define SHOW_SLIDER @"ShowSpeedSlider"

	// long options
#define SCREEN_ORIENTATION @"ScreenOrientation"
#define VIEW_ID @"ViewID"
#define LANGUAGE_MODEL_ID @"LanguageModelID"
#define EDIT_FONT_SIZE @"EditFontSize"
#define EDIT_HEIGHT @"EditHeight"
#define SCREEN_WIDTH @"ScreenWidth"
#define SCREEN_HEIGHT @"ScreenHeight"
#define DASHER_FONTSIZE @"DasherFontSize"
#define FILE_ENCODING @"FileEncodingFormat"
#define MAX_BITRATE_TIMES100 @"MaxBitRateTimes100"
#define DASHER_DIMENSIONS @"NumberDimensions"

	// string options
#define ALPHABET_ID @"AlphabetID"
#define DASHER_FONT @"DasherFont"
#define EDIT_FONT @"EditFont"

