//
//  DasherAppDelegate.m
//  Dasher
//
//  Created by Alan Lawrence on 18/03/2009.
//  Copyright Cavendish Laboratory 2009. All rights reserved.
//

#import "DasherAppDelegate.h"
#import "EAGLView.h"
#import "LanguagesController.h"
#import "InputMethodSelector.h"
#import "CalibrationController.h"
#import "DasherUtil.h"
#import "Common.h"
#import "TextView.h"
#import "MiscSettings.h"

//declare some private methods!
@interface DasherAppDelegate ()
- (void)settings;
- (void)initDasherInterface;
- (void)finishStartup;
- (void)doSpeedBtnImage:(NSString *)msg;
- (CGRect)doLayout:(UIInterfaceOrientation)orient;
@property (retain) UILabel *screenLockLabel;
@end

@implementation DasherAppDelegate

@synthesize screenLockLabel;

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  if (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
    return NO;
  if (m_bLandscapeSupported || interfaceOrientation == UIInterfaceOrientationPortrait)
    return YES;
  return NO;
}

-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
  [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
  [self doLayout:self.interfaceOrientation];
}

-(void)viewWillAppear:(BOOL)animated {
  if (m_bLandscapeSupported) {
    UIDeviceOrientation devOrient = [[UIDevice currentDevice] orientation];
    UIInterfaceOrientation intOrient;
    CGAffineTransform trans;
    switch (devOrient) {
      case UIDeviceOrientationLandscapeLeft:
        intOrient = UIInterfaceOrientationLandscapeRight;
        trans = CGAffineTransformMakeRotation(M_PI/2.0);
        break;
      case UIDeviceOrientationLandscapeRight:
        intOrient = UIInterfaceOrientationLandscapeLeft;
        trans = CGAffineTransformMakeRotation(-M_PI/2.0);
        break;
      case UIDeviceOrientationPortrait:
      case UIDeviceOrientationPortraitUpsideDown:
      default: //???
        intOrient = UIInterfaceOrientationPortrait;
        trans = CGAffineTransformIdentity;
        break;
    }
    if (self.interfaceOrientation != intOrient) {
      [[UIApplication sharedApplication] setStatusBarOrientation:intOrient];
      self.view.transform = trans;
      [self doLayout:intOrient];
    }
  }
}

-(void)setLandscapeSupported:(BOOL)bLandscapeSupported {
  m_bLandscapeSupported = bLandscapeSupported;
  //note that if we've just _enabled_ landscape support when the phone
  // was previously in landscape orientation (but the interface _not_),
  // there will be no rotation event sent by the OS; however,
  // we do our best to adjust the interface to suit (i.e., putting it into
  // landscape mode, now that it newly supports this) in viewWillAppear:, above.

  //The case of _disabling_ landscape support when the phone+interface were already in landscape
  // orientation, we do _not_ need to handle (for now): the landscape interface does not allow
  // calling up the settings in order to select a different input method (which might disable it!)
}

/// Sets sizes of toolbar and textview according to supplied orientation
/// Also computes and returns desired size of glView, and sets said _iff_ glView is non-nil
-(CGRect)doLayout:(UIInterfaceOrientation)orient {
  CGRect appFrame = [UIScreen mainScreen].applicationFrame;
  window.frame=appFrame;
  self.view.frame = CGRectMake(0.0, 0.0, appFrame.size.width, appFrame.size.height);

  CGSize mainSize = self.view.bounds.size;
  CGRect dashRect;
  switch (orient) {
    case UIInterfaceOrientationPortrait: {
      dashRect = CGRectMake(0.0, 0.0, mainSize.width, mainSize.height - 100.0);
      CGRect textRect = CGRectMake(0.0, dashRect.size.height, mainSize.width, 70.0);
      text.frame = textRect;
      text.bLandscape = NO;
      messageLabel.frame = CGRectMake(0.0, textRect.origin.y + textRect.size.height - 30.0, mainSize.width, 30.0);
      tools.frame = CGRectMake(0.0, mainSize.height - 30.0, mainSize.width, 30.0);
      [self.view addSubview:tools];
      break;
    }
    case UIInterfaceOrientationLandscapeRight:
    case UIInterfaceOrientationLandscapeLeft: {
      CGRect textRect = CGRectMake(0.0, 0.0, 100.0, mainSize.height);//-30.0);
      text.frame = textRect;
      text.bLandscape = YES;
      messageLabel.frame = CGRectMake(0.0, textRect.origin.y + textRect.size.height - 100.0, textRect.size.width, 100.0);
      dashRect = CGRectMake(textRect.size.width, 0.0, mainSize.width-textRect.size.width, mainSize.height);//-30.0);
      [tools removeFromSuperview];
      break;
    }
    default:
      NSAssert(false, @"Unexpected interface orientation");
  }
  if (glView) glView.frame=dashRect;
  return dashRect;
}

