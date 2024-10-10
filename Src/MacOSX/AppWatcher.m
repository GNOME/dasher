//
//  AppWatcher.m
//  Dasher
//
//  Created by dougie on 12/04/2005.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "AppWatcher.h"
#import <Carbon/Carbon.h>

@implementation AppWatcher

- (id)init {
  if (self = [super init]) {
    [self setApps:[NSMutableArray arrayWithCapacity:20]];
    [self registerNotifications];
    [self fillApps];
  }
  return self;
}

- (AXUIElementRef)targetAppUIElementRef {

  if ([self targetAppInfo] == nil) {
    return NULL;
  }
  
  pid_t pid = [[[self targetAppInfo] valueForKey:@"NSApplicationProcessIdentifier"] intValue];
  // TODO - is there an object being created here? does it need deleting?
  return AXUIElementCreateApplication(pid);
}

- (int)targetAppPid {

  if ([self targetAppInfo] == nil) {
    return -1;
  }
  
  pid_t pid = [[[self targetAppInfo] valueForKey:@"NSApplicationProcessIdentifier"] intValue];
  return pid;
}

- (void)awakeFromNib {
  NSSortDescriptor *d = [[[NSSortDescriptor alloc] initWithKey:@"NSApplicationName" ascending:YES selector:@selector(caseInsensitiveCompare:)] autorelease];
  [appsController setSortDescriptors:[NSArray arrayWithObject:d]];
  [self setTargetAppInfo:nil];
}

- (void)fillApps {
  NSEnumerator *e = [[[NSWorkspace sharedWorkspace] launchedApplications] objectEnumerator];
  NSDictionary *d;
  while (d = [e nextObject]) {
    [self addAppInfo:d];
  }
}

- (unsigned long)indexOfAppWithApplicationName:(NSString *)aName {
  unsigned long len = [[self apps] count];
  unsigned long i;
  for (i = 0; i < len; i++) {
    NSDictionary *d = [[self apps] objectAtIndex:i];
    if ([[d objectForKey:@"NSApplicationName"] isEqualToString:aName]) {
      return i;
    }
  }
  
  return NSNotFound;
}

- (BOOL)psnOfAppInfo:(NSDictionary *)appInfo1 isEqualToPsnOfAppInfo:(NSDictionary *)appInfo2 {
  return ([[appInfo1 objectForKey:@"NSApplicationProcessSerialNumberHigh"] isEqual: [appInfo2 objectForKey:@"NSApplicationProcessSerialNumberHigh"]] && [[appInfo1 objectForKey:@"NSApplicationProcessSerialNumberLow"] isEqual: [appInfo2 objectForKey:@"NSApplicationProcessSerialNumberLow"]]);
}

- (unsigned long)indexOfAppWithInfo:(NSDictionary *)anAppInfo {
  unsigned long len = [[self apps] count];
  unsigned long i;
  for (i = 0; i < len; i++) {
    NSDictionary *d = [[self apps] objectAtIndex:i];
    if ([self psnOfAppInfo:d isEqualToPsnOfAppInfo:anAppInfo]) {
      return i;
    }
  }
  
  return NSNotFound;
}


- (void)seeAppLaunch:(NSNotification *)note {
  if ([self addAppInfo:[note userInfo]]) {
    [appsController rearrangeObjects];
    
    [self seeAppActive: note];
  }
}

- (NSString *)applicationName {
	
  return [[NSProcessInfo processInfo] processName];
}

// TODO problem if two apps have the same name - happens too infrequently to worry over now
// fix would be to have method -uniqueName which includes the psn for names which are ==
- (BOOL)addAppInfo:(NSDictionary *)anAppInfo {
    BOOL result = NO;
    if ([self indexOfAppWithInfo:anAppInfo] == NSNotFound) {
        [[self apps] addObject:anAppInfo];
        result = YES;
    }
    return result;
}

- (void)seeAppDie:(NSNotification *)note {
  NSDictionary *d = [note userInfo];
  unsigned long i = [self indexOfAppWithInfo:d];
  if (i != NSNotFound) {
    [[self apps] removeObjectAtIndex:i];
    [appsController rearrangeObjects];
    if ([self psnOfAppInfo:d isEqualToPsnOfAppInfo:[self targetAppInfo]]) {
      [self setTargetAppInfo:nil];
    }
  }
}

- (void)seeAppActive:(NSNotification *)note {
    NSRunningApplication* currentApp = [[NSWorkspace sharedWorkspace] frontmostApplication];
    unsigned long index = [self indexOfAppWithApplicationName: currentApp.localizedName];
    if (index != NSNotFound) {
        NSDictionary *d = [[self apps] objectAtIndex: index];
        if (![[d objectForKey:@"NSApplicationName"] isEqualToString:[self applicationName]]) {
            [self setTargetAppInfo: d];
        }
    }
}

- (void)registerNotifications {
  NSWorkspace *w = [NSWorkspace sharedWorkspace];
  NSNotificationCenter *nc = [w notificationCenter];
  [nc addObserver:self selector:@selector(seeAppLaunch:) name:NSWorkspaceDidLaunchApplicationNotification object:nil];
  [nc addObserver:self selector:@selector(seeAppDie:) name:NSWorkspaceDidTerminateApplicationNotification object:nil];
  [nc addObserver:self selector:@selector(seeAppActive:) name:NSWorkspaceDidActivateApplicationNotification object:nil];
}

- (NSMutableArray *)apps {
  return [[_apps retain] autorelease];
}

- (void)setApps:(NSMutableArray *)newApps {
  if (_apps != newApps) {
    [_apps release];
    _apps = [newApps retain];
  }
}

- (NSDictionary *)targetAppInfo {
  return [[_targetAppInfo retain] autorelease];
}

- (void)setTargetAppInfo:(NSDictionary *)newTargetAppInfo {
  if (_targetAppInfo != newTargetAppInfo) {
    [_targetAppInfo release];
    _targetAppInfo = [newTargetAppInfo retain];
    
    if (_targetAppInfo != nil && self.directMode) {
      pid_t pid = [[_targetAppInfo valueForKey:@"NSApplicationProcessIdentifier"] intValue];

      [self openAppByPid: pid];
    }
  }
}

- (void)openAppByPid: (pid_t) appPID {
    //Create event target.
    NSAppleEventDescriptor* targetDescriptor
     = [NSAppleEventDescriptor descriptorWithDescriptorType:typeKernelProcessID
                                                      bytes:&appPID
                                                     length:sizeof(appPID)];

    //Create "reopen" event.
    NSAppleEventDescriptor* reopenDescriptor
     = [NSAppleEventDescriptor appleEventWithEventClass:kCoreEventClass
                                                eventID:kAEReopenApplication
                                       targetDescriptor:targetDescriptor
                                               returnID:kAutoGenerateReturnID
                                          transactionID:kAnyTransactionID];

    //Create "activate" event.
    NSAppleEventDescriptor* activateDescriptor
     = [NSAppleEventDescriptor appleEventWithEventClass:kAEMiscStandards
                                                eventID:kAEActivate
                                       targetDescriptor:targetDescriptor
                                               returnID:kAutoGenerateReturnID
                                          transactionID:kAnyTransactionID];

    //Send "activate" followed by "reopen".
    AESendMessage([activateDescriptor aeDesc], NULL, kAENoReply, kAEDefaultTimeout);
    AESendMessage([reopenDescriptor   aeDesc], NULL, kAENoReply, kAEDefaultTimeout);

}
 
- (void)dealloc {
  [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
  [_apps release];
  [_targetAppInfo release];
  [super dealloc];
}

@end
