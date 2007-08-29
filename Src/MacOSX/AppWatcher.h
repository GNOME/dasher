//
//  AppWatcher.h
//  Dasher
//
//  Created by dougie on 12/04/2005.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface AppWatcher : NSObject {
  NSMutableArray *_apps;
  NSDictionary *_targetAppInfo;
  NSTimer *watchActiveAppTimer;
  BOOL lockTargetApp;
  IBOutlet NSArrayController *appsController;
}

- (AXUIElementRef)targetAppUIElementRef;
- (BOOL)psnOfAppInfo:(NSDictionary *)appInfo1 isEqualToPsnOfAppInfo:(NSDictionary *)appInfo2;
- (id)init;
- (void)startTimer;
- (void)stopTimer;
- (void)checkActiveApp:(NSTimer *)timer;
- (void)awakeFromNib;
- (void)fillApps;
- (unsigned int)indexOfAppWithApplicationName:(NSString *)aName;
- (unsigned int)indexOfAppWithInfo:(NSDictionary *)anAppInfo;
- (void)seeAppLaunch:(NSNotification *)note;
- (NSString *)applicationName;
- (BOOL)addAppInfo:(NSDictionary *)anAppInfo;
- (void)seeAppDie:(NSNotification *)note;
- (void)registerNotifications;
- (NSMutableArray *)apps;
- (void)setApps:(NSMutableArray *)newApps;
- (NSDictionary *)targetAppInfo;
- (void)setTargetAppInfo:(NSDictionary *)newTargetAppInfo;
- (BOOL)lockTargetApp;
- (void)setLockTargetApp:(BOOL)newLockTargetApp;
- (void)dealloc;

@end