-(CDasherInterfaceBridge *)dasherInterface {return _dasherInterface;}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
  //by default, we support landscape mode (i.e. unless the input device _disables_ it)
  // - hence, set now, before the input device is activate()d...
  m_bLandscapeSupported = YES;

  //sizes set in doLayout, below...
	window = [[UIWindow alloc] init];
	self.view = [[[UIView alloc] init] autorelease];

	[window addSubview:self.view];
  
  //create GUI components...
	text = [[[TextView alloc] init] autorelease];
  messageLabel = [[[UILabel alloc] init] autorelease];
  tools = [[UIToolbar alloc] init]; //retain a reference (until dealloc) because of rotation
	glView = [[[EAGLView alloc] initWithFrame:[self doLayout:UIInterfaceOrientationPortrait] Delegate:self] autorelease];
		
  //start training in a separate thread. (Has to be after first
  // call to doLayout, or get a black band across top of screen)
  [self doAsyncLocked:@"Initializing..." target:self selector:@selector(initDasherInterface) param:nil];

	text.text=@"";
	text.editable = NO;
	text.delegate = self;
	selectedText.location = selectedText.length = 0;

  messageLabel.backgroundColor = [UIColor grayColor];
  messageLabel.textColor = [UIColor whiteColor];
  messageLabel.adjustsFontSizeToFitWidth = YES;
  messageLabel.hidden = YES;
  
  //...and lay them out
	UIBarButtonItem *settings = [[[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"cog.png"] style:UIBarButtonItemStylePlain target:self action:@selector(settings)] autorelease];
	speedBtn = [UIButton buttonWithType:UIButtonTypeCustom];
	[speedBtn setImageEdgeInsets:UIEdgeInsetsMake(0.0, 2.0, 0.0, 2.0)];
	[speedBtn addTarget:self action:@selector(doSpeedBtn:forEvent:) forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];
	//[self doSpeedBtnImage:@"Foo"];

	UIBarButtonItem *clear = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemTrash target:self action:@selector(clear)] autorelease];
	UIBarButtonItem *mail = [[[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"mail.png"] style:UIBarButtonItemStylePlain target:self action:@selector(mail)] autorelease];
	[tools setItems:[NSArray arrayWithObjects:
				settings,
				[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
				[[[UIBarButtonItem alloc] initWithCustomView:speedBtn] autorelease],
				[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
				clear,
				[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
				mail,
				nil]];
	
	[self.view addSubview:glView];
	[self.view addSubview:text];
  //relying here that adding messageLabel after text, means messageLabel'll be on top?
  [self.view addSubview:messageLabel];
	[self.view addSubview:tools];
	[window makeKeyAndVisible];
  //exit this routine; initDasherInterface (in separate thread) will cause this (main) thread
  // to execute finishStartup, and finally unlock the display, when it's done with training etc.
}

- (void)initDasherInterface {
  //training takes too long to perform in applicationDidFinishLaunching;
  // so we do it here instead (having let the main thread display a "training" message);
  _dasherInterface = new CDasherInterfaceBridge(self);
  //the rest has to be done on the main thread to avoid problems with OpenGL contexts
  // (which are local to one thread); however, we'll have the background thread wait...
  [self performSelectorOnMainThread:@selector(finishStartup) withObject:nil waitUntilDone:YES];
  //...so that initDasherInterface doesn't finish before we're ready
  // (it returns to aSyncMain, which dismisses the modal view controller!)
}

- (void)finishStartup {
  self.dasherInterface->ChangeScreen(new CDasherScreenBridge(glView));
  
	[CalibrationController doSetup]; //restore tilt settings
	[self notifySpeedChange];
  self.dasherInterface->OnUIRealised(); //that does startAnimation...
  doneSetup = YES;
}

- (void)displayMessage:(NSString *)msg ID:(int)iId Type:(int)type {
  //set initial state - overriding any animation in progress...
  messageLabel.text = msg;
  messageLabel.alpha = 1.0;
  messageLabel.hidden = NO;
  //now setup an animation that'll fade it out...
  [UIView beginAnimations:@"MessageAnim" context:nil];
  [UIView setAnimationDuration:2.0];
  [UIView setAnimationCurve:UIViewAnimationCurveEaseIn];
  [UIView setAnimationDelegate:self];
  [UIView setAnimationDidStopSelector:@selector(animationDidStop:finished:context:)];
  [messageLabel setAlpha:0.0];
  [UIView commitAnimations]; //that'll abort any previous animation in progress
  // (and thus proceed with just this one)
}

- (void)animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
  if ([finished boolValue]) {
    messageLabel.hidden = YES;
  }
  //else, we've been called because the old animation's been aborted by a newer one.
  // In which case, leave the new one to proceed without interference...
}

- (void)doSpeedBtn:(id)sender forEvent:(UIEvent *)e {
    CGPoint p=[[[e allTouches] anyObject] locationInView:speedBtn];
	float x = (p.x-[speedBtn bounds].size.width/2.0)/200.0;
	float a = (x < 0.0f) ? min(-0.1f, x) : max (0.1f, x);
	self.dasherInterface->SetLongParameter(LP_MAX_BITRATE,
										   min(1200, max(10, static_cast<int>(self.dasherInterface->GetLongParameter(LP_MAX_BITRATE) + a * 100.0))));
}

- (void)clear {
	UIActionSheet *confirm = [[[UIActionSheet alloc] initWithTitle:@"Start New Document" delegate:self cancelButtonTitle:@"Keep Existing" destructiveButtonTitle:@"Discard Existing" otherButtonTitles:nil] autorelease];
	[confirm showFromToolbar:tools];
}
	
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == actionSheet.destructiveButtonIndex)
	{
		text.text=@"";
		selectedText.location = selectedText.length = 0;
		self.dasherInterface->SetBuffer(0);
	}
	//...and dismiss?
}
	
