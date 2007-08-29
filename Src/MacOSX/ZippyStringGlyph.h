//
//  ZippyStringGlyph.h
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "ZippyString.h"

@class NSImage, NSString, NSDictionary, NSBezierPath;

@interface ZippyStringGlyph : ZippyString {
  NSData *glyphsData;
  int glyphCount;
  NSBezierPath *bezierPath;
}
  
+ (id)zippyStringWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes;
- (id)initWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes;
- (void)drawAtPoint:(NSPoint)aPoint;
- (NSData *)glyphsData;
- (void)setGlyphsData:(NSData *)value;
- (NSBezierPath *)bezierPath;
- (void)setBezierPath:(NSBezierPath *)value;

- (void)dealloc;


@end
