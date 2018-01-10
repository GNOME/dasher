//
//  DasherUtil.h
//  Dasher
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>

unsigned long get_time();

NSString *NSStringFromStdString(const std::string& aString);
std::string StdStringFromNSString(const NSString *const aString);