- (void)mail {
  NSString *mailString = [NSString stringWithFormat:@"mailto:?body=%@", 
						  [text.text stringByAddingPercentEscapesUsingEncoding:NSASCIIStringEncoding]];
  [[UIApplication sharedApplication] openURL:[NSURL URLWithString:mailString]];
}
	
- (void)settings {
  //avoid awful muddle if we change out of tap-to-start mode whilst running.... 
  _dasherInterface->PauseAt(0,0);
  [glView stopAnimation];
	
  UITabBarController *tabs = [[[UITabBarController alloc] init] autorelease];
  tabs.title = @"Settings";
  tabs.navigationItem.leftBarButtonItem = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(settingsDone)] autorelease];
  UINavigationController *settings = [[[UINavigationController alloc] initWithRootViewController:tabs] autorelease];

    tabs.viewControllers = [NSArray arrayWithObjects:
							[[[InputMethodSelector alloc] init] autorelease],
						    [[[CalibrationController alloc] initWithTabCon:tabs] autorelease],
						    [[[LanguagesController alloc] init] autorelease],
                [[[StringParamController alloc] initWithTitle:@"Colour" image:[UIImage imageNamed:@"palette.png"] settingParam:SP_COLOUR_ID] autorelease],
						    [[[MiscSettings alloc] init] autorelease],
						    nil];
  [self presentModalViewController:settings animated:YES];
}

- (void)settingsDone {
	[self dismissModalViewControllerAnimated:YES];
	[glView startAnimation];
}

