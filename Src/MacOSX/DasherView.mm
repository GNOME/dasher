/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 20 April 2003
 */

// define TOY for testing this view outisde the context of the dashser core and Dasher.app
// just comment the line out for using inside Dasher

#import "DasherView.h"
#import <AppKit/AppKit.h>

#if !defined(TOY)
#import "DasherApp.h"
#import "PreferencesController.h"

#include "libdasher.h"
#endif

#import "ZippyCache.h"
#import "ZippyString.h"
 
#define MAX_CACHE_COUNT 300

@implementation DasherView

- (id)initWithFrame:(NSRect)frame {

  if (self = [super initWithFrame:frame]) {

    rectCache = (NSRect *)malloc(MAX_CACHE_COUNT * sizeof(NSRect));
    rectColorCache = (NSColor **)malloc(MAX_CACHE_COUNT * sizeof(NSColor *));
    rectCacheCount = 0;

    textCache = (ZippyString **)malloc(MAX_CACHE_COUNT * sizeof(ZippyString *));
    textPointCache = (NSPoint *)malloc(MAX_CACHE_COUNT * sizeof(NSPoint));
    textCacheCount = 0;

    isPaused = NO;

#if defined(TOY)
//    fontName = [[[NSFont userFontOfSize:10.0] fontName] retain];
    fontName = [@"Monaco" retain];
#endif
    
  }
  
  return self;
}

- (void)setFrame:(NSRect)newRect
{
  [super setFrame:newRect];

#if !defined(TOY)
  dasher_resize_canvas((int)newRect.size.width, (int)newRect.size.height);
  dasher_redraw();
#endif
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

- (BOOL)acceptsFirstResponder
{
  // TODO should the view should do something to indicate it has focus
  // is drawing a blue rectangle around the whole thing acceptable?
  return YES;
}

- (BOOL)becomeFirstResponder
{
#if !defined(TOY)
  [[NSFontManager sharedFontManager] setSelectedFont:[NSFont fontWithName:[[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT] size:10.0] isMultiple:NO];
#endif

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

- (void)mouseDown:(NSEvent *)e
{
#if !defined(TOY)
  if ([[NSUserDefaults standardUserDefaults] boolForKey:START_MOUSE])
    {
    [[[NSApplication sharedApplication] delegate] toggleDashing];
    }
#endif
}

- (void)keyDown:(NSEvent *)e
{
#if !defined(TOY)
  if ([[NSUserDefaults standardUserDefaults] boolForKey:START_SPACE] && [[e characters] isEqualToString:@" "])
    {
    [[[NSApplication sharedApplication] delegate] toggleDashing];
    }
#endif
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



- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 color:(NSColor *)aColor
{
  int x = x1;
  int y = y1;
  int width = x2 - x1;
  int height = y2 - y1;
  NSRect r;

  if (x1 > x2)
    {
    x = x2;
    width = x1 - x2;
    }
  if (y1 > y2)
    {
    y = y2;
    height = y1 - y2;
    }

  r = NSMakeRect(x1, y1, width, height);
  [self addRect:r color:aColor];
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

- (IBAction)changeFont:(id)sender
{
#if !defined(TOY)
  [[PreferencesController preferencesController] changeDasherFont:sender];
#endif
}

- (NSDictionary *)textAttributesWithTextSize:(int)aSize
{
#if !defined(TOY)
  NSString *fontName = [[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT];
#endif
  
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

- (NSDictionary *)textAttributeCache {
  return _textAttributeCache;
}

- (void)setTextAttributeCache:(NSMutableDictionary *)newTextAttributes {
  if (_textAttributeCache != newTextAttributes) {
    NSDictionary *oldValue = _textAttributeCache;
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

- (void)dealloc
{
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
  
  [super dealloc];
}

@end
