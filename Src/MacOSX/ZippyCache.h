//
//  ZippyCache.h
//  RectAl
//
//  Created by Doug Dickinson on Mon May 26 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

//#define ZIPPY_STRING_GLYPH

@class ZippyString;
@class NSMutableDictionary, NSString, NSDictionary, NSNumber;

@interface ZippyCache : NSObject {
  NSMutableDictionary *_cache;
}

+ zippyCache;

- (ZippyString *)zippyStringWithString:(NSString *)aString size:(int)aSize attributes:(NSDictionary *)someAttributes;

- (NSNumber *)sizeKeyForSize:(int)aSize;
- (void)flush;

- (NSMutableDictionary *)cache;
- (void)setCache:(NSMutableDictionary *)newCache;

@end
