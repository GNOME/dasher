//
//  ZippyString.h
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSImage, NSString, NSDictionary;

@interface ZippyString : NSObject {

  NSSize _size;
  NSString *_string;
  NSDictionary *_attributes;
}


+ (id)zippyStringWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes;

- (id)initWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes;

- (void)drawAtPoint:(NSPoint)aPoint;
  
- (NSSize)size;
- (void)setSize:(NSSize)newSize;
- (NSString *)string;
- (void)setString:(NSString *)newString;
- (NSDictionary *)attributes;
- (void)setAttributes:(NSDictionary *)newAttributes;

@end
