//
//  ActionConfigurator.h
//  Dasher
//
//  Created by Alan Lawrence on 24/11/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import "Actions.h"

@interface ActionConfigurator : UITableViewController {
  ActionButton *button;
  UIView *headers[3];
}
-(id)init;
@end
