/*
 *  COSXSettingsStore.h
 *  Dasher
 *
 *  Created by Doug Dickinson on 18/11/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#import <string>

#import "SettingsStore.h"

#import <stdio.h>

@class NSDictionary;

class COSXSettingsStore:public Dasher::CSettingsStore {
public:
  COSXSettingsStore();
  ~COSXSettingsStore();
  
private:

  bool LoadSetting(const std::string & Key, bool * Value);
  bool LoadSetting(const std::string & Key, long *Value);
  bool LoadSetting(const std::string & Key, std::string * Value);
  
  void SaveSetting(const std::string & Key, bool Value);
  void SaveSetting(const std::string & Key, long Value);
  void SaveSetting(const std::string & Key, const std::string & Value);
};

