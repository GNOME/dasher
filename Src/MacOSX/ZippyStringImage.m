//
//  ZippyStringImage.m
//  RectAl
//
//  Created by Doug Dickinson on Sat May 24 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <AppKit/AppKit.h>
#import "ZippyStringImage.h"


@implementation ZippyStringImage

- (id)initWithString:(NSString *)aString attributes:(NSDictionary *)someAttributes {
  if (self = [super initWithString:aString attributes:someAttributes]) {
    NSSize s = [self size];
    if ([aString length] == 1 && s.width != 0 && s.height != 0) {
      NSImage *m = [[NSImage alloc] initWithSize:s];
      [m setFlipped:YES];
      [m lockFocus];
      [self drawAtPoint:NSMakePoint(0, 0)];
      [m unlockFocus];
      [self setImage:m];
      [m release];
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
  
  [super drawAtPoint:aPoint];
}


- (NSImage *)image {
  return [[_image retain] autorelease];
}

- (void)setImage:(NSImage *)value {
  if (_image != value) {
    [_image release];
    _image = [value retain];
  }
}


- (void)dealloc {
  [_image release];
  [super dealloc];
}

@end
