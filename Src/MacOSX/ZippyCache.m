//
//  ZippyCache.m
//  RectAl
//
//  Created by Doug Dickinson on Mon May 26 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ZippyCache.h"
#import "ZippyString.h"

@implementation ZippyCache

- (ZippyString *)zippyStringWithString:(NSString *)aString size:(int)aSize attributes:(NSDictionary *)someAttributes {
  ZippyString *result = nil;
  NSMutableDictionary *sizeCache;

  if (latestSize == aSize && [latestString isEqualToString:aString]) {
    return latestZippyString;
  }

  sizeCache = [[self cache] objectForKey:[self sizeKeyForSize:aSize]];
  if (sizeCache == nil) {
    sizeCache = [NSMutableDictionary dictionary];
    [[self cache] setObject:sizeCache forKey:[self sizeKeyForSize:aSize]];
  }

  result = [sizeCache objectForKey:aString];
  if (result == nil) {
    result = [ZippyString zippyStringWithString:aString attributes:someAttributes];
    if ([aString length] == 1) {
      [sizeCache setObject:result forKey:aString];
    }
  }

  [latestString release];
  latestString = [aString retain];
  latestSize = aSize;
  [latestZippyString release];
  latestZippyString = [result retain];
  
  return result;
}

- (NSNumber *)sizeKeyForSize:(int)aSize {
  return [NSNumber numberWithInt:aSize];
}

+ zippyCache {
  return [[[self alloc] init] autorelease];
}

- init {
  if (self = [super init]) {
    [self setCache:[NSMutableDictionary dictionary]];
  }

  return self;
}

- (NSMutableDictionary *)cache {
  return _cache;
}

- (void)setCache:(NSMutableDictionary *)newCache {
  if (_cache != newCache) {
    NSMutableDictionary *oldValue = _cache;
    _cache = [newCache retain];
    [oldValue release];
  }
}

- (void)dealloc {
  [_cache release];
  [super dealloc];
}

@end
