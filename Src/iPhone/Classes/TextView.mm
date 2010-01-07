//
//  TextView.mm
//  Dasher
//
//  Created by Alan Lawrence on 16/07/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "TextView.h"


@implementation TextView

@synthesize bLandscape;

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
    }
    return self;
}

-(void)performBecomeEditableTasks {
	//shouldn't be called, but disable it just in case...
	//...to make dang sure that keyboard never pops up!
	//i.e., do not: [super performBecomeEditableTasks];
}

-(void)setEditable:(BOOL)bEditable {
  if (!bEditable) [super setEditable:NO];
}

-(void)drawRect:(CGRect) rect {
  [super drawRect:rect];
  //add one-pixel border along appropriate edge, to separate from Dasher "canvas"
  CGContextRef currentContext = UIGraphicsGetCurrentContext();
  CGContextSetLineWidth(currentContext, 1.0); //or whatever width you want
  CGContextSetRGBStrokeColor(currentContext, 0.0, 0.0, 0.0, 1.0);
  CGContextBeginPath(currentContext);
  CGSize sz = self.frame.size;
  if (bLandscape) {
    CGContextMoveToPoint(currentContext,sz.width,0.0);
    CGContextAddLineToPoint(currentContext, sz.width, sz.height);
  } else {
    CGContextMoveToPoint(currentContext,0.0,0.0);
    CGContextAddLineToPoint(currentContext, sz.width, 0.0);
  }
  CGContextStrokePath(currentContext);
}

@end
