//
//  DasherViewAqua.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherViewAqua.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import "DasherApp.h"
#import "PreferencesController.h"

#import <sys/time.h>

#import "ZippyCache.h"
#import "ZippyString.h"
#import "Chatter.h"
#import "DasherUtil.h"
#import "DasherApp.h"

#import "COSXDasherScreen.h"

#define MAX_CACHE_COUNT 1000

@implementation DasherViewAqua

/*
 BEWARE!
 This is doing funny stuff with lockFocus and two buffers in order to make drawing as swift as possible.  The buffer handling is in sendMarker:
 */

- (void)sendMarker:(int)iMarker {

  // TODO this lock/unlock technique doesn't work; if they go to prefs and fiddle around, they get:
  // Unlocking Focus on wrong view (<NSTableView: 0x74ed880>), expected <NSImageCacheView: 0x74da1e0>
  // would a lock/unlock on enter/exit work?
  
//  [currentBuffer unlockFocus];
  
//  if (iMarker == 0) {
//    currentBuffer = [self boxesBuffer];
//  } else if (iMarker == 1) {
//    currentBuffer = [self mouseBuffer];
//    [currentBuffer lockFocus];
//    [[self boxesBuffer] compositeToPoint:NSMakePoint(0.0, [self bounds].size.height) operation:NSCompositeCopy];
//    [currentBuffer unlockFocus];
//  }
//  [currentBuffer lockFocus];
}

- (void)displayCallback
{
//E  [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect {
  if (![dasherApp aquaDasherControl]->GetBoolParameter(BP_DASHER_PAUSED)) {
    
    [dasherApp aquaDasherControl]->goddamn(get_time(), false); //E
  }
  
//  NSPoint p = NSMakePoint(0.0, [self bounds].size.height);
//  [[self mouseBuffer] compositeToPoint:p operation:NSCompositeCopy];
//  [[self boxesBuffer] compositeToPoint:p operation:NSCompositeCopy];
//  [[self mouseBuffer] drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
//  [[self boxesBuffer] drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
  
//  [[self mouseBuffer] draw];
//  [[self boxesBuffer] draw];
}



- (void)mouseEntered:(NSEvent *)theEvent
{
}

- (void)mouseExited:(NSEvent *)theEvent
{
}

- (void)mouseDown:(NSEvent *)e
{
  NSPoint q = [self convertPoint:[e locationInWindow] fromView:nil];
    
  [dasherApp aquaDasherControl]->HandleClickDown(get_time(), q.x, q.y);
}

- (void)mouseUp:(NSEvent *)e
{
  NSPoint q = [self convertPoint:[e locationInWindow] fromView:nil];
  
  [dasherApp aquaDasherControl]->HandleClickUp(get_time(), q.x, q.y);
}


- (void)keyDown:(NSEvent *)e
{
}

- (void)circleCallbackCentrePoint:(NSPoint)aCentrePoint radius:(float)aRadius outlineColorIndex:(int)anOutlineColorIndex fillColourIndex:(int)aFillColourIndex shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth {
  
  NSBezierPath *bp = [NSBezierPath bezierPathWithOvalInRect:NSMakeRect(aCentrePoint.x - aRadius, aCentrePoint.y - aRadius, 2.0 * aRadius, 2.0 * aRadius)];
  
  [[self colorWithColorIndex:anOutlineColorIndex == -1 ? 3 : anOutlineColorIndex] set];
  
  [NSBezierPath setDefaultLineWidth:aLineWidth];
  [bp stroke];
  
  if (shouldFill) {
    [[self colorWithColorIndex:aFillColourIndex] set];
    [bp fill];
  }
}


- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex lineWidth:(int)aLineWidth {

  float x, y, width, height;
  
  if( x2 > x1 ) {
    x = x1;
    width = x2 - x1;
  }
  else {
    x = x2;
    width = x1 - x2;
  }
  
  if( y2 > y1 ) {
    y = y1;
    height = y2 - y1;
  }
  else {
    y = y2;
    height = y1 - y2;
  }
  
  NSRect r = NSMakeRect(x, y, width, height);
  
  if (aFillColorIndex!=-1) {
    [[self colorWithColorIndex:aFillColorIndex] set];
    [NSBezierPath fillRect:r];
  }
  
  if (aLineWidth>0) {
    [[self colorWithColorIndex:anOutlineColorIndex == -1 ? 3 : anOutlineColorIndex] set];
    
    [NSBezierPath setDefaultLineWidth:aLineWidth];
    // gtk also sets these: GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND
    [NSBezierPath strokeRect:r];
  }
}

- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize colorIndex:(int)aColorIndex
{

  return [[[self zippyCache] zippyStringWithString:aString size:aSize attributes:[self textAttributesWithTextSize:aSize color:[self colorWithColorIndex:aColorIndex]]] size];
}

- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex
{

  [[[self zippyCache] zippyStringWithString:aString size:aSize attributes:[self textAttributesWithTextSize:aSize color:[self colorWithColorIndex:aColorIndex]]] drawAtPoint:NSMakePoint(x1, y1 /*+ [self textSizeCallbackWithString:aString size:aSize colorIndex:aColorIndex].height / 2.0*/)];
}


- (void)polylineCallbackPoints:(NSArray *)points width:(int)aWidth colorIndex:(int)aColorIndex
{
  int len = [points count];
  int i;
  NSBezierPath *bp = [NSBezierPath bezierPath];

  if (len < 1)
    {
    return;
    }

  [bp moveToPoint:[[points objectAtIndex:0] pointValue]];

  for (i = 1; i < len; i++)
    {
    [bp lineToPoint:[[points objectAtIndex:i] pointValue]];
    }
  
  [[self colorWithColorIndex:aColorIndex] set];
  [bp setLineWidth:aWidth];
  [bp stroke];
}

- (void)colourSchemeCallbackWithColours:(NSArray *)colours
{
  [self setColourScheme:colours];
  [[self zippyCache] flush];
}
  
- (NSColor *)colorWithColorIndex:(int) aColor
{
  return [[self colourScheme] objectAtIndex:aColor];
}


- (NSDictionary *)textAttributesWithTextSize:(int)aSize color:(NSColor *)aColor
{
  NSString *fontName = [self cachedFontName];

  NSString *cacheKey = [NSString stringWithFormat:@"%d", aSize];

  NSDictionary *textAttributes;

  textAttributes = [[self textAttributeCache] objectForKey:cacheKey];

  if (textAttributes == nil) {
#if defined(ZIPPY_STRING_GLYPH)
    // flipped matrix
    float fm[] = {aSize, 0.0, 0.0, -aSize, 0.0, 0.0};
#else
    // normal matrix
    float fm[] = {aSize, 0.0, 0.0, aSize, 0.0, 0.0};
#endif
    textAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSFont fontWithName:fontName matrix:fm], NSFontAttributeName,
      //[NSFont fontWithName:fontName size:(float)aSize], NSFontAttributeName,
      aColor, NSForegroundColorAttributeName,
      nil];

    [[self textAttributeCache] setObject:textAttributes forKey:cacheKey];
  }

  return textAttributes;
}

