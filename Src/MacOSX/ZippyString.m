//
//  ZippyString.m
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
#import "ZippyString.h"


@implementation ZippyString

+ (id)zippyStringWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  return [[[self alloc] initWithString:aString attributes:someAttributes] autorelease];
}

- (id)initWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  if (self = [super init]) {
    NSImage *m;
    NSSize s = [aString sizeWithAttributes:someAttributes];
    [self setString:aString];
    [self setAttributes:someAttributes];
    [self setSize:[aString sizeWithAttributes:someAttributes]];
    if ([aString length] == 1 && s.width != 0 && s.height != 0) {
      m = [[NSImage alloc] initWithSize:[self size]];
      [m setFlipped:YES];
      [m lockFocus];
      [self drawAtPoint:NSMakePoint(0, 0)];
      [m unlockFocus];
      [self setImage:[m autorelease]];
    }
  }

  return self;
}

- (void)drawAtPoint:(NSPoint)aPoint {
  if ([self image] != nil) {
    NSPoint at = NSMakePoint(aPoint.x, aPoint.y + [self size].height);
    [[self image] compositeToPoint:at operation:NSCompositeSourceAtop];
    return;
  }
  
  [[self string] drawAtPoint:aPoint withAttributes:[self attributes]];
}

- (NSSize)size {
  return _size;
}

- (void)setSize:(NSSize)newSize {
  _size = newSize;
}

- (NSImage *)image {
  return _image;
}

- (void)setImage:(NSImage *)newImage {
  if (_image != newImage) {
    NSImage *oldValue = _image;
    _image = [newImage retain];
    [oldValue release];
  }
}

- (NSString *)string {
  return _string;
}

- (void)setString:(NSString *)newString {
  if (_string != newString) {
    NSString *oldValue = _string;
    _string = [newString retain];
    [oldValue release];
  }
}

- (NSDictionary *)attributes {
  return _attributes;
}

- (void)setAttributes:(NSDictionary *)newAttributes {
  if (_attributes != newAttributes) {
    NSDictionary *oldValue = _attributes;
    _attributes = [newAttributes retain];
    [oldValue release];
  }
}


- (void)dealloc {
  [_image release];
  [_string release];
  [_attributes release];
  [super dealloc];
}

@end
