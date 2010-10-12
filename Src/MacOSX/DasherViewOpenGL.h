//
//  DasherViewOpenGL.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//



#import <AppKit/NSOpenGLView.h>
#import <AppKit/NSNibDeclarations.h>
#import "COSXDasherScreen.h"
#import "AlphabetLetter.h"
#import "ColourIO.h"
#import "DasherViewCocoa.h"
#import "KeyboardHelper.h"

@class NSColor, NSTimer, NSTextField, NSString, NSMutableDictionary;
@class NSBezierPath;
@class DasherApp;

typedef struct {
  float r, g, b;
} colour_t;


@interface DasherViewOpenGL : NSOpenGLView <DasherViewCocoa> {
  
  COSXDasherScreen *aquaDasherScreen;
  
  IBOutlet DasherApp *dasherApp;
  
  NSTrackingRectTag trackingRectTag;

  NSString *_cachedFontName;

  NSMutableDictionary *_textAttributeCache;

  NSArray *_colourScheme;

  GLuint frameBuffers[2];
  GLuint textures[2];
  GLfloat texcoords[8];
  
  ///current (last successful) framebuffer width & height
	int fw,fh;
  
  colour_t *colourTable;
  
  NSMutableDictionary *_letterDict;
  CKeyboardHelper *_keyboardHelper;
  
}

- (void)sendMarker:(int)iMarker;
- (void)displayCallback;
- (void)drawRect:(NSRect)rect;
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)e;
- (void)mouseUp:(NSEvent *)e;
- (void)keyDown:(NSEvent *)e;
- (void)keyUp:(NSEvent *)e;
- (void)circleCallbackCentrePoint:(NSPoint)aCentrePoint radius:(float)aRadius outlineColorIndex:(int)anOutlineColorIndex fillColourIndex:(int)aFillColourIndex shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth;
- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex lineWidth:(int)aLineWidth;
- (AlphabetLetter *)letterForString:(NSString *)aString;
- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize;
- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex;
- (void)colourSchemeCallbackWithColourTable:(colour_t *)aColourTable;
- (void)polygonCallbackPoints:(NSArray *)points fillColorIndex:(int)fColorIndex outlineColorIndex:(int)iColorIndex lineWidth:(int)aWidth;
- (void)polylineCallbackPoints:(NSArray *)points width:(int)aWidth colorIndex:(int)aColorIndex;
- (id)initWithFrame:(NSRect)frame;
- (void)userDefaultsDidChange:(NSNotification *)aNote;
- (void)adjustTrackingRect;
- (BOOL)isFlipped;
- (BOOL)isOpaque;
- (void)awakeFromNib;
- (void)finishRealization;
- (COSXDasherScreen *)aquaDasherScreen;
- (BOOL)acceptsFirstResponder;
- (NSArray *)colourScheme;
- (void)setColourScheme:(NSArray *)newColourScheme;
- (void)setColourSchemeFromColourInfo:(const CColourIO::ColourInfo *)pColourScheme;
- (NSString *)cachedFontName;
- (void)setCachedFontName:(NSString *)newCachedFontName;
- (void)dealloc;
- (void)gl_init;
- (void)reshape;
- (void) gl_reshape:(int)w :(int)h;
- (void)flushCaches;

- (NSPoint)mouseLocation;
- (float)boundsWidth;
- (float)boundsHeight;


@end

