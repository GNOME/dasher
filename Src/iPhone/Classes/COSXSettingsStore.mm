/*
 *  COSXSettingsStore.cpp
 *  Dasher
 *
 *  Created by Doug Dickinson on 18/11/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#import "COSXSettingsStore.h"

#import <iostream>
#import "DasherUtil.h"

COSXSettingsStore::COSXSettingsStore() {
  LoadPersistent();
};

COSXSettingsStore::~COSXSettingsStore() {
};

/*
 the default values for all the parameters are stored in the core.  Rather than trying to ferret out those values in order to construct a registration domain, I'm just going to get the object for the key and if it is nil, let the core know I failed to read the default and it will supply the correct default value.
 */
bool COSXSettingsStore::LoadSetting(const std::string & Key, bool * Value) {
  if ([[NSUserDefaults standardUserDefaults] objectForKey:NSStringFromStdString(Key)] == nil) {
    return false;
  }
  
  BOOL result = [[NSUserDefaults standardUserDefaults] boolForKey:NSStringFromStdString(Key)];
  *Value = result == YES;
  return true;
}

bool COSXSettingsStore::LoadSetting(const std::string & Key, long *Value) {
  if ([[NSUserDefaults standardUserDefaults] objectForKey:NSStringFromStdString(Key)] == nil) {
    return false;
  }
  // TODO are int/long interchangeable?
  int result = [[NSUserDefaults standardUserDefaults] integerForKey:NSStringFromStdString(Key)];
  *Value = (long)result;
  return true;
}

bool COSXSettingsStore::LoadSetting(const std::string & Key, std::string * Value) {
  if ([[NSUserDefaults standardUserDefaults] objectForKey:NSStringFromStdString(Key)] == nil) {
    return false;
  }
  NSString *result = [[NSUserDefaults standardUserDefaults] stringForKey:NSStringFromStdString(Key)];
  // TODO what are the memory management implications here?
  *Value = StdStringFromNSString(result);
  return true;
}

void COSXSettingsStore::SaveSetting(const std::string & Key, bool Value) {
  [[NSUserDefaults standardUserDefaults] setBool:Value forKey:NSStringFromStdString(Key)];
}

void COSXSettingsStore::SaveSetting(const std::string & Key, long Value) {
  [[NSUserDefaults standardUserDefaults] setInteger:Value forKey:NSStringFromStdString(Key)];
}

void COSXSettingsStore::SaveSetting(const std::string & Key, const std::string & Value) {
  [[NSUserDefaults standardUserDefaults] setObject:NSStringFromStdString(Value) forKey:NSStringFromStdString(Key)];
}

bool COSXSettingsStore::IsParameterSaved(const std::string &Key) {
    return [[NSUserDefaults standardUserDefaults] objectForKey:NSStringFromStdString(Key)];
}
