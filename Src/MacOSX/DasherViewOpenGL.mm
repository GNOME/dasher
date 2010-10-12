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

#import "DasherUtil.h"
#import "DasherApp.h"
#import "GLUtils.h"

#import "COSXDasherScreen.h"

#define MAX_CACHE_COUNT 1000

@implementation DasherViewOpenGL

/*
 BEWARE!
 This is doing funny stuff with OpenGL framebuffers - rendering the boxes and the decorations to separate buffers, then compositing the two together.  The buffer switching is in sendMarker:
 */

- (void)sendMarker:(int)iMarker {
  [[self openGLContext] makeCurrentContext];
  if (iMarker == -1)
  {
	  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }
  else
  {
	  //NSLog(@"SendMarker %i\n",iMarker);
	  glDisable(GL_TEXTURE_2D);
	  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffers[iMarker]);
	  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textures[iMarker], 0);
  }
  //glMatrixMode(GL_PROJECTION); glLoadIdentity();
  //glOrtho(0, [self boundsWidth], [self boundsHeight], 0, -1.0, 1.0);
  //glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glClearColor(1.0, 1.0, 1.0, iMarker == 1 ? 0.0 /*transparent*/ : 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

- (void)displayCallback
{
  [self sendMarker:-1];
  glEnable(GL_TEXTURE_2D);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  NSSize r = [self bounds].size;
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  GLshort coords[] = {0,0, r.width,0, 0,r.height, r.width,r.height};
  glVertexPointer(2, GL_SHORT, 0, coords);
  glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
  
  for (int i=0; i<2; i++)
  {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
  glFlush();
}

- (void)drawRect:(NSRect)rect {
  //if (![dasherApp aquaDasherControl]->GetBoolParameter(BP_DASHER_PAUSED)) {
    // (ACL) want to redraw even if paused, to get mouse line etc. So invoke
    // standard Dasher redraw procedure (but not too early or it'll crash!)
  if ([dasherApp timer]) {	
    [dasherApp aquaDasherControl]->goddamn(get_time(), false); //E
    
//  [[self openGLContext] flushBuffer]; // (ACL) done in displayCallback
  }
  
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
  /*TODO, note that this isn't really "key down", rather it's "character entered"
	or similar - if the key is held down long enough to repeat, we get multiple keyDowns
   before a keyUp, and we just send them all along to the DasherCore code...*/
  NSString *chars = [e characters];
  if ([chars length] > 1)
    NSLog(@"KeyDown event for %i chars %@ - what to do? Ignoring all but first...\n", [chars length], chars);
  int keyCode = _keyboardHelper->ConvertKeyCode([chars characterAtIndex:0]);
  if (keyCode != -1)
    [dasherApp aquaDasherControl]->KeyDown(get_time(), keyCode);
}

- (void)keyUp:(NSEvent *)e
{
  NSString *chars = [e characters];
  if ([chars length] > 1)
    NSLog(@"KeyUp event for %i chars %@ - what to do? Ignoring all but first...\n", [chars length], chars);
  int keyCode = _keyboardHelper->ConvertKeyCode([chars characterAtIndex:0]);
  if (keyCode != -1)
    [dasherApp aquaDasherControl]->KeyUp(get_time(), keyCode);
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


- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)oci lineWidth:(int)aLineWidth {
  
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
  glDisable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  if (aFillColorIndex!=-1) {
    glColor4f(colourTable[aFillColorIndex].r, colourTable[aFillColorIndex].g, colourTable[aFillColorIndex].b, 1.0);
    GLshort coords[8] = {x1,y1, x2,y1, x1,y2, x2,y2};
    glVertexPointer(2, GL_SHORT, 0, coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
  
  if (aLineWidth>0) {
    glColor4f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b, 1.0);
    glLineWidth(aLineWidth);
    GLshort coords[] = {x1,y1, x2,y1, x2,y2, x1,y2};
    glVertexPointer(2, GL_SHORT, 0, coords);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  }
  
  
}


- (AlphabetLetter *)letterForString:(NSString *)aString {
  AlphabetLetter *result = [_letterDict objectForKey:aString];
  if (result == nil) {
    result = [[AlphabetLetter alloc] initWithString:aString];
    [_letterDict setObject:result forKey:aString];
  }
  return result;
}

- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize
{
  return [[self letterForString:aString] sizeWithSize:aSize];
}


- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex
{
  AlphabetLetter *letter = [self letterForString:aString];
  glEnable(GL_TEXTURE_2D);
  // TODO could pass the whole colour_t in and let it deal with splitting out the items
  [letter drawWithSize:/*1.0*/ aSize x:x1 y:y1 r:colourTable[aColorIndex].r g:colourTable[aColorIndex].g b:colourTable[aColorIndex].b];
  glDisable(GL_TEXTURE_2D);
}

- (void)colourSchemeCallbackWithColourTable:(colour_t *)aColourTable {
  
  if (colourTable != NULL) {
    free(colourTable);
  }
  
  colourTable = aColourTable;
}

- (void)polygonCallbackPoints:(NSArray *)points fillColorIndex:(int)fColorIndex outlineColorIndex:(int)iColorIndex lineWidth:(int)iWidth {
  int len = [points count];
	if (len < 2) return;
  //1. fill...
	glDisable(GL_TEXTURE_2D);
  if (fColorIndex != -1) {
    glColor3f(colourTable[fColorIndex].r, colourTable[fColorIndex].g, colourTable[fColorIndex].b);
	  glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < len; i++)
    {
      NSPoint nsp = [[points objectAtIndex:i] pointValue];
      glVertex2i(nsp.x,nsp.y);
    }
    glEnd();
  }
  if (iWidth>0) {
    glColor3f(colourTable[iColorIndex].r,colourTable[iColorIndex].g,colourTable[iColorIndex].b);
    glLineWidth(iWidth);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < len; i++)
    {
      NSPoint nsp = [[points objectAtIndex:i] pointValue];
      glVertex2i(nsp.x,nsp.y);
    }
    glEnd();
  }
}

