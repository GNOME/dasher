//
//  DasherApp.m
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "DasherApp.h"
#import "DasherView.h"
#import "PreferencesController.h"
#include "libdasher.h"

/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 18 April 2003
 */

#import <Cocoa/Cocoa.h>
#include <sys/time.h>

// TODO, figure out best way to get into obj-c instance from c function
DasherView *XXXdasherView;
DasherApp *XXXdasherApp;


NSColor *getColor(int aColor, Opts::ColorSchemes aColorScheme)
{
#define c(__x) (__x * 257.0) / 65535.0

  static float c0 = 0.0;
  static float c100 = c(100);
  static float c140 = c(140);
  static float c160 = c(160);
  static float c175 = c(175);
  static float c185 = c(185);
  static float c180 = c(180);
  static float c200 = c(200);
  static float c240 = c(240);
  static float c245 = c(245);
  static float c255 = c(255);
  static float alpha = 1.0;

  if (aColorScheme == Special1) {
    return [NSColor colorWithCalibratedRed:c240 green:c240 blue:c240 alpha:alpha];
  }
  if (aColorScheme == Special2) {
    return [NSColor colorWithCalibratedRed:c255 green:c255 blue:c255 alpha:alpha];
  }
  if (aColorScheme == Objects) {
    return [NSColor colorWithCalibratedRed:c0 green:c0 blue:c0 alpha:alpha];
  }
  if (aColorScheme == Groups) {
    if (aColor%3 == 0) {
      return [NSColor colorWithCalibratedRed:c255 green:c255 blue:c0 alpha:alpha];
    }
    if (aColor%3 == 1) {
      return [NSColor colorWithCalibratedRed:c255 green:c100 blue:c100 alpha:alpha];
    }
    if (aColor %3 == 2) {
      return [NSColor colorWithCalibratedRed:c0 green:c255 blue:c0 alpha:alpha];
    }
  }
  if (aColorScheme == Nodes1) {
    if (aColor%3 == 0) {
      return [NSColor colorWithCalibratedRed:c180 green:c245 blue:c180 alpha:alpha];
    }
    if (aColor%3 == 1) {
      return [NSColor colorWithCalibratedRed:c160 green:c200 blue:c160 alpha:alpha];
    }
    if (aColor %3 == 2) {
      return [NSColor colorWithCalibratedRed:c0 green:c255 blue:c255 alpha:alpha];
    }
  }
  if (aColorScheme == Nodes2) {
    if (aColor%3 == 0) {
      return [NSColor colorWithCalibratedRed:c255 green:c185 blue:c255 alpha:alpha];
    }
    if (aColor%3 == 1) {
      return [NSColor colorWithCalibratedRed:c140 green:c200 blue:c255 alpha:alpha];
    }
    if (aColor %3 == 2) {
      return [NSColor colorWithCalibratedRed:c255 green:c175 blue:c175 alpha:alpha];
    }
  }
}

NSString *NSStringFromStdString(const std::string& aString)
{
  // inside DasherCore, std::string is used as a container for a string of UTF-8 bytes, so
  // we can just get the raw bytes (with c_str()) and shove them into an NSString.
  return [NSString stringWithUTF8String:aString.c_str()];
}

void blank_callback()
{
  [XXXdasherView blankCallback];
}


void display_callback()
{
  [XXXdasherView displayCallback];
}

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  [XXXdasherView rectangleCallbackX1:x1 y1:y1 x2:x2 y2:y2 color:getColor(Color, ColorScheme)];
}

void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number)
{
  NSMutableArray *p = [NSMutableArray arrayWithCapacity:Number];
  int i;
  
  for (i = 0; i < Number; i++) {
    [p addObject:[NSValue valueWithPoint:NSMakePoint(Points[i].x, Points[i].y)]];
  }

  [XXXdasherView polylineCallbackPoints:p];
}

void draw_text_callback(symbol Character, int x1, int y1, int size)
{
  [XXXdasherView drawTextCallbackWithString:NSStringFromStdString(dasher_get_display_text(Character)) x1:x1 y1:y1 size:size];
}

void text_size_callback(symbol Character, int* Width, int* Height, int Size)
{
  NSSize s = [XXXdasherView textSizeCallbackWithString:NSStringFromStdString(dasher_get_display_text(Character)) size:Size];

  *Width = (int)s.width;
  *Height = (int)s.height;
}


void edit_output_callback(symbol Symbol)
{
  [XXXdasherApp outputCallback:NSStringFromStdString(dasher_get_edit_text(Symbol))];
}

// TODO this flush stuff never gets called?  and the context only gets called at the very start!
void edit_flush_callback(symbol Symbol)
{
  [XXXdasherApp flushCallback:NSStringFromStdString(dasher_get_edit_text(Symbol))];
}

void edit_unflush_callback()
{
  [XXXdasherApp unflushCallback];
}

void get_new_context_callback( std::string &str, int max )
{
  NSString *s = [XXXdasherApp getNewContextCallback:max];
  str = (s == nil || [s length] == 0) ? new std::string() : new std::string([s cString]);
}

void clipboard_callback( clipboard_action act )
{
  [XXXdasherApp clipboardCallbackWithAction:(clipboard_action)act];
}


