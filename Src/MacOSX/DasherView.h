//
//  DasherView.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//



#import <AppKit/NSView.h>
#import <AppKit/NSNibDeclarations.h>

#include <string>

@class NSColor, NSTimer, NSString, NSMutableDictionary;
@class NSBezierPath;
@class ZippyCache, ZippyString;

unsigned long int get_time();


@interface DasherView : NSView {
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
}

- (void)blankCallback;
- (void)displayCallback;

- (IBAction)changeSpeed:(id)sender;

- (void)startDashing;
- (void)stopDashing;
- (BOOL)isDashing;


- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize;

- (NSString *)cachedFontName;
- (void)setCachedFontName:(NSString *)newCachedFontName;

- (void)clearRectCache;
- (void)drawRectCache;
- (void)addRect:(NSRect)aRect color:(NSColor *)aColor;

- (void)clearTextCache;
- (void)drawTextCache;
- (void)addText:(ZippyString *)aZippyString point:(NSPoint)aPoint;

- (NSBezierPath *)polylineCache;
- (void)setPolylineCache:(NSBezierPath *)newPolylineCache;

- (NSDictionary *)textAttributesWithTextSize:(int)aSize;
- (void)validateCacheWithFontName:(NSString *)fontName;

- (void)blankCallback;
- (void)displayCallback;
- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 color:(NSColor *)aColor;
- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize;

- (ZippyCache *)zippyCache;
- (void)setZippyCache:(ZippyCache *)newZippyCache;

- (NSMutableDictionary *)textAttributeCache;
- (void)setTextAttributeCache:(NSMutableDictionary *)newTextAttributeCache;

- (NSTimer *)timer;
- (void)setTimer:(NSTimer *)newTimer;


@end

