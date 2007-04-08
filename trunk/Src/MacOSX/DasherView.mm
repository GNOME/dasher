//
//  DasherView.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import "DasherView.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import "DasherUtil.h"

#import "DasherApp.h"
#import "PreferencesController.h"

#include <sys/time.h>

#import "ZippyCache.h"
#import "ZippyString.h"

#define MAX_CACHE_COUNT 500

static DasherView *XXXdasherView;



unsigned long int get_time() {

  long s_now;
  long ms_now;

  struct timeval tv;

  gettimeofday (&tv, NULL);

  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;

  return (s_now * 1000 + ms_now);
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
  [XXXdasherView rectangleCallbackX1:x1 y1:y1 x2:x2 y2:y2 colorNumber:Color colorScheme:ColorScheme];
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



static void registerCallbacks()
{
  dasher_set_blank_callback( blank_callback );
  dasher_set_display_callback( display_callback );
  dasher_set_draw_rectangle_callback( draw_rectangle_callback );
  dasher_set_draw_polyline_callback( draw_polyline_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_text_size_callback( text_size_callback );
}



@implementation DasherView

- (id)initWithFrame:(NSRect)frame {

  if (self = [super initWithFrame:frame]) {

    rectCache = (NSRect *)malloc(MAX_CACHE_COUNT * sizeof(NSRect));
    rectColorCache = (NSColor **)malloc(MAX_CACHE_COUNT * sizeof(NSColor *));
    rectCacheCount = 0;

    textCache = (ZippyString **)malloc(MAX_CACHE_COUNT * sizeof(ZippyString *));
    textPointCache = (NSPoint *)malloc(MAX_CACHE_COUNT * sizeof(NSPoint));
    textCacheCount = 0;

    [self setupColors];

    XXXdasherView = self;

    dasher_early_initialise();

    setlocale (LC_ALL, "");
  }

  return self;
}

- (void)setFrame:(NSRect)newRect
{
  [super setFrame:newRect];

  dasher_resize_canvas((int)newRect.size.width, (int)newRect.size.height);
  dasher_redraw();

  [self adjustTrackingRect];
}

- (void)adjustTrackingRect
{
  [self removeTrackingRect:trackingRectTag];
  trackingRectTag = [self addTrackingRect:[self frame] owner:self userData:nil assumeInside:[self isDashing]];
}

- (BOOL)isFlipped
{
  return YES;
}

- (BOOL)isOpaque
{
  return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
  return YES;
}

- (void)awakeFromNib
{
  registerCallbacks();
  
// TODO - these produce compiler errors...what do they mean? what do they do?
//    bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
//    textdomain (PACKAGE);


  NSString *systemDir = [NSString stringWithFormat:@"%@/", [[NSBundle mainBundle] resourcePath]];
  NSString *userDir = [NSString stringWithFormat:@"%@/Library/Application Support/Dasher/", NSHomeDirectory()];
  
    // system resources are inside the .app, under the Resources directory
  dasher_set_parameter_string( STRING_SYSTEMDIR, [systemDir cString]);
  dasher_set_parameter_string( STRING_USERDIR, [userDir cString]);

  dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
  dasher_set_parameter_int( INT_VIEW, 0 );

  const char *alphabet;

  dasher_get_alphabets( &alphabet, 1 );
  
//  choose_filename();

  dasher_late_initialise((int)[self bounds].size.width, (int)[self bounds].size.height);

  dasher_start();

  dasher_redraw();

  [self adjustTrackingRect];

  isPaused = NO;
}

- (BOOL)acceptsFirstResponder
{
  // TODO should the view should do something to indicate it has focus
  // is drawing a blue rectangle around the whole thing acceptable?
  return YES;
}

- (BOOL)becomeFirstResponder
{
  [[NSFontManager sharedFontManager] setSelectedFont:[NSFont fontWithName:[[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT] size:10.0] isMultiple:NO];

  return YES;
}

- (BOOL)resignFirstResponder
{
  return YES;
}


- (void)drawRect:(NSRect)rect {
  NSRect r = [self bounds];
  [[NSColor whiteColor] set];
  NSRectFill(r);

  [self drawRectCache];
  [self drawTextCache];

  [[NSColor blackColor] set];
  [[self polylineCache] stroke];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey:WINDOW_PAUSE] && ![self isDashing])
    {
    [self startDashing];
    }
}

- (void)mouseExited:(NSEvent *)theEvent
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey:WINDOW_PAUSE] && [self isDashing])
    {
    [self stopDashing];
    }
}

- (void)mouseDown:(NSEvent *)e
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey:START_MOUSE])
    {
    [self toggleDashing];
    }
}

- (void)keyDown:(NSEvent *)e
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey:START_SPACE] && [[e characters] isEqualToString:@" "])
    {
    [self toggleDashing];
    }
}

