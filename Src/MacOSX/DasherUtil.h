//
//  DasherUtil.h
//  Dasher
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>

NSString *NSStringFromStdString(const std::string& aString);
std::string StdStringFromNSString(NSString *aString);

void importTrainingFile(NSString *aFileName);
