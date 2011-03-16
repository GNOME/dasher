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
#import "Actions.h"

@class EAGLView;
@class FliteTTS;

@interface DasherAppDelegate : UIViewController <UIApplicationDelegate, UIActionSheetDelegate, UITextViewDelegate> {
    UIWindow *window;
    EAGLView *glView;
  FliteTTS *fliteEng;
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
  ActionButton *actions;
  NSString *m_wordBoundary, *m_sentenceBoundary, *m_lineBoundary;
}

- (void)startTimer;
- (void)shutdownTimer;
- (void)setAlphabet:(const CAlphInfo *)pAlph;
- (void)outputCallback:(NSString *)s;
- (void)deleteCallback:(NSString *)s;
- (unsigned int)move:(CControlManager::EditDistance)amt forwards:(BOOL)bForwards;
- (unsigned int)del:(CControlManager::EditDistance)amt forwards:(BOOL)bForwards;
- (BOOL)supportsSpeech;
- (void)speak:(NSString *)text interrupt:(BOOL)bInt;
- (void)copy:(NSString *)text;
//forcibly inserts text - and then rebuilds the model
- (void)insertText:(NSString *)text;
- (void)clearBtn; //prompts for confirmation, then calls:
- (void)clearText;
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