- (void)polylineCallbackPoints:(NSArray *)points width:(int)aWidth colorIndex:(int)aColorIndex
{
	int len = [points count];
	if (len < 2) return;
	glDisable(GL_TEXTURE_2D);
	glColor3f(colourTable[aColorIndex].r, colourTable[aColorIndex].g, colourTable[aColorIndex].b);
	glLineWidth(aWidth);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < len; i++)
	{
		NSPoint nsp = [[points objectAtIndex:i] pointValue];
		glVertex2i(nsp.x,nsp.y);
	}
	glEnd();
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
    //note these give us framebuffer _references_...
    glGenFramebuffersEXT(2, frameBuffers);
    glGenTextures(2, textures);
    //...i.e. they don't identify any storage at this point!
    fw = fh = -1;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
	  
    _keyboardHelper = new CKeyboardHelper();	  
  }
  return self;
}

- (void)userDefaultsDidChange:(NSNotification *)aNote {
  if (![[[NSUserDefaults standardUserDefaults] objectForKey:@"DasherFont"] isEqualToString:[self cachedFontName]]) {
    [self flushCaches];
    [dasherApp aquaDasherControl]->ScheduleRedraw();
  }
}

- (void)flushCaches {
  [_letterDict release];
  _letterDict = [[NSMutableDictionary alloc] init];
  [self setCachedFontName:[[NSUserDefaults standardUserDefaults] objectForKey:@"DasherFont"]];
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
  return YES;
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
  //glClearColor(1.0, 1.0, 1.0, 1.0);
  glShadeModel(GL_FLAT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //glEnable(GL_TEXTURE_2D);
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
  int tw = P2Ceiling(w), th = P2Ceiling(h);
  if (tw!=fw || th!=fh) {
    //need to (re)allocate storage for framebuffer...
    for (int i=0; i<2; i++)
    {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffers[i]);
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, nil);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textures[i], 0);
      if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
      {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
        //...and do what, to report failure? we can't render anything, so Dasher is useless...?
      }
    }
    fw=tw; fh=th;
  }
  GLfloat tc_x = w/(double)tw, tc_y = h/(double)th;
  texcoords[0] = 0.0; texcoords[1] = tc_y;
  texcoords[2] = tc_x; texcoords[3] = tc_y;
  texcoords[4] = 0.0; texcoords[5] = 0.0;
  texcoords[6] = tc_x; texcoords[7] = 0.0;
  
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
