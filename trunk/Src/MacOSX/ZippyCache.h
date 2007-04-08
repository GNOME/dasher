//
//  ZippyCache.h
//  RectAl
//
//  Created by Doug Dickinson on Mon May 26 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@ class ZippyString;
@class NSMutableDictionary, NSString, NSDictionary, NSNumber;

@interface ZippyCache:NSObject {
  NSMutableDictionary *_cache;
  NSString *latestString;
  int latestSize;
  ZippyString *latestZippyString;
}

+zippyCache;

-(ZippyString *) zippyStringWithString:(NSString *)
     aString size:(int)
     aSize attributes:(NSDictionary *) someAttributes;

-(NSNumber *)sizeKeyForSize:(int)aSize;

-(NSMutableDictionary *)cache;
-(void)setCache:(NSMutableDictionary *)newCache;

@end
