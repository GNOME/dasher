//
//  LanguagesController.h
//  Dasher
//
//  Created by Alan Lawrence on 20/04/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface StringParamController : UITableViewController {
	NSArray *items;
	NSIndexPath *selectedPath;
  int m_iParam;
}

-(id)initWithTitle:(NSString *)title image:(UIImage *)image settingParam:(int)iParam;
-(void)doSet:(NSString *)val;
@end
