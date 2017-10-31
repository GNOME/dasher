//
//  DasherAppDelegate.h
//  Dasher
//
//  Created by Alan Lawrence on 18/03/2009.
//  Copyright Cavendish Laboratory 2009. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CDasherInterfaceBridge.h"
#import "TextView.h"
#import "Actions.h"

@class EAGLView;
@class FliteTTS;

@interface DasherAppDelegate : UIViewController <UIApplicationDelegate, UIActionSheetDelegate, UITextViewDelegate, UIWebViewDelegate> {
    EAGLView *glView;
  FliteTTS *fliteEng;
	TextView *textView;
  UIWebView *webView;
	NSRange selectedText;
	CDasherInterfaceBridge* _dasherInterface;
	UIToolbar *tools;
  UITextView *messageLabel;
  UISlider *speedSlider;
  BOOL doneSetup;
  BOOL m_bAllowsRotation;
  /// Should really be part of UIViewController (lockable), below...but then, how to find?
  UILabel *screenLockLabel;
  NSMutableArray *toolbarItems;
  NSString *m_wordBoundary, *m_sentenceBoundary, *m_lineBoundary;
}

- (void)setAlphabet:(const Dasher::CAlphInfo *)pAlph;
- (void)outputCallback:(NSString *)s;
- (void)deleteCallback:(NSString *)s;
- (unsigned int)move:(Dasher::CControlManager::EditDistance)amt forwards:(BOOL)bForwards;
- (unsigned int)del:(Dasher::CControlManager::EditDistance)amt forwards:(BOOL)bForwards;
- (BOOL)supportsSpeech;
- (void)speak:(NSString *)text interrupt:(BOOL)bInt;
- (void)copy:(NSString *)text;
//forcibly inserts text - and then rebuilds the model
- (void)insertText:(NSString *)text;
- (void)clearBtn; //prompts for confirmation, then calls:
- (void)clearText;
- (NSString *)allText;
- (void)notifySpeedChange;
- (void)refreshToolbar;
- (NSString *)textAtOffset:(unsigned int)offset Length:(unsigned int)length;
- (void)setLockText:(NSString *)s;
- (void)displayMessage:(NSString *)msg;
+ (DasherAppDelegate *)theApp;

@property (readonly) CDasherInterfaceBridge *dasherInterface;
@property BOOL allowsRotation;
@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) UILabel *speedLabel;

@property (nonatomic, assign) CGFloat speedChangeStep;
@end

@interface UIViewController (lockable)
- (void)doAsyncLocked:(NSString *)msg target:(id)obj selector:(SEL)sel param:(id)param,...;
@end
