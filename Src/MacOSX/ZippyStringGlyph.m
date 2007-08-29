//
//  ZippyStringGlyph.m
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <AppKit/AppKit.h>
#import "ZippyStringGlyph.h"

static NSTextStorage *textStorage = nil;
static NSLayoutManager *layoutManager = nil;
static NSTextContainer *textContainer = nil;

@implementation ZippyStringGlyph

+ (id)zippyStringWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  if (textStorage == nil) {
    textStorage = [[NSTextStorage alloc] initWithString:@"Here's to the crazy ones, the misfits, the rebels, the troublemakers, the round pegs in the square holes, the ones who see things differently."];
    layoutManager = [[NSLayoutManager alloc] init];
    textContainer = [[NSTextContainer alloc] init];
    [layoutManager addTextContainer:textContainer];
    [textContainer release];	// The layoutManager will retain the textContainer
    [textStorage addLayoutManager:layoutManager];
    [layoutManager release];	// The textStorage will retain the layoutManager
    
    // Screen fonts are not suitable for scaled or rotated drawing.
    // Views that use NSLayoutManager directly for text drawing should
    // set this parameter appropriately.
    [layoutManager setUsesScreenFonts:NO]; 
  }
  
  return [[[self alloc] initWithString:aString attributes:someAttributes] autorelease];
}

- (id)initWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  if (self = [super initWithString:aString attributes:someAttributes]) {
    NSAttributedString *as = [[NSAttributedString alloc] initWithString:aString attributes:someAttributes];
    [textStorage replaceCharactersInRange:NSMakeRange(0, [textStorage length]) withAttributedString:as];
    [as release];
    NSRange glyphRange = [layoutManager glyphRangeForTextContainer:textContainer];
    int maxNumGlyphs = glyphRange.length + 1;
    NSGlyph *glyphs = malloc(maxNumGlyphs);
    glyphCount = [layoutManager getGlyphs:glyphs range:glyphRange];
    [self setGlyphsData:[NSData dataWithBytesNoCopy:glyphs length:maxNumGlyphs]];

    NSBezierPath *bp = [NSBezierPath bezierPath];
    [bp moveToPoint:NSMakePoint(1, 1)];
    [bp appendBezierPathWithGlyphs:(NSGlyph *)[[self glyphsData] bytes] count:glyphCount inFont:[[self attributes] objectForKey:NSFontAttributeName]];

    [self setBezierPath:bp];
    

// TODO, can the NSGlyphs be turned into CGGlyphs, and then the +...packedGlyphs bezierpath method used?
  }

  return self;
}

- (void)drawAtPoint:(NSPoint)aPoint {
  
//  NSBezierPath *bp = [NSBezierPath bezierPath];
//  [bp moveToPoint:aPoint];

//  [bp appendBezierPathWithGlyphs:(NSGlyph *)[[self glyphsData] bytes] count:glyphCount inFont:[[self attributes] objectForKey:NSFontAttributeName]];
  
//  [[[self attributes] objectForKey:NSForegroundColorAttributeName] set];
//  [bp fill];
  
  NSAffineTransform *t = [NSAffineTransform transform];
  [t translateXBy:aPoint.x yBy:aPoint.y];
  //[t scaleXBy:5.0 yBy:5.0];
  [t concat];
  [[[self attributes] objectForKey:NSForegroundColorAttributeName] set];
  [[self bezierPath] fill];
  [t invert];
  [t concat];
}

- (NSData *)glyphsData {
  return [[glyphsData retain] autorelease];
}

- (void)setGlyphsData:(NSData *)value {
  if (glyphsData != value) {
    [glyphsData release];
    glyphsData = [value retain];
  }
}

- (NSBezierPath *)bezierPath {
  return [[bezierPath retain] autorelease];
}

- (void)setBezierPath:(NSBezierPath *)value {
  if (bezierPath != value) {
    [bezierPath release];
    bezierPath = [value retain];
  }
}


- (void)dealloc {
  [glyphsData release];
  [bezierPath release];
  [super dealloc];
}

@end