static void registerCallbacks()
{

  dasher_set_blank_callback( blank_callback );
  dasher_set_display_callback( display_callback );
  dasher_set_draw_rectangle_callback( draw_rectangle_callback );
  dasher_set_draw_polyline_callback( draw_polyline_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_text_size_callback( text_size_callback );

  dasher_set_edit_output_callback( edit_output_callback );
  dasher_set_edit_flush_callback( edit_flush_callback );
  dasher_set_edit_unflush_callback( edit_unflush_callback );
  
  dasher_set_get_new_context_callback( get_new_context_callback );

  dasher_set_clipboard_callback( clipboard_callback );
  
}


// TODO make another file (c? c++?) of utilities used by both this and the view
unsigned long int get_time() {

  long s_now;
  long ms_now;

  struct timeval tv;
 
  gettimeofday (&tv, NULL);

  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;

  return (s_now * 1000 + ms_now);
}


@implementation DasherApp

- (id)init
{
  if (self = [super init])
    {
    dasher_early_initialise();
    registerCallbacks();
    
  // TODO prefcont should have a method like setup or something to call, not just the shared instance
    [PreferencesController preferencesController];
    
    setlocale (LC_ALL, "");
    
// TODO - these produce compiler errors...what do they mean? what do they do?
//    bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
//    textdomain (PACKAGE);


    NSString *systemDir = [NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] resourcePath]];
    NSString *userDir = [NSString stringWithFormat:@"%@/.dasher/", NSHomeDirectory()];

    // system resources are inside the .app, under the Resources directory
    dasher_set_parameter_string( STRING_SYSTEMDIR, [systemDir cString]);
    dasher_set_parameter_string( STRING_USERDIR, [userDir cString]);

    dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
    dasher_set_parameter_int( INT_VIEW, 0 );

    const char *alphabet;

    dasher_get_alphabets( &alphabet, 1 );
    
//  choose_filename();

    dasher_late_initialise(398, 201);  // values must be coordinated with the DasherView size in the nib

    dasher_start();
    }

  return self;
}

- (void)awakeFromNib
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  
    // TODO megaNONO, what's the best way to integrate callbacks with instances?
  XXXdasherView = dasherView;
  XXXdasherApp = self;
  
  // TODO this should actually happen on creating a new document
  [dasherEdit setFont:[NSFont fontWithName:[defaults stringForKey:EDIT_FONT] size:(float)[defaults integerForKey:EDIT_FONT_SIZE]]];

  dasher_redraw();
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
  flushCount = 0;

}

- (void)startDashing
{
  dasher_unpause(get_time());

  [self setTimer:[NSTimer scheduledTimerWithTimeInterval:1.0/17.0 target:self selector:@selector(timerCallback:) userInfo:nil repeats:YES]];
}

- (void)stopDashing
{
  dasher_pause(0, 0);
  [[self timer] invalidate];
  [self setTimer:nil];
}

- (BOOL)isDashing
{
  return [self timer] != nil;
}

- (void)toggleDashing
{
  [self isDashing] ? [self stopDashing] : [self startDashing];  
}

- (void)timerCallback:(NSTimer *)aTimer
{
  NSPoint p = [dasherWindow mouseLocationOutsideOfEventStream];
  NSPoint q = [dasherView convertPoint:p fromView:nil];

  dasher_tap_on( (int)q.x, (int)q.y, get_time() );
}


- (void)outputCallback:(NSString *)aString
{
  [dasherEdit insertText:aString];
}

- (void)flushCallback:(NSString *)aString
{
  [dasherEdit insertText:aString];

  if (![aString isEqualToString:@""])
    {
    flushCount++;
    }

}

- (void)unflushCallback
{
  NSString *s = [dasherEdit string];
  int numChar = [s length] - flushCount;
  
  if (numChar <= 0)
    {
    numChar = [s length];
    }
  [dasherEdit setString:[s substringToIndex:numChar]];
  
  flushCount = 0;
}

- (NSString *)getNewContextCallback:(int)maxChars
{
  NSString *s = [dasherEdit string];
  int i = [s length] - maxChars;
  if (i < 0)
    {
    i = 0;
    }
  return [s substringFromIndex:i];
}

- clipboardCallbackWithAction:(clipboard_action)act
{
  switch( act )
    {
    case CLIPBOARD_CUT:
      [dasherEdit cut:self];
      break;
    case CLIPBOARD_COPY:
      [dasherEdit copy:self];
      break;
    case CLIPBOARD_PASTE:
      [dasherEdit paste:self];
      break;
    case CLIPBOARD_COPYALL:
      [dasherEdit selectAll:self];
      [dasherEdit copy:self];
      break;
    case CLIPBOARD_SELECTALL:
      [dasherEdit selectAll:self];
      break;
    case CLIPBOARD_CLEAR:
      [dasherEdit setString:nil];
      break;
    }
}



- (void)trainingTextOpenPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{
  if (returnCode == NSOKButton)
    {
    dasher_train_file([[sheet filename] cString]);
    }
}

- (IBAction)importTrainingText:(id)sender
{
  [[NSOpenPanel openPanel] beginSheetForDirectory:nil file:nil types:nil modalForWindow:dasherWindow modalDelegate:self didEndSelector:@selector(trainingTextOpenPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (IBAction)showPreferences:(id)sender
{
  [[PreferencesController preferencesController] makeKeyAndOrderFront:sender];
}

- (NSTimer *)timer {
  return _timer;
}

- (void)setTimer:(NSTimer *)newTimer {
  if (_timer != newTimer) {
    NSTimer *oldValue = _timer;
    _timer = [newTimer retain];
    [oldValue release];
  }
}

@end
