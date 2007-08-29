//
//  ZippyCache.m
//  RectAl
//
//  Created by Doug Dickinson on Mon May 26 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ZippyCache.h"
#import "ZippyStringImage.h"
#import "ZippyStringGlyph.h"

#if defined(ZIPPY_STRING_GLYPH)
            
// ZippyStringGlyph is not yet ready for primetime
#define ZIPPY_STRING_CLASS ZippyStringGlyph
#else            
// not terribly sophisticated way of choosing which Zippy string type to use
#define ZIPPY_STRING_CLASS ZippyStringImage
#endif
            
@implementation ZippyCache

- (ZippyString *)zippyStringWithString:(NSString *)aString size:(int)aSize attributes:(NSDictionary *)someAttributes {
  ZippyString *result = nil;
  NSMutableDictionary *sizeCache;

  sizeCache = [[self cache] objectForKey:[self sizeKeyForSize:aSize]];
  if (sizeCache == nil) {
    sizeCache = [NSMutableDictionary dictionary];
    [[self cache] setObject:sizeCache forKey:[self sizeKeyForSize:aSize]];
  }

  result = [sizeCache objectForKey:aString];
  if (result == nil) {
    result = [ZIPPY_STRING_CLASS zippyStringWithString:aString attributes:someAttributes];
    if ([aString length] == 1) {
      [sizeCache setObject:result forKey:aString];
    }
  }

  return result;
}

- (NSNumber *)sizeKeyForSize:(int)aSize {
//#if defined(ZIPPY_STRING_GLYPH)
//  return [NSNumber numberWithInt:42]; // just testing
//#else 
  return [NSNumber numberWithInt:aSize];
//#endif
}

+ zippyCache {
  return [[[self alloc] init] autorelease];
}

- init {
  if (self = [super init]) {
    [self flush];
  }

  return self;
}

- (void)flush
{
  [self setCache:[NSMutableDictionary dictionary]];
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
