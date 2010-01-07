//
//  TextView.h
//  Dasher
//
//  Created by Alan Lawrence on 16/07/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TextView : UITextView {
  BOOL bLandscape;
}

/// TRUE => draw border along right edge; FALSE => draw border along top edge
@property (nonatomic) BOOL bLandscape;
@end
