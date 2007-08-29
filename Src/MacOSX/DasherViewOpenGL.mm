//
//  DasherViewOpenGL.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherViewOpenGL.h"
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

@implementation DasherViewOpenGL

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

- (void)blankCallback
{
  [[self openGLContext] makeCurrentContext];
  glClear(GL_COLOR_BUFFER_BIT);
}

- (void)displayCallback
{
//E  [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect {
  [[self openGLContext] makeCurrentContext];
  if (![dasherApp aquaDasherControl]->GetBoolParameter(BP_DASHER_PAUSED)) {
   [self blankCallback];
    
    glLoadIdentity();
    [dasherApp aquaDasherControl]->goddamn(get_time(), false); //E
    
//  [[self openGLContext] flushBuffer];
  }
  
  glFlush();
  
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
  
//  NSBezierPath *bp = [NSBezierPath bezierPathWithOvalInRect:NSMakeRect(aCentrePoint.x - aRadius, aCentrePoint.y - aRadius, 2.0 * aRadius, 2.0 * aRadius)];
//  
//  int oci = anOutlineColorIndex == -1 ? 3 : anOutlineColorIndex;
//  glColor3f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b);
//  
//  [NSBezierPath setDefaultLineWidth:aLineWidth];
//  [bp stroke];
//  
//  if (shouldFill) {
//    [[self colorWithColorIndex:aFillColourIndex] set];
//    [bp fill];
//  }
}


- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex shouldOutline:(BOOL)shouldOutline shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth {
  
  // don't know if this is needed with opengl...does it cope with wonky coords?
  //  float x, y, width, height;
  //  
  //  if( x2 > x1 ) {
  //    x = x1;
  //    width = x2 - x1;
  //  }
  //  else {
  //    x = x2;
  //    width = x1 - x2;
  //  }
  //  
  //  if( y2 > y1 ) {
  //    y = y1;
  //    height = y2 - y1;
  //  }
  //  else {
  //    y = y2;
  //    height = y1 - y2;
  //  }
  
  if (shouldFill) {
    glColor3f(colourTable[aFillColorIndex].r, colourTable[aFillColorIndex].g, colourTable[aFillColorIndex].b);
    glRecti(x1, y1, x2, y2);
  }
  
  glDisable(GL_TEXTURE_2D);
  if (shouldOutline) {
    int oci = anOutlineColorIndex == -1 ? 3 : anOutlineColorIndex;
    glColor3f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b);
    glBegin(GL_LINE_LOOP);
    glVertex2i (x1, y1);
    glVertex2i (x1, y2);
    glVertex2i (x2, y2);
    glVertex2i (x2, y1);    
    glEnd();
  }
  
  
}


- (AlphabetLetter *)letterForString:(NSString *)aString {
  AlphabetLetter *result = [_letterDict objectForKey:aString];
  if (result == nil) {
    result = [[AlphabetLetter alloc] initWithString:aString small:YES];
    [_letterDict setObject:result forKey:aString];
  }
  return result;
}

- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize colorIndex:(int)aColorIndex
{
  return [[self letterForString:aString] sizeWithSize:aSize];
}


- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex
{
  AlphabetLetter *letter = [self letterForString:aString];
  
  glEnable(GL_TEXTURE_2D);
  // TODO could pass the whole colour_t in and let it deal with splitting out the items
  [letter drawWithSize:/*1.0*/ aSize / 18.0 x:x1 y:y1 r:colourTable[aColorIndex].r g:colourTable[aColorIndex].g b:colourTable[aColorIndex].b];
}

- (void)colourSchemeCallbackWithColourTable:(colour_t *)aColourTable {
  
  if (colourTable != NULL) {
    free(colourTable);
  }
  
  colourTable = aColourTable;
}


- (void)polylineCallbackPoints:(NSArray *)points width:(int)aWidth colorIndex:(int)aColorIndex
{
//  int len = [points count];
//  int i;
//  NSBezierPath *bp = [NSBezierPath bezierPath];
//
//  if (len < 1)
//    {
//    return;
//    }
//
//  [bp moveToPoint:[[points objectAtIndex:0] pointValue]];
//
//  for (i = 1; i < len; i++)
//    {
//    [bp lineToPoint:[[points objectAtIndex:i] pointValue]];
//    }
//  
//  [[self colorWithColorIndex:aColorIndex] set];
//  [bp setLineWidth:aWidth];
//  [bp stroke];
}



- (id)initWithFrame:(NSRect)aFrame {
  
  NSOpenGLPixelFormatAttribute attrs[] =
  {
    NSOpenGLPFADoubleBuffer,
//    NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)32,
    (NSOpenGLPixelFormatAttribute)0
  };
  
  NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  
  /* Check if initWithAttributes succeeded. */
//  if(pixFmt == nil) {
    pixFmt = [[self class] defaultPixelFormat];
//  }
  
  if (self = [super initWithFrame:aFrame pixelFormat:pixFmt]) {
        // init open gl
    [self gl_init];
    [self flushCaches];
    [self setFrameSize:aFrame.size];
    
    [self blankCallback];  
  }
  
  return self;
}

- (void)userDefaultsDidChange:(NSNotification *)aNote {
  [self flushCaches];
  [dasherApp aquaDasherControl]->ScheduleRedraw();
}

- (void)flushCaches {
  [_letterDict release];
  _letterDict = [[NSMutableDictionary alloc] init];
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

  [self adjustTrackingRect];
  
  [self flushCaches];
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
  
  
  int iNumColours = pColourScheme->Reds.size();
  
  colour_t *ct = (colour_t *)malloc(iNumColours * sizeof(colour_t));
  
  for(int i = 0; i < iNumColours; i++) {
    ct[i].r = pColourScheme->Reds[i] / 255.0;
    ct[i].g = pColourScheme->Greens[i] / 255.0;
    ct[i].b = pColourScheme->Blues[i] / 255.0;
  }
  
  // colour table must be freed elsewhere...
  [self colourSchemeCallbackWithColourTable:ct];
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



- (void)dealloc
{
  [_cachedFontName release];
  [_zippyCache release];
  [_textAttributeCache release];
  
  [[self openGLContext] makeCurrentContext];
  
  if (colourTable != NULL) {
    free(colourTable);
  }
  
  [_letterDict release];
  
  [super dealloc];
}


- (void)gl_init
{
  [[self openGLContext] makeCurrentContext];
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glShadeModel(GL_FLAT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);

}

- (void)reshape {
  [self gl_reshape:[self bounds].size.width :[self bounds].size.height];
  
  delete( aquaDasherScreen );
  aquaDasherScreen = new COSXDasherScreen(self);
  [dasherApp changeScreen:aquaDasherScreen];
  
  [self adjustTrackingRect];  
}

- (void) gl_reshape:(int)w :(int)h
{
  [[self openGLContext] makeCurrentContext];
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
//  glOrtho(0, w, 0, h, -1.0, 1.0);
  glOrtho(0, w, h, 0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
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







@end
