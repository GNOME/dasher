//
//  DasherAppDelegate.h
//  Dasher
//
//  Created by Alan Lawrence on 18/03/2009.
//  Copyright Cavendish Laboratory 2009. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CDasherInterfaceBridge.h"
#import "CDasherScreenBridge.h"
#import "TextView.h"

@class EAGLView;

@interface DasherAppDelegate : UIViewController <UIApplicationDelegate, UIActionSheetDelegate, UITextViewDelegate> {
    UIWindow *window;
    EAGLView *glView;
	TextView *text;
	NSRange selectedText;
	CDasherInterfaceBridge* _dasherInterface;
	UIButton *speedBtn;
	UIToolbar *tools;
  UILabel *messageLabel;
  BOOL doneSetup;
  BOOL m_bLandscapeSupported;
  /// Should really be part of UIViewController (lockable), below...but then, how to find?
  UILabel *screenLockLabel;
}

- (void)startTimer;
- (void)shutdownTimer;
- (void)outputCallback:(NSString *)s;
- (void)deleteCallback:(NSString *)s;
- (void)notifySpeedChange;
- (NSString *)textAtOffset:(int)offset Length:(int)length;
- (void)setLockText:(NSString *)s;
- (void)displayMessage:(NSString *)msg ID:(int)iId Type:(int)type;
- (void)setLandscapeSupported:(BOOL)supported;
+ (DasherAppDelegate *)theApp;

//@property (nonatomic, retain) IBOutlet EAGLView *glView;
//@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (readonly) CDasherInterfaceBridge *dasherInterface;
@end

@interface UIViewController (lockable)
- (void)doAsyncLocked:(NSString *)msg target:(id)obj selector:(SEL)sel param:(id)param,...;
@end