- (void)startTimer {
	[glView startAnimation];
}

- (void)shutdownTimer {//Dasher closing...
	[glView stopAnimation];
}

- (void)outputCallback:(NSString *)s {
	text.text=[text.text stringByReplacingCharactersInRange:selectedText withString:s];
	selectedText.location++; 
	selectedText.length = 0;
	text.selectedRange = selectedText; //shows keyboard, seems unavoidable :-(
	//This isn't quite right, it jumps up then down again once you have >3 lines...
	[text scrollRangeToVisible:selectedText];
}

- (void)deleteCallback:(NSString *)s {
  if (selectedText.length == 0) selectedText.location -= (selectedText.length = 1); //select previous character
  text.text=[text.text stringByReplacingCharactersInRange:selectedText withString:@""];
  selectedText.length = 0;
  text.selectedRange = selectedText;
}

- (void)applicationWillResignActive:(UIApplication *)application {
	[glView stopAnimation];//.animationInterval = 1.0 / 5.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
  if (doneSetup) [glView startAnimation];//glView.animationInterval = 1.0 / 60.0;
}

- (void)newFrameAt:(unsigned long)time ForceRedraw:(BOOL)bForce {
	self.dasherInterface->NewFrame(time, bForce);
}

- (void)notifySpeedChange {
	NSString *caption = [NSString stringWithFormat:@"%.2f",
						 self.dasherInterface->GetLongParameter(LP_MAX_BITRATE) / 100.0];
	[self doSpeedBtnImage:caption];
}

- (void)doSpeedBtnImage:(NSString *)msg {
	CGSize size = [msg sizeWithFont:[UIFont boldSystemFontOfSize:12.0]];
	int h = size.height, w = size.width + h + 4.0;
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(nil, w, h, 8, w*4, colorSpace, kCGImageAlphaPremultipliedLast);
	UIGraphicsPushContext(context);
	CGContextClearRect(context, CGRectMake(0.0, 0.0, w, h));
	const CGFloat whiteComps[] = {1.0, 1.0, 1.0, 1.0};
	CGColorRef white = CGColorCreate(colorSpace, whiteComps);
	CGContextSetFillColorWithColor(context, white);
	CGContextSetStrokeColorWithColor(context, white);
	CGContextTranslateCTM(context, 0.0, h);
	CGContextScaleCTM(context, 1.0, -1.0);
	[msg drawAtPoint:CGPointMake(h/2.0 + 2.0, 0.0) withFont:[UIFont boldSystemFontOfSize:12.0]];
	CGContextBeginPath(context);
	CGContextMoveToPoint(context, 0.0, h/2.0);
	CGContextAddLineToPoint(context, h/2.0, 0.0);
	CGContextAddLineToPoint(context, h/2.0, h);
	CGContextFillPath(context); //implicitly ClosePath's first

	CGContextBeginPath(context);
	
	CGContextMoveToPoint(context, w - h/2.0, 0.0);
	CGContextAddLineToPoint(context, w - h/2, h);
	CGContextAddLineToPoint(context, w, h/2.0);
	CGContextFillPath(context);
	
	CGColorRelease(white);
	UIGraphicsPopContext();
	CGImageRef whole = CGBitmapContextCreateImage(context);
	CGContextRelease(context);
	[speedBtn setImage:[UIImage imageWithCGImage:whole] forState:UIControlStateNormal];
	[speedBtn sizeToFit];
	CGImageRelease(whole);
}

- (void)dealloc {
	[window release];
  [tools release];
	[super dealloc];
}

- (NSString *)textAtOffset:(int)offset Length:(int)length {
  NSRange range;
  range.location = offset;
  range.length = length;
  return [text.text substringWithRange:range];
}

#pragma mark TextViewDelegate methods

-(void)textViewDidChangeSelection:(UITextView *)textView {
  DASHER_ASSERT(textView == text);
  selectedText = text.selectedRange;
  if (selectedText.location == NSIntegerMax) selectedText.location = text.text.length;
  _dasherInterface->SetOffset(selectedText.location);
}

