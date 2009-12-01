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

COSXSettingsStore::COSXSettingsStore(Dasher::CEventHandler *pEventHandler):CSettingsStore(pEventHandler) {
  LoadPersistent();
};

COSXSettingsStore::~COSXSettingsStore() {
};

NSDictionary *COSXSettingsStore::ParameterDictionary() {
  static NSMutableDictionary *parameterDictionary = nil;
  
  if (parameterDictionary == nil) {
    parameterDictionary = [[NSMutableDictionary alloc] initWithCapacity:NUM_OF_BPS + NUM_OF_LPS + NUM_OF_SPS];
    int ii;
 
    for(ii = 0; ii < NUM_OF_BPS; ii++) {
      [parameterDictionary setObject:[NSDictionary dictionaryWithObjectsAndKeys:
        NSStringFromStdString(s_oParamTables.BoolParamTable[ii].regName), @"regName",
        NSStringFromStdString(s_oParamTables.BoolParamTable[ii].humanReadable), @"humanReadable",
        [NSNumber numberWithInt:s_oParamTables.BoolParamTable[ii].key], @"key",
        nil] forKey:NSStringFromStdString(s_oParamTables.BoolParamTable[ii].regName)];
    }
    
    for(ii = 0; ii < NUM_OF_LPS; ii++) {
      [parameterDictionary setObject:[NSDictionary dictionaryWithObjectsAndKeys:
        NSStringFromStdString(s_oParamTables.LongParamTable[ii].regName), @"regName",
        NSStringFromStdString(s_oParamTables.LongParamTable[ii].humanReadable), @"humanReadable",
        [NSNumber numberWithInt:s_oParamTables.LongParamTable[ii].key], @"key",
        nil] forKey:NSStringFromStdString(s_oParamTables.LongParamTable[ii].regName)];
    }
    
    for(ii = 0; ii < NUM_OF_SPS; ii++) {
      [parameterDictionary setObject:[NSDictionary dictionaryWithObjectsAndKeys:
        NSStringFromStdString(s_oParamTables.StringParamTable[ii].regName), @"regName",
        NSStringFromStdString(s_oParamTables.StringParamTable[ii].humanReadable), @"humanReadable",
        [NSNumber numberWithInt:s_oParamTables.StringParamTable[ii].key], @"key",
        nil] forKey:NSStringFromStdString(s_oParamTables.StringParamTable[ii].regName)];
    }
  }
  
  return parameterDictionary;
}


int COSXSettingsStore::GetParameterIndex(const std::string & aKey) {
  NSString *key = NSStringFromStdString(aKey);
  NSDictionary *parameterEntry = [ParameterDictionary() objectForKey:key];
  if (parameterEntry == nil) {
    NSLog(@"COSXSettingsStore::GetParameterIndex - unknown key: %@", key);
    return NSNotFound;
  }
  
  return [[parameterEntry objectForKey:@"key"] intValue];
}

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
