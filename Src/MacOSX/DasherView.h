/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 20 April 2003
 */


#import <AppKit/NSView.h>
@class NSColor, nSString, NSMutableDictionary;
@class NSBezierPath;
@class ZippyCache, ZippyString;

@interface DasherView : NSView {
  BOOL isPaused;

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

#if defined(TOY)
  NSString *fontName;
#endif
  
}


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

@end

