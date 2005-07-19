//
//  DasherView.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <AppKit/NSView.h>
#import <AppKit/NSNibDeclarations.h>
#include "libdasher.h"

#include <string>

@ class NSColor, NSTimer, NSString, NSMutableDictionary;
@class NSBezierPath;
@class ZippyCache, ZippyString;

unsigned long int get_time();

@interface DasherView:NSView {
  BOOL isPaused;
  NSTrackingRectTag trackingRectTag;
  int rectCacheCount;
  NSRect *rectCache;
  NSColor **rectColorCache;

  int textCacheCount;
  ZippyString **textCache;
  NSPoint *textPointCache;

  NSBezierPath *_polylineCache;

  NSString *_cachedFontName;

  ZippyCache *_zippyCache;

  NSMutableDictionary *_textAttributeCache;

  NSTimer *_timer;

  NSColor *special1Color;
  NSColor *special2Color;
  NSColor *objectsColor;
  NSColor *groupsColor[3];
  NSColor *nodes1Color[3];
  NSColor *nodes2Color[3];

}

-(id) initWithFrame:(NSRect) frame;
-(void)setFrame:(NSRect) newRect;
-(void)adjustTrackingRect;
-(BOOL) isFlipped;
-(BOOL) isOpaque;
-(BOOL) acceptsFirstMouse:(NSEvent *) theEvent;
-(void)awakeFromNib;
-(BOOL) acceptsFirstResponder;
-(BOOL) becomeFirstResponder;
-(BOOL) resignFirstResponder;
-(void)drawRect:(NSRect) rect;
-(void)mouseEntered:(NSEvent *) theEvent;
-(void)mouseExited:(NSEvent *) theEvent;
-(void)mouseDown:(NSEvent *) e;
-(void)keyDown:(NSEvent *) e;
-(void)startDashing;
-(void)stopDashing;
-(BOOL) isDashing;
-(void)toggleDashing;
-(void)blankCallback;
-(void)displayCallback;
-(void)rectangleCallbackX1:(int)
     x1 y1:(int)
     y1 x2:(int)
     x2 y2:(int)
     y2 colorNumber:(int)
     aColorNumber colorScheme:(Opts::ColorSchemes) aColorScheme;
-(NSSize) textSizeCallbackWithString:(NSString *)
     aString size:(int)aSize;
-(void)drawTextCallbackWithString:(NSString *)
     aString x1:(int)
     x1 y1:(int)
     y1 size:(int)aSize;
-(void)polylineCallbackPoints:(NSArray *)points;
-(void)setupColors;
-(NSColor *)colorWithColorNumber:(int)
     aColor colorScheme:(Opts::ColorSchemes) aColorScheme;
-(IBAction) changeFont:(id) sender;
-(NSDictionary *)textAttributesWithTextSize:(int)aSize;
-(void)validateCacheWithFontName:(NSString *)fontName;
-(NSMutableDictionary *)textAttributeCache;
-(void)setTextAttributeCache:(NSMutableDictionary *)newTextAttributes;
-(void)clearTextCache;
-(void)drawTextCache;
-(void)addText:(ZippyString *)
     aZippyString point:(NSPoint) aPoint;
-(void)addRect:(NSRect)
     aRect color:(NSColor *) aColor;
-(void)drawRectCache;
-(void)clearRectCache;
-(NSString *)cachedFontName;
-(void)setCachedFontName:(NSString *)newCachedFontName;
-(ZippyCache *)zippyCache;
-(void)setZippyCache:(ZippyCache *)newZippyCache;
-(NSBezierPath *)polylineCache;
-(void)setPolylineCache:(NSBezierPath *)newPolylineCache;
-(void)clearPolylineCache;
-(NSTimer *)timer;
-(void)setTimer:(NSTimer *)newTimer;
-(void)timerCallback:(NSTimer *)aTimer;
-(void)dealloc;

@end
