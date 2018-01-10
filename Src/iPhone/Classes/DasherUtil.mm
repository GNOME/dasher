//
//  DasherUtil.mm
//  Dasher
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "DasherUtil.h"
#include <sys/time.h>

unsigned long get_time() {
    // We need to provide a monotonic time source that ticks every millisecond
  long s_now;
  long ms_now;
  
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  
  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;
  
  unsigned long result = ((((unsigned long)s_now) & 0x0000ffff) * 1000UL) + ms_now;
  return result;
}



NSString *NSStringFromStdString(const std::string& aString)
{
  // inside DasherCore, std::string is used as a container for a string of UTF-8 bytes, so
  // we can just get the raw bytes (with c_str()) and shove them into an NSString.
  return [NSString stringWithUTF8String:aString.c_str()];
}

std::string StdStringFromNSString(const NSString *const aString)
{
  std::string result;

  if (aString && [aString length]) {
    result = std::string([aString UTF8String]);
  } else {
    result = std::string("");
  }

  return result;
}