- (void)flushFontCache {
  [self setTextAttributeCache:[NSMutableDictionary dictionaryWithCapacity:5]];
  [self setZippyCache:[ZippyCache zippyCache]];
  [self setCachedFontName:[[NSUserDefaults standardUserDefaults] objectForKey:@"DasherFont"]];
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

- (id)initWithFrame:(NSRect)aFrame {
  
  if (self = [super initWithFrame:aFrame]) {
    [self flushFontCache];
    [self setFrameSize:aFrame.size];
    [self setupFrame:aFrame];
  }
  
  return self;
}

- (void)userDefaultsDidChange:(NSNotification *)aNote {
  if (![[[NSUserDefaults standardUserDefaults] objectForKey:@"DasherFont"] isEqualToString:[self cachedFontName]]) {
    [self flushFontCache];
    [dasherApp aquaDasherControl]->ScheduleRedraw();
  }
}

- (void)setupCache {
  NSImage *b = [[NSImage alloc] initWithSize:[self bounds].size];
  NSImage *m = [[NSImage alloc] initWithSize:[self bounds].size];
  [self setBoxesBuffer:b];
  [[self boxesBuffer] setFlipped:YES];
  [self setMouseBuffer:m];
  [[self mouseBuffer] setFlipped:YES];
  [b release];
  [m release];
  
//  NSImage *b = [[NSCachedImageRep alloc] initWithSize:[self bounds].size depth:0 separate:NO alpha:NO];
//  NSImage *m = [[NSCachedImageRep alloc] initWithSize:[self bounds].size depth:0 separate:NO alpha:NO];
//  [self setBoxesBuffer:b];
//  [[self boxesBuffer] setFlipped:YES];
//  [self setMouseBuffer:m];
//  [[self mouseBuffer] setFlipped:YES];
//  [b release];
//  [m release];
}

- (void)setupFrame:(NSRect)newRect {
  delete( aquaDasherScreen );
  aquaDasherScreen = new COSXDasherScreen(self);
  [dasherApp changeScreen:aquaDasherScreen];
  [self setupCache];
  
  [self adjustTrackingRect];
  
  [self flushFontCache];
}

- (void)setFrame:(NSRect)newRect
{
  // TODO is this the right method to override, or resizeWithOldSuperviewSize: ?
  [super setFrame:newRect];
  [self setupFrame:newRect];
  
}

- (void)adjustTrackingRect
{
// need this in order to get mouseEntered and Exited
//  [self removeTrackingRect:trackingRectTag];
//  trackingRectTag = [self addTrackingRect:[self frame] owner:self userData:nil assumeInside:[self isDashing]];
}

- (BOOL)isFlipped
{
  return YES;
//  return NO;
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
//  aquaDasherScreen = new COSXDasherScreen(self);
  [dasherApp setDasherView:self];

  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(userDefaultsDidChange:) name:NSUserDefaultsDidChangeNotification object:nil];
}

- (void)finishRealization {
  [dasherApp changeScreen:aquaDasherScreen];
  
}

- (COSXDasherScreen *)aquaDasherScreen {
  return aquaDasherScreen;
}

- (BOOL)acceptsFirstResponder
{
  return NO;
}


- (NSArray *)colourScheme {
  return [[_colourScheme retain] autorelease];
}

- (void)setColourScheme:(NSArray *)newColourScheme {
  if (_colourScheme != newColourScheme) {
    [_colourScheme release];
    _colourScheme = [newColourScheme retain];
  }
}

- (void)setColourSchemeFromColourInfo:(const CColourIO::ColourInfo *)pColourScheme {
  
  int iNumColours(pColourScheme->Reds.size());
  NSMutableArray *colours = [NSMutableArray arrayWithCapacity:iNumColours];

#define c(__x) (__x * 257.0) / 65535.0
    
  for(int i = 0; i < iNumColours; i++) {
    [colours addObject:[NSColor colorWithCalibratedRed:c(pColourScheme->Reds[i]) green:c(pColourScheme->Greens[i]) blue:c(pColourScheme->Blues[i]) alpha:1.0]];
  }
  
  [self colourSchemeCallbackWithColours:(NSArray *)colours];
}


- (NSImage *)boxesBuffer {
  return [[_boxesBuffer retain] autorelease];
}

- (void)setBoxesBuffer:(NSImage *)value {
  if (_boxesBuffer != value) {
    [_boxesBuffer release];
    _boxesBuffer = [value retain];
  }
}

- (NSImage *)mouseBuffer {
  return [[_mouseBuffer retain] autorelease];
}

- (void)setMouseBuffer:(NSImage *)value {
  if (_mouseBuffer != value) {
    [_mouseBuffer release];
    _mouseBuffer = [value retain];
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

- (NSPoint)mouseLocation {
  NSPoint p = [[self window] mouseLocationOutsideOfEventStream];
  return [self convertPoint:p fromView:nil];
}

- (float)boundsWidth {
  return [self bounds].size.width;
}

- (float)boundsHeight {
  return [self bounds].size.height;
}

- (void)redisplay {
  [self setNeedsDisplay:YES];
}


- (void)dealloc
{
  [_cachedFontName release];
  [_zippyCache release];
  [_textAttributeCache release];
  
  [_colourScheme release];
  
  [_boxesBuffer release];
  [_mouseBuffer release];
  
  [super dealloc];
}


@end
