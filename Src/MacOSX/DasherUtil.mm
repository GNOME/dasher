//
//  DasherUtil.mm
//  Dasher
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import "DasherUtil.h"

#include "libdasher.h"

NSString *NSStringFromStdString(const std::string& aString)
{
  // inside DasherCore, std::string is used as a container for a string of UTF-8 bytes, so
  // we can just get the raw bytes (with c_str()) and shove them into an NSString.
  return [NSString stringWithUTF8String:aString.c_str()];
}

std::string *StdStringFromNSString(NSString *aString)
{
  return aString && [aString length] ? new std::string([aString UTF8String]) : new std::string();
}

void importTrainingFile(NSString *aFileName)
{
  dasher_train_file([aFileName cString]);
}


