//
//  InputMethodSelector.h
//  Dasher
//
//  Created by Alan Lawrence on 20/04/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface InputMethodSelector : UITableViewController {
	UINavigationController *encNavCon;
	NSIndexPath *selectedPath;
}

- (id)initWithNavCon:(UINavigationController *)encNavCon;

@end
