//
//  ZippyString.m
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <AppKit/AppKit.h>
#import "ZippyString.h"


@implementation ZippyString

+ (id)zippyStringWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  return [[[self alloc] initWithString:aString attributes:someAttributes] autorelease];
}

- (id)initWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  if (self = [super init]) {
    [self setString:aString];
    [self setAttributes:someAttributes];
    [self setSize:[aString sizeWithAttributes:someAttributes]];
  }

  return self;
}

- (void)drawAtPoint:(NSPoint)aPoint {
  [[self string] drawAtPoint:aPoint withAttributes:[self attributes]];
}

- (NSSize)size {
  return _size;
}

- (void)setSize:(NSSize)newSize {
  _size = newSize;
}

- (NSString *)string {
  return [[_string retain] autorelease];
}

- (void)setString:(NSString *)value {
  if (_string != value) {
    [_string release];
    _string = [value retain];
  }
}

- (NSDictionary *)attributes {
  return [[_attributes retain] autorelease];
}

- (void)setAttributes:(NSDictionary *)value {
  if (_attributes != value) {
    [_attributes release];
    _attributes = [value retain];
  }
}


- (void)dealloc {
  [_string release];
  [_attributes release];
  [super dealloc];
}

@end
