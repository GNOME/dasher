//
//  TextView.mm
//  Dasher
//
//  Created by Alan Lawrence on 16/07/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "TextView.h"


@implementation TextView


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

@end