- (void)setLockText:(NSString *)s {
  if ([NSThread isMainThread]) {
    UILabel *lbl = self.screenLockLabel;
    if (!lbl) return;
    if (s) {
      [lbl setText:s];
      lbl.hidden = NO;
    }
    else lbl.hidden = YES;
  }
  else [self performSelectorOnMainThread:@selector(setLockText:) withObject:s waitUntilDone:YES];
}

+ (DasherAppDelegate *)theApp {
	id<UIApplicationDelegate> app = [UIApplication sharedApplication].delegate;
	NSAssert ([app isMemberOfClass:[DasherAppDelegate class]], @"AppDelegate is not DasherAppDelegate!");
	return (DasherAppDelegate *)app;
}
@end

/*@interface UIViewController (lockable)
- (void)aSyncMain:(NSInvocation *)action;
@end*/

@implementation UIViewController (lockable)

- (void)doAsyncLocked:(NSString *)msg target:(id)obj selector:(SEL)sel param:(id)param,... {
  CGSize mainSize = [UIScreen mainScreen].applicationFrame.size;
  UIViewController *lockCon = [[[UIViewController alloc] init] autorelease];
  UIImageView *imgView = [[[UIImageView alloc] initWithFrame:CGRectMake(0.0, 0.0, mainSize.width, mainSize.height)] autorelease];
  imgView.image = [UIImage imageNamed:@"Locked.png"];
  lockCon.view = imgView;
  CGRect mainLabelRect = CGRectMake(40.0, 200.0, mainSize.width-80.0, 80.0);
  UILabel *lbl1 = [[[UILabel alloc] initWithFrame:mainLabelRect] autorelease];
  lbl1.backgroundColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.8];
  lbl1.textColor = [UIColor whiteColor];
  lbl1.textAlignment = UITextAlignmentCenter;
  lbl1.text = msg;
  lbl1.adjustsFontSizeToFitWidth = YES;
  [imgView addSubview:lbl1];
  UILabel *lbl2 = [[[UILabel alloc] initWithFrame:CGRectMake(mainLabelRect.origin.x+20.0,mainLabelRect.origin.y+mainLabelRect.size.height+20,mainLabelRect.size.width-40, mainLabelRect.size.height-40)] autorelease];
  lbl2.backgroundColor = lbl1.backgroundColor;
  lbl2.textColor = lbl1.textColor;
  lbl2.textAlignment = UITextAlignmentCenter;
  lbl2.adjustsFontSizeToFitWidth = YES;
  lbl2.hidden = YES;
  [imgView addSubview:lbl2];
  //think we just hope old value of screenLockLabel was null
  // (i.e. no nested calls to doAsyncLocked...)
  [DasherAppDelegate theApp].screenLockLabel = lbl2;
  [self presentModalViewController:lockCon animated:NO];
  NSMethodSignature *sig = [obj methodSignatureForSelector:sel];
  NSInvocation *action = [NSInvocation invocationWithMethodSignature:sig];
  int numArgs = [sig numberOfArguments];
  [action setTarget:obj];
  [action setSelector:sel];
  if (numArgs>2) //NSInvocation/NSMethodSignature args start at 2
  {              // (0 = self, 1 = _cmd)
    [action setArgument:&param atIndex:2];  // First arg <param> isn't part of the varargs list,
    va_list argumentList;                 
    va_start(argumentList, param); //look for varargs *after* param
    for (int posn = 3; posn < numArgs ; posn++) {
      param = va_arg(argumentList, id);
      [action setArgument:&param atIndex:posn];
    }
    va_end(argumentList);
  }
  [self performSelectorInBackground:@selector(aSyncMain:) withObject:action];
}

- (void)aSyncMain:(NSInvocation *)action {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [action invoke];
  //passing 'nil' here, where a BOOL is expected, is a horrendous trick - nil = 0x0 is effectively reinterpret_casted... 
  [DasherAppDelegate theApp].screenLockLabel = nil;
  [self performSelectorOnMainThread:@selector(dismissModalViewControllerAnimated:) withObject:nil waitUntilDone:NO];
  [pool release];
}

@end
