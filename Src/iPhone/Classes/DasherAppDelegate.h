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

typedef enum {
  EDIT_CHAR,
  EDIT_WORD,
  EDIT_LINE,
  EDIT_FILE
} EEditDistance;

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
  UISlider *speedSlider;
  BOOL doneSetup;
  BOOL m_bLandscapeSupported;
  /// Should really be part of UIViewController (lockable), below...but then, how to find?
  UILabel *screenLockLabel;
  
  NSString *m_wordBoundary, *m_sentenceBoundary, *m_lineBoundary;
}

- (void)startTimer;
- (void)shutdownTimer;
- (void)setAlphabet:(CAlphabet *)pAlph;
- (void)outputCallback:(NSString *)s;
- (void)deleteCallback:(NSString *)s;
- (void)move:(EEditDistance)amt forwards:(BOOL)bForwards;
- (void)del:(EEditDistance)amt forwards:(BOOL)bForwards;
- (NSString *)allText;
- (void)notifySpeedChange;
- (NSString *)textAtOffset:(unsigned int)offset Length:(unsigned int)length;
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
