//
//  ZippyStringImage.h
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "ZippyString.h"

@class NSImage, NSString, NSDictionary;

@interface ZippyStringImage : ZippyString {

  NSImage *_image;
}

- (NSImage *)image;
- (void)setImage:(NSImage *)newImage;

@end
