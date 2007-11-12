//
//  DasherViewAqua.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//



#import <AppKit/NSView.h>
#import <AppKit/NSNibDeclarations.h>
#import "COSXDasherScreen.h"

@class NSColor, NSTimer, NSTextField, NSString, NSMutableDictionary;
@class NSBezierPath;
@class ZippyCache;
@class DasherApp;

@interface DasherViewAqua : NSView  <DasherViewCocoa> {
  
  COSXDasherScreen *aquaDasherScreen;
  
  IBOutlet DasherApp *dasherApp;
  
  NSTrackingRectTag trackingRectTag;

  NSString *_cachedFontName;

  ZippyCache *_zippyCache;

  NSMutableDictionary *_textAttributeCache;

  NSArray *_colourScheme;
  
  NSImage *_boxesBuffer;
  NSImage *_mouseBuffer;
  NSImage *currentBuffer;
}

- (void)finishRealization;

- (void)sendMarker:(int)iMarker;
- (void)blankCallback;
- (void)displayCallback;
- (void)drawRect:(NSRect)rect;
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)e;
- (void)mouseUp:(NSEvent *)e;
- (void)keyDown:(NSEvent *)e;
- (void)circleCallbackCentrePoint:(NSPoint)aCentrePoint radius:(float)aRadius outlineColorIndex:(int)anOutlineColorIndex fillColourIndex:(int)aFillColourIndex shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth;
- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex shouldOutline:(BOOL)shouldOutline shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth;
- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize colorIndex:(int)aColorIndex;
- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex;
- (void)polylineCallbackPoints:(NSArray *)points width:(int)aWidth colorIndex:(int)aColorIndex;
- (void)colourSchemeCallbackWithColours:(NSArray *)colours;
- (NSColor *)colorWithColorIndex:(int) aColor;
- (NSDictionary *)textAttributesWithTextSize:(int)aSize color:(NSColor *)aColor;
- (void)flushFontCache;
- (COSXDasherScreen *)aquaDasherScreen;
- (NSMutableDictionary *)textAttributeCache;
- (void)setTextAttributeCache:(NSMutableDictionary *)newTextAttributes;
- (id)initWithFrame:(NSRect)frame;
- (void)setupCache;
- (void)setFrame:(NSRect)newRect;
- (void)adjustTrackingRect;
- (BOOL)isFlipped;
- (BOOL)isOpaque;
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent;
- (void)awakeFromNib;
- (BOOL)acceptsFirstResponder;
- (NSArray *)colourScheme;
- (void)setColourScheme:(NSArray *)newColourScheme;
- (NSImage *)boxesBuffer;
- (void)setBoxesBuffer:(NSImage *)value;
- (NSImage *)mouseBuffer;
- (void)setMouseBuffer:(NSImage *)value;
- (void)dealloc;
- (ZippyCache *)zippyCache;
- (void)setZippyCache:(ZippyCache *)newZippyCache;
- (NSString *)cachedFontName;
- (void)setCachedFontName:(NSString *)newCachedFontName;
- (void)setupFrame:(NSRect)newRect;
@end