- (void)startDashing
{
  dasher_unpause(get_time());

  [self setTimer:[NSTimer scheduledTimerWithTimeInterval:1.0/20.0 target:self selector:@selector(timerCallback:) userInfo:nil repeats:YES]];
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



- (void)blankCallback
{
  [self clearRectCache];
  [self clearTextCache];
  [self clearPolylineCache];
}

- (void)displayCallback
{
  [self setNeedsDisplay:YES];
}



- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 colorNumber:(int)aColorNumber colorScheme:(Opts::ColorSchemes)aColorScheme
{
  int x = x1;
  int y = y1;
  int width = x2 - x1;
  int height = y2 - y1;
  NSRect r;

  if (x1 > x2) {
    x = x2;
    width = x1 - x2;
  }
  
  if (y1 > y2) {
    y = y2;
    height = y1 - y2;
  }

  r = NSMakeRect(x, y, width, height);
  [self addRect:r color:[self colorWithColorNumber:aColorNumber colorScheme:aColorScheme]];
}

- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize
{
  [self validateCacheWithFontName:[[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT]];

  return [[[self zippyCache] zippyStringWithString:aString size:aSize attributes:[self textAttributesWithTextSize:aSize]] size];
}

- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize
{
  [self validateCacheWithFontName:[[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT]];

  [self addText:[[self zippyCache] zippyStringWithString:aString size:aSize attributes:[self textAttributesWithTextSize:aSize]] point:NSMakePoint(x1, y1)];
}


- (void)polylineCallbackPoints:(NSArray *)points
{
  int len = [points count];
  int i;
  NSBezierPath *bp = [self polylineCache];

  if (len < 1)
    {
    return;
    }

  [bp moveToPoint:[[points objectAtIndex:0] pointValue]];

  for (i = 1; i < len; i++)
    {
    [bp lineToPoint:[[points objectAtIndex:i] pointValue]];
    }

  [bp closePath];
}

- (void)setupColors
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

  special1Color = [[NSColor colorWithCalibratedRed:c240 green:c240 blue:c240 alpha:alpha] retain];
  special2Color = [[NSColor colorWithCalibratedRed:c255 green:c255 blue:c255 alpha:alpha] retain];
  objectsColor = [[NSColor colorWithCalibratedRed:c0 green:c0 blue:c0 alpha:alpha] retain];
  groupsColor[0] = [[NSColor colorWithCalibratedRed:c255 green:c255 blue:c0 alpha:alpha] retain];
  groupsColor[1] = [[NSColor colorWithCalibratedRed:c255 green:c100 blue:c100 alpha:alpha] retain];
  groupsColor[2] = [[NSColor colorWithCalibratedRed:c0 green:c255 blue:c0 alpha:alpha] retain];
  nodes1Color[0] = [[NSColor colorWithCalibratedRed:c180 green:c245 blue:c180 alpha:alpha] retain];
  nodes1Color[1] = [[NSColor colorWithCalibratedRed:c160 green:c200 blue:c160 alpha:alpha] retain];
  nodes1Color[2] = [[NSColor colorWithCalibratedRed:c0 green:c255 blue:c255 alpha:alpha] retain];
  nodes2Color[0] = [[NSColor colorWithCalibratedRed:c255 green:c185 blue:c255 alpha:alpha] retain];
  nodes2Color[1] = [[NSColor colorWithCalibratedRed:c140 green:c200 blue:c255 alpha:alpha] retain];
  nodes2Color[2] = [[NSColor colorWithCalibratedRed:c255 green:c175 blue:c175 alpha:alpha] retain];
}

- (NSColor *)colorWithColorNumber:(int) aColor colorScheme:(Opts::ColorSchemes)aColorScheme
{
  if (aColorScheme == Special1) {
    return special1Color;
  }
  if (aColorScheme == Special2) {
    return special2Color;
  }
  if (aColorScheme == Objects) {
    return objectsColor;
  }
  if (aColorScheme == Groups) {
    return groupsColor[aColor % 3];
  }
  if (aColorScheme == Nodes1) {
    return nodes1Color[aColor % 3];
  }
  if (aColorScheme == Nodes2) {
    return nodes2Color[aColor % 3];
  }

  return nil;
}



- (IBAction)changeFont:(id)sender
{
  [[PreferencesController preferencesController] changeDasherFont:sender];
}

- (NSDictionary *)textAttributesWithTextSize:(int)aSize
{
  NSString *fontName = [[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT];

  NSString *cacheKey = [NSString stringWithFormat:@"%d", aSize];

  NSDictionary *textAttributes;

  textAttributes = [[self textAttributeCache] objectForKey:cacheKey];

  if (textAttributes == nil) {
    textAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSFont fontWithName:fontName size:(float)aSize], NSFontAttributeName,
      nil];

    [[self textAttributeCache] setObject:textAttributes forKey:cacheKey];
  }

  return textAttributes;
}

- (void)validateCacheWithFontName:(NSString *)fontName {
  if (![fontName isEqualToString:[self cachedFontName]]) {
    [self setTextAttributeCache:[NSMutableDictionary dictionaryWithCapacity:5]];
    [self setZippyCache:[ZippyCache zippyCache]];
    [self setCachedFontName:fontName];
  }
}

- (NSMutableDictionary *)textAttributeCache {
  return _textAttributeCache;
}

- (void)setTextAttributeCache:(NSMutableDictionary *)newTextAttributes {
  if (_textAttributeCache != newTextAttributes) {
    NSMutableDictionary *oldValue = _textAttributeCache;
    _textAttributeCache = [newTextAttributes retain];
    [oldValue release];
  }
}


- (void)clearTextCache {
  int i;
  for (i = 0; i < textCacheCount; i++) {
    [textCache[i] release];
  }

  textCacheCount = 0;
}

- (void)drawTextCache {
  if (textCacheCount == 0) {
    return;
  }
  
  // assumes focus is already LOCKED
  int i;
  for (i = 0; i < textCacheCount; i++) {
    [textCache[i] drawAtPoint:textPointCache[i]];
  }
}

- (void)addText:(ZippyString *)aZippyString point:(NSPoint)aPoint {
  if (textCacheCount >= MAX_CACHE_COUNT)
    {
    [NSException raise:@"TextCacheOverflow" format:@"Number of strings: %d exceeds limit: %d; increase MAX_CACHE_COUNT", textCacheCount + 1, MAX_CACHE_COUNT];
    return; // unreachable
    }

  textPointCache[textCacheCount] = aPoint;
  textCache[textCacheCount] = [aZippyString retain];
  textCacheCount++;
}



- (void)addRect:(NSRect)aRect color:(NSColor *)aColor
{
  if (rectCacheCount >= MAX_CACHE_COUNT)
    {
    [NSException raise:@"RectCacheOverflow" format:@"Number of rects: %d exceeds limit: %d; increase MAX_CACHE_COUNT", rectCacheCount + 1, MAX_CACHE_COUNT];
    return; // unreachable
    }

  rectCache[rectCacheCount] = aRect;
  rectColorCache[rectCacheCount] = [aColor retain];
  rectCacheCount++;
}

- (void)drawRectCache {
  if (rectCacheCount == 0) {
    return;
  }
  
  // assumes focus is already LOCKED
  NSRectFillListWithColors(rectCache, rectColorCache, rectCacheCount);
}

- (void)clearRectCache {
  int i;
  for (i = 0; i < rectCacheCount; i++) {
    [rectColorCache[i] release];
  }

  rectCacheCount = 0;
}

- (NSString *)cachedFontName {
  return _cachedFontName;
}

- (void)setCachedFontName:(NSString *)newCachedFontName {
  if (_cachedFontName != newCachedFontName) {
    NSString *oldValue = _cachedFontName;
    _cachedFontName = [newCachedFontName retain];
    [oldValue release];
  }
}

- (ZippyCache *)zippyCache {
  return _zippyCache;
}

- (void)setZippyCache:(ZippyCache *)newZippyCache {
  if (_zippyCache != newZippyCache) {
    ZippyCache *oldValue = _zippyCache;
    _zippyCache = [newZippyCache retain];
    [oldValue release];
  }
}

- (NSBezierPath *)polylineCache {
  return _polylineCache;
}

- (void)setPolylineCache:(NSBezierPath *)newPolylineCache {
  if (_polylineCache != newPolylineCache) {
    NSBezierPath *oldValue = _polylineCache;
    _polylineCache = [newPolylineCache retain];
    [oldValue release];
  }
}

- (void)clearPolylineCache {
  [self setPolylineCache:[NSBezierPath bezierPath]];
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

- (void)timerCallback:(NSTimer *)aTimer
{
  NSPoint p = [[self window] mouseLocationOutsideOfEventStream];
  NSPoint q = [self convertPoint:p fromView:nil];

  dasher_tap_on( (int)q.x, (int)q.y, get_time() );
}

- (void)dealloc
{
  [_timer invalidate];
  [_timer release];

  [_cachedFontName release];
  [_zippyCache release];
  [_textAttributeCache release];

  [_polylineCache release];

  [self clearRectCache];
  free(rectCache);
  free(rectColorCache);

  [self clearTextCache];
  free(textCache);
  free(textPointCache);

  [special1Color release];
  [special2Color release];
  [objectsColor release];

  for (int i = 0; i < 3; i++) {
    [groupsColor[i] release];
    [nodes1Color[i] release];
    [nodes2Color[i] release];
  }
  

  [super dealloc];
}

@end
